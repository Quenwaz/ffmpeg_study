#define __STDC_CONSTANT_MACROS
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL/SDL.h"
}
#include <Windows.h>


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
		// 1000 / 40 = 25 ��25 FPS
		SDL_Delay(40);
	}

	//Break
	SDL_Event event;
	event.type = SDL_BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}


int WINAPI WinMain(HINSTANCE h1,HINSTANCE h2,LPTSTR cmdline,int cmdshow)
// int main(int argc, char const *argv[])
{
    av_register_all();
    avformat_network_init();

    AVFormatContext *pFormatCtx = avformat_alloc_context();
    char *filepath = R"(testdata\wmkd2.mp4)";
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) < 0)
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, 0) < 0)
    {
        return -1;
    }

    av_dump_format(pFormatCtx, 0, filepath, 0);

    int videoindex = -1;
    //pFormatCtx->Streams 仅仅是一组pFormatCtx->nb_streams 的指针 包含了哪些流
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }

    if (videoindex == -1)
    {
        return -1;
    }

    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Could not open codec.\n");
        return -1;
    }

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameYUV = av_frame_alloc();
    unsigned char *out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    struct SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                                        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }


    double scale_ratio = 0.5;
    int screen_w = pCodecCtx->width * scale_ratio;
    int screen_h = pCodecCtx->height* scale_ratio;
    //SDL 2.0 Support for multiple windows
    SDL_Window *screen = SDL_CreateWindow(filepath, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          screen_w, screen_h,
                                          SDL_WINDOW_OPENGL/* | SDL_WINDOW_FULLSCREEN_DESKTOP*/ | SDL_WINDOW_RESIZABLE );

    if (!screen)
    {
        printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
    SDL_Rect sdlRect;
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = screen_w;
    sdlRect.h = screen_h;

    _thread_data thread_data{false, false};
    SDL_Thread *video_tid = SDL_CreateThread(wnd_refresh_thread,NULL,&thread_data);

	for (;;) {
		//Wait
        SDL_Event event;
		SDL_WaitEvent(&event);
		if(event.type==SDL_REFRESH_EVENT){
			while(true){
				if(av_read_frame(pFormatCtx, packet)<0)
					thread_data.stop = true;
 
				if(packet->stream_index==videoindex)
					break;
			}

            int got_picture = 0;
			if(avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet) < 0){
				printf("Decode Error.\n");
				break;
			}

            sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
            //SDL---------------------------
            SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0] );  
            SDL_RenderClear( sdlRenderer );  
            //SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );  
            SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, NULL);  
            SDL_RenderPresent( sdlRenderer );  
            //SDL End-----------------------
            av_free_packet(packet);
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
	SDL_Quit();
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

    return 0;
}
