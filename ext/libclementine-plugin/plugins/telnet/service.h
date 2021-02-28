#include <QObject>

#include "base/servicebase.h"

class QTcpServer;
class QTcpSocket;

namespace TelnetPlugin {

  class Player;

  class Service : public ServiceBase {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.clementine-player.plugins.Telnet")

  public:
    static const char* kSettingsGroup;
    static const char* kSettingPort;
    static const int kDefaultPort;

    Service();

    IClementine::Player* GetPlayerInterface() override;

  private:
    const QString GetName() override { return "Telnet Example"; };
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
    // Interface implementations
    Player* player_;
  };
};
