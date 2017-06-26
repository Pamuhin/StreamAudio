#include "sound.h"
#include "ui_sound.h"

#define RX_ETH_BUF_LENGTH 102400

Sound::Sound(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Sound)
{
    ui->setupUi(this);
    connect(&socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));

    connect(&server, SIGNAL(newConnection()), this, SLOT(serverConnected()));

    micBufMonitorTimer.setSingleShot(false);
    micBufMonitorTimer.setInterval(100);
    connect(&micBufMonitorTimer, SIGNAL(timeout()), this, SLOT(monitorMicBuffer()));

    telBufMonitorTimer.setSingleShot(false);
    telBufMonitorTimer.setInterval(100);
    connect(&telBufMonitorTimer, SIGNAL(timeout()), this, SLOT(monitorTelBuffer()));

    RxTxEthBufMonitor.setSingleShot(false);
    RxTxEthBufMonitor.setInterval(100);
    connect(&RxTxEthBufMonitor, SIGNAL(timeout()), this, SLOT(monitorRxTxEhtBuffers()));

    ui->RxEthBufUsageProgressBar->setMaximum(RX_ETH_BUF_LENGTH);

    activ_socket_ptr = NULL;
    tel = NULL;
    mic = NULL;


    audioOutputs = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    foreach (const QAudioDeviceInfo &deviceInfo, audioOutputs){
        ui->logTextEdit->append( "Output device name: " + deviceInfo.deviceName());
        ui->audioOutputsComboBox->addItem(deviceInfo.deviceName());
    }
    audioOutputs.insert(0, QAudioDeviceInfo::defaultOutputDevice());

    audioInputs = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach (const QAudioDeviceInfo &deviceInfo, audioInputs){
        ui->logTextEdit->append( "Input device name: " + deviceInfo.deviceName());
        ui->audioInputsComboBox->addItem(deviceInfo.deviceName());
    }
    audioInputs.insert(0, QAudioDeviceInfo::defaultInputDevice());
}
/**********************************************************************************/

Sound::~Sound()
{
    delete ui;
}
/**********************************************************************************/

void Sound::micStateChanged(QAudio::State newState)
{
    switch (newState) {
              case QAudio::StoppedState:
                  if (mic->error() != QAudio::NoError) {
                      ui->logTextEdit->append("Mic error");
                  } else {
                      ui->logTextEdit->append("Mic stoped");
                  }
                  break;

              case QAudio::ActiveState:
                  ui->logTextEdit->append("Mic started");
                  break;

              default:
                  ui->logTextEdit->append("Mic other cases as appropriate");
                  break;
          }

}
/**********************************************************************************/

void Sound::telStateChanged(QAudio::State newState)
{
    switch (newState) {
        case QAudio::IdleState:
            ui->logTextEdit->append("Tel in idle");
            break;

        case QAudio::StoppedState:
            // Stopped for other reasons
            if (tel->error() != QAudio::NoError) {
                ui->logTextEdit->append("Tel error");
            }else{
                ui->logTextEdit->append("Tel stoped");
            }
            break;
        case QAudio::ActiveState:
            ui->logTextEdit->append("Tel started");
            break;
        default:
            ui->logTextEdit->append("Tel other cases as appropriate");
            break;
    }
}
/**********************************************************************************/

void Sound::on_recordPushButton_toggled(bool checked)
{
    /*if(checked){
        destinationFile.setFileName(ui->filenameLineEdit->text());
        destinationFile.open( QIODevice::WriteOnly | QIODevice::Truncate );

        QAudioFormat format;
        // Set up the desired format, for example:
        format.setSampleRate(ui->sampleRateSpinBox->value());
        format.setChannelCount(ui->channelsNumberComboBox->currentText().toInt());
        format.setSampleSize(ui->resolutionComboBox->currentText().toInt());
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

//        qDebug() << "sample rate = " << format.sampleRate();
//        qDebug() << "channels count = " << format.channelCount();
//        qDebug() << "sample size = " << format.sampleSize();
//        qDebug() << "codec = " << format.codec();
//        qDebug() << "byte order = " << format.byteOrder();
//        qDebug() << "sample type = " << format.sampleType();

        QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(format)) {
            qWarning() << "Default format not supported, trying to use the nearest.";
            format = info.nearestFormat(format);
            qDebug() << "sample rate = " << format.sampleRate();
            qDebug() << "channels count = " << format.channelCount();
            qDebug() << "sample size = " << format.sampleSize();
            qDebug() << "codec = " << format.codec();
            qDebug() << "byte order = " << format.byteOrder();
            qDebug() << "sample type = " << format.sampleType();
        }

        audio = new QAudioInput(format, this);
        connect(audio, SIGNAL(stateChanged(QAudio::State)), this, SLOT(handleStateChanged(QAudio::State)));

        //QTimer::singleShot(3000, this, SLOT(stopRecording()));
        audio->start(&destinationFile);
        // Records audio for 3000ms
    }else{
        audio->stop();
        destinationFile.close();
        delete audio;
    }*/
}
/**********************************************************************************/

