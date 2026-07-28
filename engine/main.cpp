#include "engine/engine.h"
#include "engine/game.h"

int main() {
  se::Engine e;
  return e.Run({}, se::CreateGame());
}
