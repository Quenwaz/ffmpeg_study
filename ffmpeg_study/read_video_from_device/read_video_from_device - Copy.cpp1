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

int sfp_refresh_thread(void* opaque)
{
	thread_exit = 0;
	while (!thread_exit)
	{
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	thread_exit = 0;
	//Break  
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}


//Show Dshow Device  
void show_dshow_device()
{
	AVFormatContext* pFormatCtx = avformat_alloc_context();
	AVDictionary* options = nullptr;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat* iformat = av_find_input_format("dshow");
	printf("========Device Info=============\n");
	avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);
	printf("================================\n");
}

//Show Dshow Device Option  
void show_dshow_device_option()
{
	AVFormatContext* pFormatCtx = avformat_alloc_context();
	AVDictionary* options = nullptr;
	av_dict_set(&options, "list_options", "true", 0);
	AVInputFormat* iformat = av_find_input_format("dshow");
	printf("========Device Option Info======\n");
	avformat_open_input(&pFormatCtx, "video=Integrated Camera", iformat, &options);
	printf("================================\n");
}

//Show VFW Device  
void show_vfw_device()
{
	AVFormatContext* pFormatCtx = avformat_alloc_context();
	AVInputFormat* iformat = av_find_input_format("vfwcap");
	printf("========VFW Device Info======\n");
	avformat_open_input(&pFormatCtx, "list", iformat, nullptr);
	printf("=============================\n");
}

//Show AVFoundation Device  
void show_avfoundation_device()
{
	AVFormatContext* pFormatCtx = avformat_alloc_context();
	AVDictionary* options = nullptr;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat* iformat = av_find_input_format("avfoundation");
	printf("==AVFoundation Device Info===\n");
	avformat_open_input(&pFormatCtx, "", iformat, &options);
	printf("=============================\n");
}


int main(int argc, char* argv[])
{
	AVFormatContext* pFormatCtx;
	int i, videoindex = 0;

	// av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	//Open File  
	//char filepath[]="src01_480x272_22.h265";  
	//avformat_open_input(&pFormatCtx,filepath,NULL,NULL)  

	//Register Device  
	avdevice_register_all();

	//Windows  
#ifdef _WIN32

	//Show Dshow Device  
	show_dshow_device();
	//Show Device Options  
	show_dshow_device_option();
	//Show VFW Options  
	show_vfw_device();

#if USE_DSHOW
	AVInputFormat* ifmt = av_find_input_format("dshow");
	//Set own video device's name  
	if (avformat_open_input(&pFormatCtx, "video=Integrated Camera", ifmt, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
#else
	AVInputFormat* ifmt = av_find_input_format("vfwcap");
	AVDictionary* format_opts = nullptr;
	av_dict_set_int(&format_opts, "rtbufsize", 1024 * 1024 * 1024, 0);
	if (avformat_open_input(&pFormatCtx, "0", ifmt, nullptr) != 0) //video=USB Camera
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}
#endif
#elif defined linux
	//Linux  
	AVInputFormat* ifmt = av_find_input_format("video4linux2");
	if (avformat_open_input(&pFormatCtx, "/dev/video0", ifmt, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
#else
	show_avfoundation_device();
	//Mac  
	AVInputFormat* ifmt = av_find_input_format("avfoundation");
	//Avfoundation  
	//[video]:[audio]  
	if (avformat_open_input(&pFormatCtx, "0", ifmt, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
#endif


	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	if (videoindex == -1)
	{
		printf("Couldn't find a video stream.\n");
		return -1;
	}
	auto pCodecpar = pFormatCtx->streams[videoindex]->codecpar;
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
	//SDL----------------------------  
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}
	int screen_w = 0, screen_h = 0;

	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;

	SDL_Window* screen;
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                          screen_w, screen_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!screen)
	{
		printf("SDL: could not set video mode - exiting:%s\n", SDL_GetError());
		return -1;
	}


	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)

	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width,
	                                            pCodecCtx->height);


	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = screen_w;
	rect.h = screen_h;
	//SDL End------------------------  
	int ret = 0;

	AVPacket* packet = static_cast<AVPacket*>(av_malloc(sizeof(AVPacket)));

#if OUTPUT_YUV420P
	FILE* fp_yuv = fopen("output.yuv", "wb+");
#endif

	// pFrame->color_range = AVCOL_RANGE_JPEG;
	struct SwsContext* img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
	                                                    pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
	                                                    SWS_BICUBIC, nullptr, NULL, NULL);

	
	//------------------------------  
	SDL_CreateThread(sfp_refresh_thread, NULL, NULL);

	SDL_Rect sdlRect;
	//Event Loop  
	SDL_Event event;

	for (;;)
	{
		//Wait  
		SDL_WaitEvent(&event);
		if (event.type == SFM_REFRESH_EVENT)
		{
			//------------------------------  
			if (av_read_frame(pFormatCtx, packet) >= 0)
			{
				if (packet->stream_index == videoindex)
				{
					if (ret < avcodec_send_packet(pCodecCtx, packet) ||
						ret < avcodec_receive_frame(pCodecCtx, pFrame))
					{
						printf("Decode Error.\n");
						return -1;
					}

	

					//pFrameYUV->data[0] = pFrame->data[0];
					//pFrameYUV->data[1] = pFrame->data[1];
					//pFrameYUV->data[2] = pFrame->data[2];
					//pFrameYUV->linesize[0] = pFrame->linesize[0];
					//pFrameYUV->linesize[1] = pFrame->linesize[1];
					//pFrameYUV->linesize[2] = pFrame->linesize[2];
					sws_scale(img_convert_ctx, static_cast<const unsigned char* const*>(pFrame->data),
					          pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
					SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
					//SDL_UpdateYUVTexture(sdlTexture, nullptr, pFrameYUV->data[0], pFrameYUV->linesize[0],
					//                     pFrameYUV->data[1], pFrameYUV->linesize[1],
					//                     pFrameYUV->data[2], pFrameYUV->linesize[2]);

					//FIX: If window is resize
					sdlRect.x = 0;
					sdlRect.y = 0;
					sdlRect.w = screen_w;
					sdlRect.h = screen_h;

					SDL_RenderClear(sdlRenderer);
					SDL_RenderCopy(sdlRenderer, sdlTexture, nullptr, &sdlRect);
					SDL_RenderPresent(sdlRenderer);
#if OUTPUT_YUV420P
					int y_size = pCodecCtx->width * pCodecCtx->height;
					fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y     
					fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U    
					fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V    
#endif
				}
				av_packet_unref(packet);
			}
			else
			{
				//Exit Thread  
				thread_exit = 1;
			}
		}
		else if (event.type == SDL_QUIT)
		{
			thread_exit = 1;
		}
		else if (event.type == SFM_BREAK_EVENT)
		{
			break;
		}
	}


	sws_freeContext(img_convert_ctx);

#if OUTPUT_YUV420P
	fclose(fp_yuv);
#endif

	SDL_Quit();

	av_free(out_buffer);  
	av_free(pFrameYUV);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);


	return 0;
}