void Sound::on_record5secPushButton_toggled(bool checked)
{
    on_recordPushButton_toggled(checked);
    QTimer::singleShot(5000, this, SLOT(stopRecording()));
}
/**********************************************************************************/

void Sound::stopRecording()
{
    ui->record5secPushButton->setChecked(false);
}
/**********************************************************************************/

void Sound::on_connectPushButton_toggled(bool checked)
{
    if(checked){
        socket.connectToHost(QHostAddress(ui->IPLineEdit->text()), ui->portLineEdit->text().toInt());
        ui->logTextEdit->append("Connecting...");
        ui->connectPushButton->setText("Disconnect");
        ui->listenPushButton->setEnabled(false);
    }else{
        socket.disconnectFromHost();
        ui->connectPushButton->setText("Connect");
        ui->logTextEdit->append("Connection closed");
        ui->listenPushButton->setEnabled(true);
    }
}
/**********************************************************************************/

void Sound::on_listenPushButton_toggled(bool checked)
{
    if(checked){
        if(!server.listen(QHostAddress(ui->IPLineEdit->text()), ui->portLineEdit->text().toInt())){
            ui->logTextEdit->append("TCP Server initialization error");
        }
        ui->logTextEdit->append("Listenig...");
        ui->listenPushButton->setText("Disconnect");
        ui->connectPushButton->setEnabled(false);
    }else{
        server.close();
        if(activ_socket_ptr != NULL)
            activ_socket_ptr->disconnectFromHost();
        ui->listenPushButton->setText("Listen");
        ui->logTextEdit->append("Connection closed");
        ui->connectPushButton->setEnabled(true);
    }
}
/**********************************************************************************/


