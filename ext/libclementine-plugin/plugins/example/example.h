#include <QObject>

#include "common/pluginbase.h"

class QTcpServer;
class QTcpSocket;
class ExamplePlayer;

class ExamplePlugin : public PluginBase {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.clementine-player.plugins.ExamplePlugin")

 public:
  ExamplePlugin();

 private:
  bool Start() override;
  bool Stop() override;
  void ReadSocket(QTcpSocket *socket);
  void Process(QTcpSocket *socket, const QString& input);

 signals:
  void MessageOut(const QString& msg);

 private slots:

  // Server
  void NewConnection();

 private:
  QTcpServer* server_;
  ExamplePlayer* player_;
};
