#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
}
#include "rtsp_player.h"
#include "pauseable_thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void paintEvent(QPaintEvent *event);
private:
    Ui::MainWindow *ui;
    QImage m_image;
    FFmpegUtils *m_ffmpeg=nullptr;
    void MyFFmpegTest();
    RTSP_Player *m_player=nullptr;
    void PlayStop();

private slots:
    void SlotGetOneFrame(QImage img);

    void on_m_play_btn_clicked();

    void on_m_stop_btn_clicked();

signals:
    void SigPlayStart(QThread::Priority p);

};

#endif // MAINWINDOW_H
