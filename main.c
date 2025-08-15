/********************************************************************************************
 *
 *  This jam game uses the raylib vscode example by @raysan5
 *
 * ********************************************************************************************/

#include "raylib.h"
#include <stdlib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables (needed for web platform)
//----------------------------------------------------------------------------------
const int screenWidth = 800;
const int screenHeight = 450;
Texture2D spriteSheet;           // The sprite sheet
unsigned int rows = 32;          // rows in the sprite sheet
unsigned int currentFrame = 0;   // Currant frame
unsigned int framesCounter = 0;  // Frame counter
unsigned int framesSpeed = 8;    // Number of spritesheet frames shown by second
unsigned int numberOfClouds = 1; // Number as clouds
bool debugMode = false;          // Debug mode
bool gameRunning = true;

typedef struct {
  Vector2 position;   // Cloud position
  Rectangle frameRec; // Frame rectangle
  float happiness;    // Happiness for the clouds
  Rectangle hitbox;
} Cloud;

Cloud *clouds = NULL; // Dynamic array of clouds
int maxClouds = 100;  // Maximum number of clouds
int activeClouds = 0; // Current number of active clouds

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);    // Update and draw one frame
static void UpdateCloudSprites(void); // Update and draw cloud sprites
static void InitClouds(void);
static void AddCloud(Vector2 position);
static void DrawClouds(void);
static void UpdateDebugText(void); // Update and draw debug text
static void UpdateHappiness(void);
static void DrawGameOverScreen(void);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main() {
  // Initialization
  //--------------------------------------------------------------------------------------
  InitWindow(screenWidth, screenHeight, "raylib sprite animation");

  // Load sprite sheet texture
  spriteSheet = LoadTexture("resources/spritesheet.png");

  InitClouds();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    if (IsKeyPressed(KEY_ENTER)) {
      AddCloud((Vector2){rand() % (40 + (screenWidth - 40) + 1),
                         rand() % (40 + 200 + 1)});
    }
    if (IsKeyPressed(KEY_SLASH)) {
      if (!debugMode) {
        debugMode = true;
      } else {
        debugMode = false;
      }
    }

    UpdateHappiness();
    UpdateDrawFrame();
  }
#endif

  // De-Initialization
  //--------------------------------------------------------------------------------------
  // Free the clouds
  if (clouds != NULL) {
    free(clouds);
    clouds = NULL;
  }

  UnloadTexture(spriteSheet); // Unload texture
  CloseWindow();              // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

// Update and draw game frame
static void UpdateDrawFrame(void) {
  if (!gameRunning) {
    BeginDrawing();

    DrawGameOverScreen();

    EndDrawing();
  } else {
    // Update
    //----------------------------------------------------------------------------------
    UpdateCloudSprites();
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(SKYBLUE);

    // Draw the animated clouds
    DrawClouds();

    // Debug info
    if (debugMode) {
      UpdateDebugText();
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }
}

// Initialize cloud system
static void InitClouds(void) {
  if (clouds == NULL) {
    clouds = (Cloud *)malloc(maxClouds * sizeof(Cloud));
  }
  activeClouds = 0;
}

// Add a new cloud
static void AddCloud(Vector2 position) {
  if (activeClouds < maxClouds) {
    clouds[activeClouds].position = position;
    clouds[activeClouds].frameRec =
        (Rectangle){0.0f, 0.0f, (float)spriteSheet.width / 8,
                    (float)spriteSheet.height / rows};
    clouds[activeClouds].happiness = 0.5f; // Default happiness
    activeClouds++;
    numberOfClouds = activeClouds; // Update the counter
  }
}

// Update all clouds
static void UpdateCloudSprites(void) {
  framesCounter++;

  if (framesCounter >= (60 / framesSpeed)) {
    framesCounter = 0;
    currentFrame++;

    if (currentFrame > 7)
      currentFrame = 0;

    // Update all active clouds
    for (int i = 0; i < activeClouds; i++) {
      clouds[i].frameRec.x = (float)currentFrame * clouds[i].frameRec.width;
    }
  }
}

// Draw all clouds
static void DrawClouds(void) {
  for (int i = 0; i < activeClouds; i++) {
    // Color based on happiness
    if (clouds[i].happiness > 0.7f) {
      clouds[i].frameRec.y = 64.0f;
    } else if (clouds[i].happiness < 0.3f) {
      clouds[i].frameRec.y = 32.0f;
    } else {
      clouds[i].frameRec.y = 0.0f;
    }

    DrawTextureRec(spriteSheet, clouds[i].frameRec, clouds[i].position, WHITE);

    if (debugMode) {
      DrawText(TextFormat("Happiness: %.2f", clouds[i].happiness),
               clouds[i].position.x, clouds[i].position.y - 15, 10, DARKGRAY);

      // Draw cloud hitbox
      clouds[i].hitbox =
          (Rectangle){clouds[i].position.x, clouds[i].position.y,
                      clouds[i].frameRec.width, clouds[i].frameRec.height};
      DrawRectangleLines(clouds[i].hitbox.x, clouds[i].hitbox.y,
                         clouds[i].hitbox.width, clouds[i].hitbox.height, RED);
    }
  }
}

static void UpdateDebugText(void) {
  DrawFPS(10, 10);
  DrawText("Sprite Sheet Animation", 10, 40, 20, DARKGRAY);
  DrawText(TextFormat("Frame: %d", currentFrame), 10, 70, 20, DARKGRAY);
  DrawText(TextFormat("Number of clouds: %d", numberOfClouds), 10, 100, 20,
           DARKGRAY);
  DrawText("Press ENTER to add cloud, / for debug", 10, 130, 12, DARKGRAY);
  DrawText("Click and drag clouds to make them happy!", 10, 145, 12, DARKGRAY);
}

static void UpdateHappiness(void) {
  Vector2 mousePos = GetMousePosition();
  int pettingCloud = GetGestureDetected();

  // Update all active clouds
  for (int i = 0; i < activeClouds; i++) {
    // Create a hitbox for the cloud
    clouds[i].hitbox =
        (Rectangle){clouds[i].position.x, clouds[i].position.y,
                    clouds[i].frameRec.width, clouds[i].frameRec.height};

    // Check if mouse is over this cloud
    if (CheckCollisionPointRec(mousePos, clouds[i].hitbox)) {
      if (pettingCloud == GESTURE_DRAG) {
        // Mouse is pressed and over cloud - increase happiness
        clouds[i].happiness += 0.01f;
      }
    } else {
      // Mouse is not over cloud - slowly decrease happiness
      clouds[i].happiness -= 0.001f;
    }

    // Clamp happiness between 0 and 1
    if (clouds[i].happiness > 1.0f)
      clouds[i].happiness = 1.0f;
    if (clouds[i].happiness < 0.0f)
      clouds[i].happiness = 0.0f;
  }

  // Check game over condition: if 75% or more of clouds have happiness < 0.05
  if (activeClouds > 3) {
    int sadClouds = 0;
    for (int i = 0; i < activeClouds; i++) {
      if (clouds[i].happiness < 0.05f) {
        sadClouds++;
      }
    }

    // If 75% or more clouds are sad, end the game
    if (sadClouds >= activeClouds / 4) {
      gameRunning = false;
    }
  }
}

static void DrawGameOverScreen(void) {
  const char *text = "GAME OVER";
  int fontSize = 40;
  int textWidth = MeasureText(text, fontSize);

  DrawText(text, screenWidth / 2 - textWidth / 2,
           screenHeight / 2 - fontSize / 2, fontSize, RED);
}