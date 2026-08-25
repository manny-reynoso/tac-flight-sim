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

  Vector3 origin = {0.0f, 0.0f, 0.0f}; // marks origin of 3d scene

  DisableCursor();  // Limits cursor relative to movement inside the window
  SetTargetFPS(60); // Sets Target Frames per second

  while (!WindowShouldClose()) {

    UpdateCamera(&camera, CAMERA_FREE);

    if (IsKeyPressed(KEY_Z))
      camera.target = (Vector3){origin};
    BeginDrawing();

    ClearBackground(WHITE);

    BeginMode3D(camera);
    DrawGrid(25, 1.0f);
    DrawCube(origin, 5.0f, 5.0f, 5.0f, RED);

    EndMode3D();

    DrawFPS(10, 10);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