void Sound::socketConnected()
{
    ui->logTextEdit->append("Socket: Connection established");
    activ_socket_ptr = &socket;
    activ_socket_ptr->setReadBufferSize(RX_ETH_BUF_LENGTH);
    ui->telGroupBox->setEnabled(true);
    ui->micGroupBox->setEnabled(true);
    connect(activ_socket_ptr, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
    connect(activ_socket_ptr, SIGNAL(readyRead()), this, SLOT(readFromSocket()));
    RxTxEthBufMonitor.start();
}
/**********************************************************************************/

void Sound::serverConnected()
{
    ui->logTextEdit->append("Server: Connection established");
    activ_socket_ptr = server.nextPendingConnection();
    activ_socket_ptr->setReadBufferSize(RX_ETH_BUF_LENGTH);
    ui->telGroupBox->setEnabled(true);
    ui->micGroupBox->setEnabled(true);
    connect(activ_socket_ptr, SIGNAL(disconnected()), this, SLOT(connectionClosed()));
    connect(activ_socket_ptr, SIGNAL(readyRead()), this, SLOT(readFromSocket()));
    RxTxEthBufMonitor.start();
}
/**********************************************************************************/

void Sound::socketError(QAbstractSocket::SocketError e)
{
    ui->logTextEdit->append("Socked error: " + socket.errorString());
}
/**********************************************************************************/

void Sound::connectionClosed()
{
    ui->telGroupBox->setEnabled(false);
    ui->micGroupBox->setEnabled(false);
    RxTxEthBufMonitor.stop();
}
/**********************************************************************************/

void Sound::on_OnMicPushButton_toggled(bool checked)
{
    if(checked){
        //ui->telGroupBox->setEnabled(false);
        QAudioFormat format;
        format.setSampleRate(ui->sampleRateSpinBox->value());
        format.setChannelCount(ui->channelsNumberComboBox->currentText().toInt());
        format.setSampleSize(ui->resolutionComboBox->currentText().toInt());
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        QAudioDeviceInfo info(audioInputs.at(ui->audioInputsComboBox->currentIndex()));
        if (!info.isFormatSupported(format)) {
            ui->logTextEdit->append("Mic format not supported, trying to use the nearest.");
            format = info.nearestFormat(format);
            ui->logTextEdit->append("sample rate = " + QString::number(format.sampleRate()));
            ui->logTextEdit->append("channels count = " + QString::number(format.channelCount()));
            ui->logTextEdit->append("sample size = " + QString::number(format.sampleSize()));
            ui->logTextEdit->append("codec = " + format.codec());
            //ui->logTextEdit->append("byte order = " + format.byteOrder());
            //ui->logTextEdit->append("sample type = " + format.sampleType());
            //return;
        }

        mic = new QAudioInput(info, format, this);
        connect(mic, SIGNAL(stateChanged(QAudio::State)), this, SLOT(micStateChanged(QAudio::State)));
        mic->start(activ_socket_ptr);
        ui->micBufferUsageProgressBar->setMaximum(mic->bufferSize());
        micBufMonitorTimer.start();
    }else{
        //ui->telGroupBox->setEnabled(true);
        mic->stop();
        delete mic;
        micBufMonitorTimer.stop();
    }
}
/**********************************************************************************/

void Sound::on_OnTelPushButton_toggled(bool checked)
{
    if(checked){
        //ui->micGroupBox->setEnabled(false);
        QAudioFormat format;
        format.setSampleRate(ui->sampleRateSpinBox_2->value());
        format.setChannelCount(ui->channelsNumberComboBox_2->currentText().toInt());
        format.setSampleSize(ui->resolutionComboBox_2->currentText().toInt());
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        QAudioDeviceInfo info(audioOutputs.at(ui->audioOutputsComboBox->currentIndex()));
          if (!info.isFormatSupported(format)) {
              ui->logTextEdit->append("Tel format not supported, trying to use the nearest.");
              format = info.nearestFormat(format);
              ui->logTextEdit->append("sample rate = " + format.sampleRate());
              ui->logTextEdit->append("channels count = " + format.channelCount());
              ui->logTextEdit->append("sample size = " + format.sampleSize());
              ui->logTextEdit->append("codec = " + format.codec());
              ui->logTextEdit->append("byte order = " + format.byteOrder());
              ui->logTextEdit->append("sample type = " + format.sampleType());
              //return;
          }

          tel = new QAudioOutput(info, format, this);
          tel->setBufferSize(ui->sampleRateSpinBox_2->value());/////////////////////////////////////////////////////////////
          connect(tel, SIGNAL(stateChanged(QAudio::State)), this, SLOT(telStateChanged(QAudio::State)));
          telBuf = tel->start();

          qDebug()<<"buffer = " << tel->bufferSize() << ", period = " << tel->periodSize();/////////////////////////////
          tel->periodSize();/////////////////////////////////////////////////////////////////////////////////////////////

          ui->telBufferUsageProgressBar->setMaximum(tel->bufferSize());
          telBufMonitorTimer.start();
    }else{
        //ui->micGroupBox->setEnabled(true);
        tel->stop();
        tel = NULL;
        delete tel;
        telBufMonitorTimer.stop();
    }
}
/**********************************************************************************/

void Sound::readFromSocket()
{
    QByteArray data = activ_socket_ptr->readAll();
    if((tel != NULL) && (tel->state() != QAudio::StoppedState)){
        telBuf->write(data);
    }
}
/**********************************************************************************/

void Sound::monitorRxTxEhtBuffers()
{
    ui->RxEthBufUsageProgressBar->setValue(activ_socket_ptr->bytesAvailable());
    ui->TxEthBufUsageProgressBar->setValue(activ_socket_ptr->bytesToWrite());
}
/**********************************************************************************/

void Sound::monitorMicBuffer()
{
    ui->micBufferUsageProgressBar->setValue(mic->bytesReady());
}
/**********************************************************************************/

void Sound::monitorTelBuffer()
{
    ui->telBufferUsageProgressBar->setValue(tel->bufferSize() - tel->bytesFree());
}
/**********************************************************************************/
