#ifndef SOUND_H
#define SOUND_H

#include <QWidget>
#include <QAudioInput>
#include <QAudioOutput>
#include <QFile>
#include <QIODevice>
#include <QDebug>
#include <QTimer>
#include <QTcpSocket>
#include <QTcpServer>
#include <QAudioDeviceInfo>
namespace Ui {
class Sound;
}

class Sound : public QWidget
{
    Q_OBJECT

public:
    explicit Sound(QWidget *parent = 0);
    ~Sound();

private slots:
   // void on_pushButton_toggled(bool checked);
    void micStateChanged(QAudio::State);
    void telStateChanged(QAudio::State);
    void on_recordPushButton_toggled(bool checked);
    void stopRecording();
    void on_record5secPushButton_toggled(bool checked);

    void on_connectPushButton_toggled(bool checked);
    void socketConnected();
    void socketError(QAbstractSocket::SocketError);
    void serverConnected();
    void connectionClosed();
    void monitorMicBuffer();
    void monitorTelBuffer();
    void monitorRxTxEhtBuffers();
    void readFromSocket();

    void on_listenPushButton_toggled(bool checked);

    void on_OnMicPushButton_toggled(bool checked);


    void on_OnTelPushButton_toggled(bool checked);

private:
    Ui::Sound *ui;
    QFile destinationFile;
    QAudioInput* mic;
    QAudioOutput *tel;
    QIODevice *telBuf;
    QTcpSocket socket, *activ_socket_ptr;
    QTcpServer server;
    QTimer micBufMonitorTimer, telBufMonitorTimer, RxTxEthBufMonitor;
    QList<QAudioDeviceInfo> audioOutputs, audioInputs;
};

#endif // SOUND_H
