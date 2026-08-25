#include <raylib.h>

int main() {
  int screenWidth{1280};
  int screenHeight{720};
  InitWindow(screenWidth, screenHeight, "tac-flight-sim");

  // 3D Camera

  Camera3D camera = {0};
  camera.position = (Vector3){0.0f, 10.0f, 10.0f}; // camera postion
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};     // Camera looks at point
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;                    // Camera FOV Y tac-flight-sim
  camera.projection = CAMERA_PERSPECTIVE; // Camera mode type

  SetTargetFPS(60); // Sets Target Frames per second

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
