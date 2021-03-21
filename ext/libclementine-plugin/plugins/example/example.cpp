#include "example.h"
#include "exampleplayer.h"
#include "examplesettings.h"

#include <core/logging.h>

#include <QTcpServer>
#include <QTcpSocket>

ExamplePlugin::ExamplePlugin()
  : PluginBase(),
    server_(new QTcpServer(this)),
    player_(new ExamplePlayer(this)),
    settings_(new ExampleSettings(this)) {
  AddInterface(player_);
  AddInterface(settings_);
}

bool ExamplePlugin::Start() {
  qLog(Debug) << "Start";
  connect(player_, SIGNAL(MessageOut(QString)), this, SIGNAL(MessageOut(QString)));
  connect(server_, SIGNAL(newConnection()), SLOT(NewConnection()));
  server_->listen(QHostAddress::Any, 1234);
  return true;
}

bool ExamplePlugin::Stop() {
  qLog(Debug) << "Stop";
  return true;
}

void ExamplePlugin::ReadSocket(QTcpSocket *socket) {
  while (socket->canReadLine()) {
    QString line = QString::fromLocal8Bit(socket->readLine()).trimmed();
    Process(socket, line);
  }
}

void ExamplePlugin::Process(QTcpSocket *socket, const QString& input) {
  if (input.isEmpty()) {
    return;
  }

  QString cmd = input.toLower();
  if (cmd == "hi") {
    socket->write("hi\n");
  } else if (cmd == "play") {
    qLog(Debug) << "play";
    emit player_->Play();
    socket->write("ok\n");
  } else if (cmd == "pause") {
    qLog(Debug) << "pause";
    emit player_->Pause();
    socket->write("ok\n");
  } else if (cmd == "stop") {
    qLog(Debug) << "stop";
    emit player_->Stop();
    socket->write("ok\n");
  } else {
    qLog(Debug) << "Unknown command:" << input;
    socket->write("error\n");
  }
}

void ExamplePlugin::NewConnection() {
  qLog(Debug) << "NewConnection";
  QTcpSocket *socket;
  while ((socket = server_->nextPendingConnection()) != nullptr) {
    socket->write("hi\n");
    connect(this, &ExamplePlugin::MessageOut,
            [socket](const QString& msg) { socket->write(msg.toLocal8Bit()); });
    connect(socket, &QIODevice::readyRead,
            [socket,this]() { ReadSocket(socket); });
  }
}
