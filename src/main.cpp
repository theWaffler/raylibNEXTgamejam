#include "Game.h"

int main() {
  InitAudioDevice();
  Game game(800, 450);
  game.Run();
  CloseAudioDevice();
  return 0;
}
