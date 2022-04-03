#include "rtsp_player.h"
#include <QThread>
#include <QDebug>

RTSP_Player::RTSP_Player(FFmpegUtils *ffmpeg, QObject *parent)
    : QThread{parent},
      pauseFlag(false),
      stopFlag(false)
    , m_ffmpeg(ffmpeg)
{

}
RTSP_Player::~RTSP_Player() {
    stop();
}
RTSP_Player::State RTSP_Player::state() const
{
    State s = Stoped;
    if (!QThread::isRunning())
    {
        s = Stoped;
    }
    else if (QThread::isRunning() && pauseFlag)
    {
        s = Paused;
    }
    else if (QThread::isRunning() && (!pauseFlag))
    {
        s = Running;
    }
    return s;
}
void RTSP_Player::start(Priority pri)
{
    stopFlag = false;
    pauseFlag = false;

    QThread::start(pri);
}

void RTSP_Player::stop()
{
    if (QThread::isRunning())
    {
        initFlag = false;
        stopFlag = true;
        pauseFlag = false;
        condition.wakeAll();
        QThread::quit();
        QThread::wait();
    }
}

void RTSP_Player::pause()
{
    if (QThread::isRunning())
    {
        pauseFlag = true;
        stopFlag = false;
    }
}

void RTSP_Player::resume()
{
    if (QThread::isRunning())
    {
        pauseFlag = false;
        stopFlag = false;
        condition.wakeAll();
    }
}

void RTSP_Player::run()
{
    qDebug() << "enter thread : " << QThread::currentThreadId();

    qDebug() << "pauseFlag: " << pauseFlag;
    qDebug() << "stopFlag: " << stopFlag;

    while (!stopFlag)
    {
        if (!initFlag) {
            m_ffmpeg->MyFFmpegSetUrl(m_playerUrl);
            m_ffmpeg->MyFFmpegInit();
            initFlag = true;
        }
        int ret = m_ffmpeg->MyFFmpepReadFrame();
        if (ret < 0) break;
        if (pauseFlag)
        {
            mutex.lock();
            condition.wait(&mutex);
            mutex.unlock();
        }
    }
    pauseFlag = false;
    stopFlag = true;
    initFlag = false;
    qDebug() << "exit thread : " << QThread::currentThreadId();
}


void RTSP_Player::SetPlayerUrl(QString playerUrl)
{
    this->m_playerUrl = playerUrl;
}


