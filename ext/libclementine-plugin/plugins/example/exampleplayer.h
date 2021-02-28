#include "interfaces/player.h"

#ifndef EXAMPLE_PLAYER_H
#define EXAMPLE_PLAYER_H

#include "interfaces/player.h"

class ExamplePlayer : public IClementine::Player {
  Q_OBJECT
 public:
  ExamplePlayer(QObject* parent) : IClementine::Player(parent) {}

  void Playing() override;
  void Paused() override;
  void Stopped() override;

 signals:
  void MessageOut(const QString& msg);
};

#endif  // EXAMPLE_PLAYER_H
