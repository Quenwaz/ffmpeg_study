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

//Output YUV420P   
#define OUTPUT_YUV420P 0
//'1' Use Dshow   
//'0' Use VFW  
#define USE_DSHOW 0


//Refresh Event  
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)


int thread_exit = 0;

int thread_pause = 0;

int sfp_refresh_thread(void* opaque) {
	thread_exit = 0;
	thread_pause = 0;

	while (!thread_exit) {
		if (!thread_pause) {
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		SDL_Delay(40);
	}
	thread_exit = 0;
	thread_pause = 0;
	//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
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

	SDL_Window* screen = NULL;
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
	*pSDLTexture = SDL_CreateTexture(*pSDLRender, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);

	return *pSDLTexture != NULL;
}


int main(int argc, char* argv[])
{
	//Register Device  
	avdevice_register_all();

	AVInputFormat* ifmt = av_find_input_format("vfwcap");
	AVFormatContext* pFormatCtx = NULL;
	if (avformat_open_input(&pFormatCtx, "0",  ifmt, nullptr) != 0) //video=USB Camera
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
	for (int i = 0; i < pFormatCtx->nb_streams; i++){
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
	uint8_t* out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

	int screen_w = pCodecCtx->width, screen_h = pCodecCtx->height;



	SDL_Texture* pSDLTexture =  NULL;
	SDL_Renderer* pSDLRender = NULL;
	if (!Create_SDL_Texture(screen_w, screen_h, &pSDLTexture, &pSDLRender))
	{
		return 0;
	}


	AVPacket* packet = static_cast<AVPacket*>(av_malloc(sizeof(AVPacket)));

	// pFrame->color_range = AVCOL_RANGE_JPEG;
	struct SwsContext* img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, nullptr, NULL, NULL);

	
	//------------------------------  
	SDL_CreateThread(sfp_refresh_thread, NULL, NULL);

	SDL_Rect sdlRect;
	//Event Loop  
	SDL_Event event;

	for (;;) {
		//Wait
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT) {
			while (1) {
				if (av_read_frame(pFormatCtx, packet) < 0)
					thread_exit = 1;

				if (packet->stream_index == videoindex)
					break;
			}
			
			if (0 < avcodec_send_packet(pCodecCtx, packet) ||
				0 < avcodec_receive_frame(pCodecCtx, pFrame))
			{
				printf("Decode Error.\n");
				break;
			}
			else {
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
				//SDL---------------------------
				SDL_UpdateTexture(pSDLTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
				SDL_RenderClear(pSDLRender);
				//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );  
				SDL_RenderCopy(pSDLRender, pSDLTexture, NULL, NULL);
				SDL_RenderPresent(pSDLRender);
				//SDL End-----------------------
			}
			av_packet_unref(packet);
		}
		else if (event.type == SDL_KEYDOWN) {
			//Pause
			if (event.key.keysym.sym == SDLK_SPACE)
				thread_pause = !thread_pause;
		}
		else if (event.type == SDL_QUIT) {
			thread_exit = 1;
		}
		else if (event.type == SFM_BREAK_EVENT) {
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
