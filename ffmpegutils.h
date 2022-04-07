#ifndef MYFFMPEG_H
#define MYFFMPEG_H

#include <QObject>
#include <QImage>
#include <QMap>

#define __STDC_CONSTANT_MACORS

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libswscale/swscale.h"
#include "libavutil/frame.h"
}


class FFmpegUtils : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegUtils(QObject *parent = 0);
    ~FFmpegUtils();

    void MyFFmpegSetUrl(QString rtspUrl);

    int MyFFmpegInit();
    int MyFFmpepReadFrame();
signals:
    void MyFFmpegSigGetOneFrame(QImage img);

public slots:

private:
    int              m_videoIndex = -1;
    QMap<int, const AVCodec*> m_audioIndex;
    QString          m_rtspUrl;
    const AVCodec   *m_AVCodec;
    const AVCodecParameters *m_AVCodecParameters;
    AVFormatContext *m_AVFormatContext;
    AVCodecContext  *m_AVCodecContext;
    AVFrame         *m_AVFrame;
    AVFrame         *m_AVFrameRGB;
    AVPacket        *m_AVPacket;
    SwsContext      *m_SwsContext = NULL;
    AVPixelFormat ConvertDeprecatedFormat(enum AVPixelFormat format);
};

#endif // MYFFMPEG_H

