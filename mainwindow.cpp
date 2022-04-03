#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_ffmpeg = new FFmpegUtils;

    m_player = new RTSP_Player(m_ffmpeg);
//    m_player->SetPlayerUrl("rtmp://58.200.131.2:1935/livetv/cctv3");
//    m_player->SetPlayerUrl("/Users/zzzhy/Downloads/DSC_4960.MOV");
    m_player->SetPlayerUrl("/Users/zzzhy/Downloads/bunny_1080p_60fps_265.mp4");
//    m_player->SetPlayerUrl("/Users/zzzhy/Downloads/out.avi");
//    connect(this,SIGNAL(SigPlayStart(QThread::Priority)), m_player, SLOT(start()));
    connect(m_ffmpeg, SIGNAL(MyFFmpegSigGetOneFrame(QImage)), this, SLOT(SlotGetOneFrame(QImage)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::SlotGetOneFrame(QImage img){
    m_image = img;
    this->update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (m_image.size().width() <= 0)
        return;


    QImage img = m_image.scaled(ui->labVideo->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    int x = ui->labVideo->geometry().x();
    int y = ui->labVideo->geometry().y();
    painter.drawImage(QPoint(x, y), img);
}
void MainWindow::PlayStop()
{
    if (m_player)
    {
        m_player->quit();
        m_player->wait(1000);
        delete m_player;
        m_player = nullptr;
    }
}

void MainWindow::on_m_play_btn_clicked()
{
    m_player->start();
}


void MainWindow::on_m_stop_btn_clicked()
{
    RTSP_Player::State s = m_player->state();
    switch (s) {
    case RTSP_Player::State::Paused:
        m_player->resume();
        break;
    case RTSP_Player::State::Running:
        m_player->pause();
        break;
    case RTSP_Player::State::Stoped:
        m_player->start();
        break;
    default:
        break;
    }
}

