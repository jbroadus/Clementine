#include "exampleplayer.h"

void ExamplePlayer::Playing() {
  emit MessageOut("playing\n");
}

void ExamplePlayer::Paused() {
  emit MessageOut("paused\n");
}

void ExamplePlayer::Stopped() {
  emit MessageOut("stopped\n");
}
