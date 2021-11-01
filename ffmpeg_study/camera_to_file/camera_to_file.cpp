extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
}

#include <cstdio>
#include <cstdlib>


int writeJPEG(AVFrame* frame,int width,int height){
    const char* out_file = "hello_world.jpg";
    //新建一个输出的AVFormatContext 并分配内存
    AVFormatContext* output_cxt = avformat_alloc_context();
    avformat_alloc_output_context2(&output_cxt,NULL,"singlejpeg",out_file);

    //设置输出文件的格式
    // output_cxt->oformat = av_guess_format("mjpeg",NULL,NULL);

    //创建和初始化一个和该URL相关的AVIOContext
    if(avio_open(&output_cxt->pb,out_file,AVIO_FLAG_READ_WRITE) < 0){
        av_log(NULL,AV_LOG_ERROR,"不能打开文件  \n");
        return -1;
    }

    //构建新的Stream
    AVStream* stream = avformat_new_stream(output_cxt,NULL);
    if(stream == NULL){
        av_log(NULL,AV_LOG_ERROR,"创建AVStream失败  \n");
        return -1;
    }
    //初始化AVStream信息
    AVCodecContext* codec_cxt = stream->codec;

    codec_cxt->codec_id = output_cxt->oformat->video_codec;
    codec_cxt->codec_type = AVMEDIA_TYPE_VIDEO;
    codec_cxt->pix_fmt = AV_PIX_FMT_YUVJ420P;
    codec_cxt->height = height;
    codec_cxt->width = width;
    codec_cxt->time_base.num = 1;
    codec_cxt->time_base.den = 25;

    //打印输出文件信息
    av_dump_format(output_cxt,0,out_file,1);

    AVCodec* codec = avcodec_find_encoder(codec_cxt->codec_id);
    if(!codec){
        av_log(NULL,AV_LOG_ERROR,"找不到编码器  \n");
        return -1;
    }

    if(avcodec_open2(codec_cxt,codec,NULL) < 0){
        av_log(NULL,AV_LOG_ERROR,"不能打开编码器  \n");
        return -1;
    }
    avcodec_parameters_from_context(stream->codecpar,codec_cxt);

    //写入文件头
    avformat_write_header(output_cxt,NULL);
    int size = codec_cxt->width * codec_cxt->height;

    AVPacket packet;
    av_new_packet(&packet,size * 3);

    int got_picture = 0;
    int result = avcodec_encode_video2(codec_cxt,&packet,frame,&got_picture);
    if(result < 0){
        av_log(NULL,AV_LOG_ERROR,"编码失败  \n");
        return -1;
    }
    printf("got_picture %d \n",got_picture);
    if(got_picture == 1){
        //将packet中的数据写入本地文件
        result = av_write_frame(output_cxt,&packet);
    }
    av_free_packet(&packet);
    //将流尾写入输出媒体文件并释放文件数据
    av_write_trailer(output_cxt);
    if(frame){
        av_frame_unref(frame);
    }
    avio_close(output_cxt->pb);
    avformat_free_context(output_cxt);
    return 0;
}

