#include <raylib.h>

int main() {
  int screenWidth{1280};
  int screenHeight{720};

  InitWindow(screenWidth, screenHeight, "tac-flight-sim");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
