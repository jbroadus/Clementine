#ifndef TELNET_PLAYER_H
#define TELNET_PLAYER_H

#include "interfaces/player.h"

namespace TelnetPlugin {
class Player : public IClementine::Player {
  Q_OBJECT
 public:
  Player(QObject* parent) : IClementine::Player(parent) {}

  void Playing() override;
  void Paused() override;
  void Stopped() override;

 signals:
  void MessageOut(const QString& msg);
};
};

#endif  // TELNET_PLAYER_H
