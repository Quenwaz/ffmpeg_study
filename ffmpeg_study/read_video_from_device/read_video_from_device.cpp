#include <exception>
#include <stdio.h>
#include "libavutil/pixfmt.h"
//#include "libavdevice/avdevice.h"

#ifdef _WIN32
//Windows  
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include <libavutil/imgutils.h>
#include "SDL/SDL.h"
};
#else
//Linux...  
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>  
#include <libavformat/avformat.h>  
#include <libswscale/swscale.h>  
#include <libavdevice/avdevice.h>  
#include <SDL/SDL.h>  
#ifdef __cplusplus
};
#endif
#endif


//Refresh Event  
#define SDL_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define SDL_BREAK_EVENT  (SDL_USEREVENT + 2)

struct _thread_data
{
	bool stop;
	bool pause;
};


int wnd_refresh_thread(void* opaque)
{
	_thread_data* p_thread_data = static_cast<_thread_data*>(opaque);

	while (!p_thread_data->stop)
	{
		if (!p_thread_data->pause)
		{
			SDL_Event event;
			event.type = SDL_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		// 1000 / 40 = 25 ¼´25 FPS
		SDL_Delay(40);
	}

	//Break
	SDL_Event event;
	event.type = SDL_BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}


bool Create_SDL_Texture(int screen_w, int screen_h, SDL_Texture** pSDLTexture, SDL_Renderer** pSDLRender)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return false;
	}

	SDL_Window* screen = nullptr;
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                          screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!screen)
	{
		printf("SDL: could not set video mode - exiting:%s\n", SDL_GetError());
		return false;
	}

	*pSDLRender = SDL_CreateRenderer(screen, -1, 0);
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	*pSDLTexture = SDL_CreateTexture(*pSDLRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_w,
	                                 screen_h);

	return *pSDLTexture != nullptr;
}


int main(int argc, char* argv[])
{
	//Register Device  
	avdevice_register_all();

	AVInputFormat* ifmt = av_find_input_format("vfwcap");
	AVFormatContext* pFormatCtx = nullptr;
	if (avformat_open_input(&pFormatCtx, "0", ifmt, nullptr) != 0) //video=USB Camera
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}


	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	int videoindex = -1;
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}

	if (videoindex == -1)
	{
		printf("Couldn't find a video stream.\n");
		return -1;
	}

	const auto pCodecpar = pFormatCtx->streams[videoindex]->codecpar;
	AVCodec* pCodec = avcodec_find_decoder(pCodecpar->codec_id);
	if (pCodec == nullptr)
	{
		printf("Codec not found.\n");
		return -1;
	}

	AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(pCodecCtx, pCodecpar);
	if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
	{
		printf("Could not open codec.\n");
		return -1;
	}
	AVFrame* pFrame = av_frame_alloc();
	AVFrame* pFrameYUV = av_frame_alloc();

	double dRatio = 0.5;
	const int screen_w = pCodecCtx->width * dRatio;
	const int screen_h = pCodecCtx->height * dRatio;


	uint8_t* out_buffer = static_cast<uint8_t*>(av_malloc(
		av_image_get_buffer_size(AV_PIX_FMT_YUV420P, screen_w, screen_h, 1)));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, screen_w, screen_h, 1);


	SDL_Texture* pSDLTexture = nullptr;
	SDL_Renderer* pSDLRender = nullptr;
	if (!Create_SDL_Texture(screen_w, screen_h, &pSDLTexture, &pSDLRender))
	{
		return 0;
	}

	AVPacket* packet = static_cast<AVPacket*>(av_malloc(sizeof(AVPacket)));

	// pFrame->color_range = AVCOL_RANGE_JPEG;
	struct SwsContext* img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
	                                                    screen_w, screen_h, AV_PIX_FMT_YUV420P,
	                                                    SWS_BICUBIC, nullptr, nullptr, nullptr);

	_thread_data thread_data{false, false};
	SDL_CreateThread(wnd_refresh_thread, NULL, &thread_data);
	//Event Loop  
	for (;;)
	{
		//Wait
		SDL_Event event;
		SDL_WaitEvent(&event);
		if (event.type == SDL_REFRESH_EVENT)
		{
			while (true)
			{
				if (av_read_frame(pFormatCtx, packet) < 0)
					thread_data.stop = true;

				if (packet->stream_index == videoindex)
					break;
			}

			if (0 < avcodec_send_packet(pCodecCtx, packet) ||
				0 < avcodec_receive_frame(pCodecCtx, pFrame))
			{
				printf("Decode Error.\n");
				break;
			}

			sws_scale(img_convert_ctx, static_cast<const unsigned char* const*>(pFrame->data), pFrame->linesize, 0,
			          pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

			SDL_UpdateYUVTexture(pSDLTexture,                   // sdl texture
				NULL,
				pFrameYUV->data[0],            // y plane
				pFrameYUV->linesize[0],        // y pitch
				pFrameYUV->data[1],            // u plane
				pFrameYUV->linesize[1],        // u pitch
				pFrameYUV->data[2],            // v plane
				pFrameYUV->linesize[2]         // v pitch
			);
			// SDL_UpdateTexture(pSDLTexture, nullptr, pFrameYUV->data[0], pFrameYUV->linesize[0]);
			SDL_RenderClear(pSDLRender);
			SDL_RenderCopy(pSDLRender, pSDLTexture, nullptr, nullptr);
			SDL_RenderPresent(pSDLRender);
			av_packet_unref(packet);
		}
		else if (event.type == SDL_KEYDOWN)
		{
			//Pause
			if (event.key.keysym.sym == SDLK_SPACE)
				thread_data.pause = !thread_data.pause;
		}
		else if (event.type == SDL_QUIT)
		{
			thread_data.stop = true;
		}
		else if (event.type == SDL_BREAK_EVENT)
		{
			break;
		}
	}
	sws_freeContext(img_convert_ctx);

	SDL_DestroyRenderer(pSDLRender);
	SDL_Quit();

	av_frame_free(&pFrame);
	av_frame_free(&pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	return 0;
}
