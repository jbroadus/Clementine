#include "player.h"

void TelnetPlugin::Player::Playing() {
  emit MessageOut("playing\n");
}

void TelnetPlugin::Player::Paused() {
  emit MessageOut("paused\n");
}

void TelnetPlugin::Player::Stopped() {
  emit MessageOut("stopped\n");
}
