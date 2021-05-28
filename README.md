# 介绍

踏入流媒体新世界大门， 记录过程中的所见所思。

# 更新日志

# 目录

- [介绍](#介绍)

- [概念](#概念)
  - [图像](#图像)
  - [视频](#视频)
  - [容器](##容器)

- [学习资源](#学习资源)

# 概念

## 图像与颜色模型

图像说白了就是一个二维矩阵，且支持多通道。 单通道图为灰度图， 三通道为彩色图。矩阵中的每个点称为**像素(图像元素)**。像素的色彩由三原色(**RGB**)的强度表示。

三通道的图像可以理解成由三个二维平面叠加而成。

![RGB](img/RGB.png)

存储颜色的强度需要一定大小的数据空间， 这个大小就是**颜色深度**。 假如每个颜色强度占用8bit（0~255）,那么**颜色深度就是24**(3*8)bit。所以1个字节表示颜色强度可有2^24 种颜色 

除了上述表示图像的RGB颜色模型外， 还有一种**YCbCr**的颜色模型，YCbCr将亮度和色度(颜色)分离:

- Y表示亮度
- Cb表示蓝色色度
- Cr表示红色色度

![](img/ycbcr.png)

RGB与YCbCr可相互转换:

RGB=>YCbCr

```
Y = 0.299R + 0.587G + 0.114B
Cb = 0.564(B - Y)
Cr = 0.713(R - Y)
```

YCbCr=>RGB

```
R = Y + 1.402Cr
B = Y + 1.772Cb
G = Y - 0.344Cb - 0.714Cr
```



图像的**分辨率**是单个平面内像素的数量， 通常由**宽*高**表示。 

图像有三个宽高比表示：

- 显示宽高比**DAR**(Display Aspect Ratio)，即最终播放出来的画面的宽与高之比。
- 单个像素宽高比**PAR**(Pixel Aspect Ratio)，长宽比为1时，这时的像素我们成为方形像素。
- 采样纵横比**SAR**(Sample Aspect Ratio)，视频横向对应的像素个数比上视频纵向的像素个数。即为我们通常提到的分辨率。

它们之间的关系是：**DAR=SAR x PAR**

## 视频

视频即是在单位时间内连续的播放 n 帧的图像。 若单位时间为秒，则n为帧率**FPS**(Frames Per Second), 表示每秒帧数。常用视频FPS有24fps、30fps、60fps三种, 流畅度差异如下图所示:

![24、30、60fps的区别](img/video-fps.gif)

**比特率**是播放视频每秒所需的数据量（即常说的**码率**）。

> 比特率 = 宽 * 高 * 颜色深度 * 帧每秒

一段每秒 30 帧，每像素 24 bits，分辨率是 480x240 的视频，它将需要 **82,944,000 比特每秒**或 82.944 Mbps (30x480x240x24)。

比特率又分为恒定比特率(**CBR**)和可变比特率(**VBR**), 差别在于一个恒定一个可变。

## 容器



# 学习资源

[FFmpeg 视频处理入门教程](http://www.ruanyifeng.com/blog/2020/01/ffmpeg.html)

[digital_video_introduction](https://github.com/leandromoreira/digital_video_introduction)

[ffmpeg-libav-tutorial](https://github.com/leandromoreira/ffmpeg-libav-tutorial)

[An ffmpeg and SDL Tutorial](http://dranger.com/ffmpeg/)

[详解yuv系列（一）---YUV444](https://blog.51cto.com/u_7335580/2059670)

