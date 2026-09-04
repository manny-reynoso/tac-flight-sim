#include <raylib.h>
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

// Load Scene
SceneAssets LoadScene() {

  SceneAssets LoadResult{};
  // Load 3D model

  Model model = LoadModel("assets/models/race.glb");
  LoadResult.model = model;
  if (model.meshCount <= 0) {
    TraceLog(LOG_ERROR, "Model did not load...");

    return LoadResult;
  }
  // Load Shader
  Shader shader =
      LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
  LoadResult.shader = shader;
  if (shader.id == 0) {
    TraceLog(LOG_ERROR, "Shader did not load and model rendered flat instead");

    return LoadResult;
  };
  LoadResult.model.materials[0].shader = shader;

  // Load lighting

  LoadResult.lights[0] = CreateLight(LIGHT_POINT, {10, 10, -5}, {0, 0, 0},
                                     YELLOW, LoadResult.shader);
  int ambientLoc = GetShaderLocation(LoadResult.shader, "ambient");
  float ambientVal[4] = {0.1f, 0.1f, 0.1f, 0.1f};
  SetShaderValue(LoadResult.shader, ambientLoc, ambientVal,
                 SHADER_UNIFORM_VEC4);

  LoadResult.viewPosLoc = GetShaderLocation(shader, "viewPos");

  // Load Camera View pos

  return LoadResult;
};

// Draw Scene
void DrawScene(const SceneAssets &scene, const Camera3D &) {

  Vector3 cubePos = {0.0f, 2.5f, 0.0f}; // Position coordinates for Cube

  // model Position
  Vector3 modelPos = {5.0f, 0.0f, 0.0f};

  DrawGrid(25, 1.0f);
  DrawCube(cubePos, 5.0f, 5.0f, 5.0f, RED);
  DrawModel(scene.model, modelPos, 5.0f, WHITE);
};

void UpdateScene(SceneAssets &scene, const Camera3D &camera) {

  SetShaderValue(scene.shader, scene.viewPosLoc, &camera.position,
                 SHADER_UNIFORM_VEC3);
};

void UnloadScene(SceneAssets &scene) {

  UnloadModel(scene.model);
  UnloadShader(scene.shader);
};

int main() {
  int screenWidth{1280};
  int screenHeight{720};

  Vector3 origin = {0.0f, 0.0f, 0.0f}; // marks origin of 3d scene

  // Initialize Window
  InitWindow(screenWidth, screenHeight, "tac-flight-sim");
  SceneAssets scene = LoadScene();
  if (!IsModelValid(scene.model) || !IsShaderValid(scene.shader)) {
    CloseWindow();
    return 1;
  }

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

    UpdateCamera(&camera, CAMERA_FREE);

    if (IsKeyPressed(KEY_Z))
      camera.target = origin;

    UpdateScene(scene, camera);
    BeginDrawing();

    ClearBackground(WHITE);

    BeginMode3D(camera);

    DrawScene(scene, camera);
    EndMode3D();

    DrawFPS(10, 10);
    EndDrawing();
  }

  UnloadScene(scene);
  CloseWindow();

  return 0;
}

// Read through code
