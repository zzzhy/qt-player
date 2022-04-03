#ifndef RTSP_PLAYER_H
#define RTSP_PLAYER_H

#include "ffmpegutils.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

class RTSP_Player : public QThread
{
    Q_OBJECT
public:
     ~RTSP_Player() override;
    enum PlayerState {
        FFmpegInitFail = 0,
        FFmpegInitSucc,
        FFmpegStoped
    };
    enum State
    {
        Stoped,     ///<停止状态，包括从未启动过和启动后被停止
        Running,    ///<运行状态
        Paused      ///<暂停状态
    };

    State state() const;

    explicit RTSP_Player(FFmpegUtils *ffmpeg = nullptr, QObject *parent = nullptr);
    void SetPlayerUrl(QString playerUrl);

signals:
    void SigOpenUrlResult(int result);
protected:
    virtual void run() override final;

public slots:
    void start(Priority pri = InheritPriority);
    void stop();
    void pause();
    void resume();

private:
    QString         m_playerUrl;			// 播放url
    FFmpegUtils     *m_ffmpeg;				// MyFFmepg指针
    std::atomic_bool pauseFlag = false;
    std::atomic_bool stopFlag = false;
    std::atomic_bool initFlag = false;
    QMutex mutex;
    QWaitCondition condition;

    void play();
};

#endif // RTSP_PLAYER_H

