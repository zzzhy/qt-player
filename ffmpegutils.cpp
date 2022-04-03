#include "ffmpegutils.h"

FFmpegUtils::FFmpegUtils(QObject *parent) : QObject(parent)
{

}

FFmpegUtils::~FFmpegUtils()
{
    m_videoIndex = -1;
    av_frame_free(&m_AVFrame);
    av_frame_free(&m_AVFrameRGB);
    sws_freeContext(m_SwsContext);
    av_packet_free(&m_AVPacket);
    m_audioIndex.clear();

    free(m_AVPacket);
    avcodec_close(m_AVCodecContext);
    avformat_close_input(&m_AVFormatContext);
    avformat_free_context(m_AVFormatContext);
    avformat_network_deinit();
}

void FFmpegUtils::MyFFmpegSetUrl(QString rtspUrl)
{
    m_rtspUrl = rtspUrl;
}

int FFmpegUtils::MyFFmpegInit()
{
    // 分配AVFormatContext，它是FFMPEG解封装（flv，mp4，rmvb，avi）功能的结构体，
    // 具体可参考https://blog.csdn.net/leixiaohua1020/article/details/14214705
    m_AVFormatContext = avformat_alloc_context();

    // 获取视频播放URL
    QByteArray byteRtspUrl = m_rtspUrl.toLocal8Bit();
    char *pRtspUrl = byteRtspUrl.data();
    // 设置参数
    AVDictionary *options = NULL;
    if (m_rtspUrl.startsWith("rtsp")) {
        // 设置传输协议为TCP协议
        av_dict_set(&options, "rtsp_transport", "tcp", 0);

        // 设置TCP连接最大延时时间
        av_dict_set(&options, "max_delay", "1000", 0);

        // 设置“buffer_size”缓存容量
        av_dict_set(&options, "buffer_size", "1024000", 0);

        // 设置avformat_open_input超时时间为10秒
        av_dict_set(&options, "stimeout", "10000000", 0);
    }

    // 打开网络流或文件流
    int ret = avformat_open_input(&m_AVFormatContext, pRtspUrl, NULL, &options);
    if (ret != 0)
    {
        qDebug("Couldn't open input stream ! ret: %d\n", ret);
        return -1;
    }

    // 读取流数据包并获取流的相关信息
    if (avformat_find_stream_info(m_AVFormatContext, NULL) < 0)
    {
        qDebug("Couldn't find stream information.\n");
        return -1;
    }

    // 确定流格式是否为视频
    for (unsigned int i = 0; i < m_AVFormatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters =  NULL;
        pLocalCodecParameters = m_AVFormatContext->streams[i]->codecpar;
        const AVCodec *pLocalCodec = NULL;
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        if (pLocalCodec == NULL) {
            continue;
        }

        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (m_videoIndex == -1) {
                qDebug("found video stream: index: %d", i);
                m_videoIndex = i;
                m_AVCodec = pLocalCodec;
                qDebug("Video Codec: resolution %d x %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
            }
        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (!m_audioIndex.contains(i)) {
                qDebug("found audio stream: index: %d", i);
                m_audioIndex[i] = pLocalCodec;
                qDebug("Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
            }
        }
        qDebug("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
    }

    if (m_videoIndex == -1)
    {
        qDebug("Didn't find a video stream.\n");
        return -1;
    }

    // 根据编码器的ID号查找对应的解码器
    if (NULL == m_AVCodec)
    {
        qDebug("avcodec_find_decoder fail!\n");
        return -1;
    }
    m_AVCodecContext = avcodec_alloc_context3(m_AVCodec);
    // 配置编码器上下文的参数
    //    m_AVCodecContext->bit_rate = 0;         //码率
    //    m_AVCodecContext->time_base.den = 25;   // 下面2行设置帧率，每秒/25帧
    //    m_AVCodecContext->time_base.num = 1;
    //    m_AVCodecContext->frame_number = 1;     //每包一个视频帧

    if (avcodec_parameters_to_context(m_AVCodecContext, m_AVFormatContext->streams[m_videoIndex]->codecpar) < 0)
    {
        qDebug("avcodec_parameters_to_context: AVERROR");
        return -1;
    }
    if (avcodec_open2(m_AVCodecContext, m_AVCodec, NULL) < 0)
    {
        qDebug("avcodec_open2: fail");
        return -1;
    }

    m_AVFrame = av_frame_alloc();
    m_AVPacket = av_packet_alloc();

    qDebug("============== MyFFmpegInit ok! ====================== ");

    return 0;
}

/**
 * @brief MyFFmpeg::MyFFmpepReadFrame 读取一帧数据
 * @return
 */
int FFmpegUtils::MyFFmpepReadFrame()
{
    int ret = -1;

    // 获取下一帧数据
    ret = av_read_frame(m_AVFormatContext, m_AVPacket);
    if (ret < 0)
    {
        qDebug("av_read_frame fail!\n");
        return -1;
    }

    if (m_AVPacket->stream_index != m_videoIndex)
    {
        qDebug("this is not a video frame!\n");
        return 0;
    }


    //  解码m_AVPacket，Decode the video frame of size avpkt->size from avpkt->data into picture
    ret = avcodec_send_packet(m_AVCodecContext, m_AVPacket);
    if (ret < 0)
    {
        qDebug("avcodec_decode_video2 fail!\n");
        return -1;
    }
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(m_AVCodecContext, m_AVFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            qDebug("avcodec_receive_frame: Resource temporarily unavailable | End of file ! ret: %d", ret);
            return 0;
        } else if (ret < 0) {
            qDebug("avcodec_receive_frame: legitimate decoding errors");
            return -1;
        }
        qDebug("get frame: width=%d,height=%d,format=%d,key_frame=%d",
               m_AVFrame->width, m_AVFrame->height, m_AVFrame->format, m_AVFrame->key_frame);

        m_AVFrameRGB = av_frame_alloc();
        if (!m_SwsContext) {
            m_SwsContext = sws_getContext(m_AVCodecContext->width,
                                          m_AVCodecContext->height,
                                          m_AVCodecContext->pix_fmt,
                                          m_AVCodecContext->width,
                                          m_AVCodecContext->height,
                                          AV_PIX_FMT_RGB24,
                                          SWS_BICUBIC, NULL, NULL, NULL);
        }

        int rgb_linesizes[8] = {0};
        rgb_linesizes[0] = 3*m_AVFrame->width;

        // prepare char buffer in array, as sws_scale expects
        unsigned char* rgbData[8];
        int imgBytesSyze = 3*m_AVFrame->height*m_AVFrame->width;
        // as explained above, we need to alloc extra 64 bytes
        rgbData[0] = (unsigned char *)malloc(imgBytesSyze+64);

        sws_scale(m_SwsContext,
                  m_AVFrame->data,
                  m_AVFrame->linesize,
                  0,
                  m_AVFrame->height,
                  rgbData,
                  rgb_linesizes);

        // then create QImage and copy converted frame data into it
        QImage img(m_AVFrame->width, m_AVFrame->height, QImage::Format_RGB888);

        for(int y=0; y< m_AVFrame->height; y++){
            memcpy(img.scanLine(y), rgbData[0]+y*3*m_AVFrame->width, 3*m_AVFrame->width);
        }

        QImage image = img;
        free(rgbData[0]);
        // 发送QImage
        emit MyFFmpegSigGetOneFrame(image);
    }

    // 释放资源
    av_packet_unref(m_AVPacket);
    return 0;
}

