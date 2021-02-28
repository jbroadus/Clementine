#include "service.h"
#include "player.h"

#include <core/logging.h>

#include <QSettings>
#include <QTcpServer>
#include <QTcpSocket>

const char* TelnetPlugin::Service::kSettingsGroup = "Telnet Plugin";
const char* TelnetPlugin::Service::kSettingPort = "port";
const int TelnetPlugin::Service::kDefaultPort = 1234;

TelnetPlugin::Service::Service()
  : ServiceBase(),
    server_(new QTcpServer(this)),
    player_(new Player(this)) {}

bool TelnetPlugin::Service::Start() {
  qLog(Debug) << "Start";

  QSettings s;
  s.beginGroup(kSettingsGroup);
  int port = s.value(kSettingPort, kDefaultPort).toInt();
  
  connect(player_, SIGNAL(MessageOut(QString)),
          this, SIGNAL(MessageOut(QString)));
  connect(server_, SIGNAL(newConnection()), SLOT(NewConnection()));
  server_->listen(QHostAddress::Any, port);
  return true;
}

bool TelnetPlugin::Service::Stop() {
  qLog(Debug) << "Stop";
  return true;
}

IClementine::Player* TelnetPlugin::Service::GetPlayerInterface() {
  return player_;
}

void TelnetPlugin::Service::ReadSocket(QTcpSocket *socket) {
  while (socket->canReadLine()) {
    QString line = QString::fromLocal8Bit(socket->readLine()).trimmed();
    Process(socket, line);
  }
}

void TelnetPlugin::Service::Process(QTcpSocket *socket, const QString& input) {
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

void TelnetPlugin::Service::NewConnection() {
  qLog(Debug) << "NewConnection";
  QTcpSocket *socket;
  while ((socket = server_->nextPendingConnection()) != nullptr) {
    socket->write("hi\n");
    connect(this, &TelnetPlugin::Service::MessageOut,
            [socket](const QString& msg) { socket->write(msg.toLocal8Bit()); });
    connect(socket, &QIODevice::readyRead,
            [socket,this]() { ReadSocket(socket); });
  }
}
