#include <raylib.h>

#include "rlgl.h"
#define RLIGHTS_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "rlights.h"
#pragma GCC diagnostic pop

struct SceneAssets {
  Model model;
  Shader shader;
  Light lights[MAX_LIGHTS];
  int viewPosLoc;
};

struct Aircraft {

  Vector3 position{0.0f, 5.0f, 0.0f};
  float yaw{0.0f};
  float pitch{0.0f};
  float roll{0.0f};
  float length{4.0f};
  float width{1.0f};
  float height{0.5};
  Vector3 wingTipLeft{2.5f, 5.0f, 2.5f};
  Vector3 wingRootFrontLeft{0.0f, 5.0f, 1.5f};
  Vector3 wingRootBackLeft{0.0f, 5.0f, -2.5f};

  Vector3 wingTipRight{-2.5f, 5.0f, 2.5f};
  Vector3 wingRootFrontRight{0.0f, 5.0f, 1.5f};
  Vector3 wingRootBackRight{0.0f, 5.0f, -2.5f};
};

constexpr float speed = 10.0f;
constexpr float rotationSpeed = 30.0f;

void UpdateAircraft(Aircraft &aircraft) {

  // Z axis
  if (IsKeyDown(KEY_W)) {

    aircraft.position.z += speed * GetFrameTime();
  };

  if (IsKeyDown(KEY_S)) {

    aircraft.position.z -= speed * GetFrameTime();
    ;
  };

  // X axis
  if (IsKeyDown(KEY_D)) {

    aircraft.position.x += speed * GetFrameTime();
  };

  if (IsKeyDown(KEY_A)) {

    aircraft.position.x -= speed * GetFrameTime();
    ;
  };

  // Z axis
  if (IsKeyDown(KEY_SPACE)) {

    aircraft.position.y += speed * GetFrameTime();
    ;
  };

  if (IsKeyDown(KEY_LEFT_CONTROL)) {

    aircraft.position.y -= speed * GetFrameTime();
    ;
  };

  // PITCH

  if (IsKeyDown(KEY_UP)) {
    aircraft.pitch -= rotationSpeed * GetFrameTime();
  };

  if (IsKeyDown(KEY_DOWN)) {
    aircraft.pitch += rotationSpeed * GetFrameTime();
  };

  // ROLL
  if (IsKeyDown(KEY_RIGHT)) {
    aircraft.roll -= rotationSpeed * GetFrameTime();
  };

  if (IsKeyDown(KEY_LEFT)) {
    aircraft.roll += rotationSpeed * GetFrameTime();
  };

  // YAW

  if (IsKeyDown(KEY_E)) {
    aircraft.yaw += rotationSpeed * GetFrameTime();
  };

  if (IsKeyDown(KEY_Q)) {
    aircraft.yaw -= rotationSpeed * GetFrameTime();
  };
  // TODO Make it read input to manipulate aircraft YAW ROLL PITCH
};
// Load Scene
SceneAssets LoadScene() {

  SceneAssets LoadResult{};
  // Load 3D model

  // Load Shader

  // Load lights
  //

  return LoadResult;
};

// Draw Scene
void DrawScene(const SceneAssets, const Camera3D &, const Aircraft &aircraft) {

  // model Position
  DrawGrid(100, 1.0f);

  // DRAW PlaceHolder

  rlPushMatrix();
  rlTranslatef(aircraft.position.x, aircraft.position.y, aircraft.position.z);

  rlRotatef(aircraft.yaw, 0.0f, 1.0f, 0.0f);
  rlRotatef(aircraft.pitch, 1.0f, 0.0f, 0.0f);
  rlRotatef(aircraft.roll, 0.0f, 0.0f, 1.0f);

  DrawCube(Vector3{0.0f, 0.0f, 0.0f}, aircraft.width, aircraft.height,
           aircraft.length, RED);
  DrawCubeWires(Vector3{0.0f, 0.0f, 0.0f}, aircraft.width + 0.01,
                aircraft.height + 0.01, aircraft.length + 0.01, BLACK);

  rlPopMatrix();
};

int main() {
  int screenWidth{1280};
  int screenHeight{720};

  Vector3 origin = {0.0f, 0.0f, 0.0f}; // marks origin of 3d scene

  // Initialize Window
  InitWindow(screenWidth, screenHeight, "tac-flight-sim");
  Aircraft aircraft{};
  SceneAssets scene = LoadScene();

  // 3D Camera

  Camera3D camera = {};
  camera.position = {0.0f, 10.0f, 10.0f}; // camera postion
  camera.target = {0.0f, 0.0f, 0.0f};     // Camera looks at point
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;                    // Camera FOV Y tac-flight-sim
  camera.projection = CAMERA_PERSPECTIVE; // Camera mode type
  DisableCursor();  // Limits cursor relative to movement inside the window
  SetTargetFPS(60); // Sets Target Frames per second

  while (!WindowShouldClose()) {

    if (IsKeyDown(KEY_LEFT_ALT)) {
      UpdateCamera(&camera, CAMERA_FREE);
    } else {
      UpdateAircraft(aircraft);
    }

    if (IsKeyPressed(KEY_Z))
      camera.target = origin;

    BeginDrawing();

    ClearBackground(WHITE);

    BeginMode3D(camera);

    DrawScene(scene, camera, aircraft);
    EndMode3D();

    DrawFPS(10, 10);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
// Read through code
