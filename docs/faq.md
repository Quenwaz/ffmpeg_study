## 学习ffmpeg常见问题集锦

#### deprecated pixel format used, make sure you did set range correctly

**原因**: 之前的格式在新版本中不适用了

**解决方案**: 进行以下转换即可

```cpp
AVPixelFormat ConvertDeprecatedFormat(enum AVPixelFormat format)
{
    switch (format)
    {
    case AV_PIX_FMT_YUVJ420P:
        return AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        return AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        return AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        return AV_PIX_FMT_YUV440P;
        break;
    default:
        return format;
        break;
    }
}
```



https://blog.csdn.net/leixiaohua1020/article/details/38868499

https://github.com/libsdl-org/SDL/blob/main/test/teststreaming.c

https://github.com/BtbN/FFmpeg-Builds/releases
