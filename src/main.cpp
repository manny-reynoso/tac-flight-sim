#include <raylib.h>
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

struct SceneAssets {
  Model model;
  Shader shader;
  Light lights[MAX_LIGHTS];
};

// Load Scene
SceneAssets LoadScene() {

  SceneAssets LoadResult{};
  // Load 3D model

  Model model = LoadModel("assets/models/race.glb");
  LoadResult.model = model;
  if (model.meshCount <= 0) {
    TraceLog(LOG_ERROR, "Model did not load...");

    CloseWindow();
    return LoadResult;
  }
  // Load Shader
  Shader shader =
      LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
  LoadResult.shader = shader;
  LoadResult.model.materials[0].shader = shader;
  if (shader.id == 0) {
    TraceLog(LOG_ERROR, "Shader did not load and model rendered flat instead");
    CloseWindow();
    return LoadResult;
  };

  return LoadResult;
};

int main() {
  int screenWidth{1280};
  int screenHeight{720};

  // Initialize Window
  InitWindow(screenWidth, screenHeight, "tac-flight-sim");
  SceneAssets scene = LoadScene();

  void UpdateScene(SceneAssets &, const Camera3D &);
  void DrawScene(const SceneAssets &, const Camera3D &);
  void UnloadScene(SceneAssets &);

  // model Position
  Vector3 modelPos = {5.0f, 0.0f, 0.0f};

  // 3D Camera

  Camera3D camera = {};
  camera.position = {0.0f, 10.0f, 10.0f}; // camera postion
  camera.target = {0.0f, 0.0f, 0.0f};     // Camera looks at point
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;                    // Camera FOV Y tac-flight-sim
  camera.projection = CAMERA_PERSPECTIVE; // Camera mode type

  Vector3 origin = {0.0f, 0.0f, 0.0f};  // marks origin of 3d scene
  Vector3 cubePos = {0.0f, 2.5f, 0.0f}; // Position coordinates for Cube

  DisableCursor();  // Limits cursor relative to movement inside the window
  SetTargetFPS(60); // Sets Target Frames per second

  while (!WindowShouldClose()) {

    UpdateCamera(&camera, CAMERA_FREE);

    if (IsKeyPressed(KEY_Z))
      camera.target = origin;
    BeginDrawing();

    ClearBackground(WHITE);

    BeginMode3D(camera);
    DrawGrid(25, 1.0f);
    DrawCube(cubePos, 5.0f, 5.0f, 5.0f, RED);
    DrawModel(scene.model, modelPos, 5.0f, WHITE);

    EndMode3D();

    DrawFPS(10, 10);
    EndDrawing();
  }

  UnloadModel(scene.model);
  CloseWindow();

  return 0;
}

// Read through code