int saveJpg(AVFrame *pFrame,int width, int height, int iframe) {
    
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    // 设置输出文件格式
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);

    // 创建并初始化输出AVIOContext

    char filename[32];
    sprintf(filename, "frame%d.jpg", iframe);
    if (avio_open(&pFormatCtx->pb, filename, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Couldn't open output file.");
        return -1;
    }

    // 构建一个新stream
    AVStream *pAVStream = avformat_new_stream(pFormatCtx, 0);
    if (pAVStream == NULL) {
        return -1;
    }

    AVCodecParameters *parameters = pAVStream->codecpar;
    parameters->codec_id = pFormatCtx->oformat->video_codec;
    parameters->codec_type = AVMEDIA_TYPE_VIDEO;
    parameters->format = AV_PIX_FMT_YUVJ420P;
    parameters->width = width;
    parameters->height = height;

    AVCodec *pCodec = avcodec_find_encoder(pAVStream->codecpar->codec_id);

    if (!pCodec) {
        printf("Could not find encoder\n");
        return -1;
    }

    AVCodecContext *pCodeCtx = avcodec_alloc_context3(pCodec);
    if (!pCodeCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    if ((avcodec_parameters_to_context(pCodeCtx, parameters)) < 0) {
        fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        return -1;
    }

    pCodeCtx->time_base = {1, 25};

    if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.");
        return -1;
    }

    int nJpegQuality = 80;     
    pCodeCtx->qcompress = (float)nJpegQuality/100.f; // 0~1.0, default is 0.5      

    int ret = avformat_write_header(pFormatCtx, NULL);
    if (ret < 0) {
        printf("write_header fail\n");
        return -1;
    }

    int y_size = width * height;

    //Encode
    // 给AVPacket分配足够大的空间
    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);

    // 编码数据
    ret = avcodec_send_frame(pCodeCtx, pFrame);
    if (ret < 0) {
        char errmsg[512]={0};
        av_strerror(ret,errmsg, sizeof(errmsg) );
        printf("Could not avcodec_send_frame:%s\n", errmsg);
        return -1;
    }

    // 得到编码后数据
    ret = avcodec_receive_packet(pCodeCtx, &pkt);
    if (ret < 0) {
        printf("Could not avcodec_receive_packet");
        return -1;
    }

    ret = av_write_frame(pFormatCtx, &pkt);

    if (ret < 0) {
        printf("Could not av_write_frame");
        return -1;
    }

    av_packet_unref(&pkt);

    //Write Trailer
    av_write_trailer(pFormatCtx);


    avcodec_close(pCodeCtx);
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    return 0;
}

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

int main(int argc, char *argv[])
{
    // Register all formats and codecs
    av_register_all();

    avdevice_register_all();

    AVFormatContext *pFormatCtx = NULL;

    // Open video camera
    AVInputFormat *ifmt = av_find_input_format("vfwcap");
    if (avformat_open_input(&pFormatCtx, "0", ifmt, NULL) != 0)
        return -1;

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
        return -1; // Couldn't find stream information

    av_dump_format(pFormatCtx, 0, "0", 0);

    int i; 
    // Find the first video stream
    int videoStream = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    if (videoStream == -1)
        return -1; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    AVCodecContext *pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;

    /*#####################STEP 4#####################*/
    AVCodec *pCodec = NULL;

    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
    if (pCodec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    // Copy context
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
    if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0)
    {
        fprintf(stderr, "Couldn't copy codec context");
        return -1; // Error copying codec context
    }
    // Open codec
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
        return -1; // Could not open codec

    /*#####################STEP 5#####################*/
    AVFrame *pFrame = NULL;

    // Allocate video frame
    pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    AVFrame *pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL)
        return -1;


    uint8_t *buffer = NULL;
    int numBytes;
    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
                                pCodecCtx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));


    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24,
                    pCodecCtx->width, pCodecCtx->height);

    /*#####################STEP 6#####################*/
    struct SwsContext *sws_ctx = NULL;
    int frameFinished;
    AVPacket packet;
    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecCtx->width,
                             pCodecCtx->height,
                             pCodecCtx->pix_fmt,
                             pCodecCtx->width,
                             pCodecCtx->height,
                             AV_PIX_FMT_RGB24,
                             SWS_BILINEAR,
                             NULL,
                             NULL,
                             NULL);

    i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0)
    {
        // Is this a packet from the video stream?
        if (packet.stream_index == videoStream)
        {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // Did we get a video frame?
            if (frameFinished)
            {
                // Convert the image from its native format to RGB
                sws_scale(sws_ctx, (uint8_t const *const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGB->data, pFrameRGB->linesize);

                // Save the frame to disk
                if (++i <= 5)
                    SaveFrame(pFrameRGB, pCodecCtx->width,
                              pCodecCtx->height, i);

                if (++i <= 10)
                saveJpg(pFrame, pCodecCtx->width,
                              pCodecCtx->height, i);
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    /*#####################STEP 7#####################*/
    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codecs
    avcodec_close(pCodecCtx);
    avcodec_close(pCodecCtxOrig);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    return 0;
}
