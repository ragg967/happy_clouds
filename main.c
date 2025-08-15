/********************************************************************************************
 *
 *  This jam game uses the raylib vscode example by @raysan5
 *
 * ********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables (needed for web platform)
//----------------------------------------------------------------------------------
Texture2D spriteSheet;
Rectangle frameRec;
int rows = 32;         // rows in the sprite sheet
int currentFrame = 0;  // Currant frame
int framesCounter = 0; // Frame counter
int framesSpeed = 8;   // Number of spritesheet frames shown by second

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void); // Update and draw one frame
static void UpdateSprite(void);    // Update and draw one sprite
static void UpdateDebugText(void); // Update and draw debug text

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main() {
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "raylib sprite animation");

  // Load sprite sheet texture
  spriteSheet = LoadTexture("resources/spritesheet.png");

  frameRec = (Rectangle){0.0f, 0.0f, (float)spriteSheet.width / 8,
                         (float)spriteSheet.height / rows};

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    UpdateDrawFrame();
  }
#endif

  // De-Initialization
  //--------------------------------------------------------------------------------------
  UnloadTexture(spriteSheet); // Unload texture
  CloseWindow();              // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

// Update and draw game frame
static void UpdateDrawFrame(void) {
  // Update
  //----------------------------------------------------------------------------------
  UpdateSprite();
  //----------------------------------------------------------------------------------

  // Draw
  //----------------------------------------------------------------------------------
  BeginDrawing();

  ClearBackground(RAYWHITE);

  // Draw the animated sprites
  DrawTextureRec(spriteSheet, frameRec, (Vector2){350, 200}, WHITE);
  DrawTextureRec(spriteSheet, frameRec, (Vector2){350, 200}, WHITE);

  // Debug info
  UpdateDebugText();

  DrawFPS(10, 10);

  EndDrawing();
  //----------------------------------------------------------------------------------
}

static void UpdateSprite(void) {
  framesCounter++;

  if (framesCounter >= (60 / framesSpeed)) {
    framesCounter = 0;
    currentFrame++;

    // Reset to first frame after last frame
    if (currentFrame > 7)
      currentFrame = 0;

    // Update frame rectangle X position
    frameRec.x = (float)currentFrame * frameRec.width;
  }
}

static void UpdateDebugText(void) {
  DrawText("Sprite Sheet Animation", 10, 40, 20, DARKGRAY);
  DrawText(TextFormat("Frame: %i", currentFrame), 10, 70, 20, DARKGRAY);
}