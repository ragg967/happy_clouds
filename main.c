/********************************************************************************************
 *
 *  Cloud Happiness Game - A Raylib-based interactive game
 *  Players must keep clouds happy by petting them with mouse drag gestures
 *  Game over occurs when too many clouds become sad
 *
 *  This jam game uses the raylib vscode example by @raysan5
 *
 * ********************************************************************************************/

#include "raylib.h"
#include <stdlib.h>

// #define PLATFORM_WEB

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables (needed for web platform)
//----------------------------------------------------------------------------------
const int screenWidth = 800;     // Screen width in pixels
const int screenHeight = 450;    // Screen height in pixels
Texture2D spriteSheet;           // The sprite sheet containing cloud animations
unsigned int rows = 32;          // Number of rows in the sprite sheet
unsigned int currentFrame = 0;   // Current animation frame (0-7)
unsigned int framesCounter = 0;  // Counter for frame timing
unsigned int framesSpeed = 8;    // Animation speed (frames per second)
unsigned int numberOfClouds = 1; // Current number of clouds (used for display)
bool debugMode = false;          // Toggle for debug information display
bool gameRunning = true;         // Game state flag
int spawnTimeMax = 5;            // Time in seconds between cloud spawns
int spawnTimer = 0;              // Timer for cloud spawning
int minX = 40;                   // Minimum X position for cloud spawning
int maxX = screenWidth - 40;     // Maximum X position for cloud spawning
int minY = 40;                   // Minimum Y position for cloud spawning
int maxY = 240;                  // Maximum Y position for cloud spawning
int score = 0;                   // Player's score (clouds spawned)
float totalHappiness = 0;        // Total happiness of all clouds added together
float averageHappiness = 0;      // All clouds averaged happiness

//----------------------------------------------------------------------------------
// Cloud Structure Definition
//----------------------------------------------------------------------------------
typedef struct {
  Vector2 position;   // Cloud's X and Y position on screen
  Rectangle frameRec; // Rectangle defining which part of spritesheet to draw
  float happiness;    // Cloud's happiness level (0.0 = sad, 1.0 = happy)
  Rectangle hitbox;   // Collision detection rectangle
} Cloud;

// Dynamic array management for clouds
Cloud *clouds = NULL; // Pointer to dynamic array of clouds
int maxClouds = 100;  // Maximum number of clouds that can exist
int activeClouds = 0; // Current number of active clouds in the array

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void); // Main game loop - update and draw one frame
static void UpdateCloudSprites(void);   // Handle cloud animation frame updates
static void InitClouds(void);           // Initialize the cloud system
static void AddCloud(Vector2 position); // Add a new cloud at specified position
static void DrawClouds(void);           // Render all active clouds to screen
static void UpdateDebugText(void);      // Display debug information
static void
UpdateHappiness(void); // Handle cloud happiness and player interaction
static void DrawGameOverScreen(void); // Display game over screen
static void UpdateHappinessBar(void); // Draw and update the happiness bar UI

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main() {
  // Initialization
  //--------------------------------------------------------------------------------------
  InitWindow(screenWidth, screenHeight, "raylib sprite animation");

  // Load the sprite sheet texture from resources folder
  spriteSheet = LoadTexture("resources/spritesheet.png");

  // Initialize the cloud management system
  InitClouds();

  // Spawn the first cloud at a random position within bounds
  AddCloud((Vector2){minX + rand() % (maxX - minX + 1),
                     minY + rand() % (maxY - minY + 1)});

#if defined(PLATFORM_WEB)
  // For web platform, use emscripten's main loop
  emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  // Main game loop for desktop platforms
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    UpdateDrawFrame();
  }
#endif

  // De-Initialization
  //--------------------------------------------------------------------------------------
  // Free the dynamically allocated cloud array memory
  if (clouds != NULL) {
    free(clouds);
    clouds = NULL;
  }

  UnloadTexture(spriteSheet); // Unload texture from GPU memory
  CloseWindow();              // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

//----------------------------------------------------------------------------------
// Main Game Loop Function
//----------------------------------------------------------------------------------
// Update and draw game frame
static void UpdateDrawFrame(void) {
  if (!gameRunning) {
    // Game Over State
    BeginDrawing();

    ClearBackground(BLACK);
    DrawGameOverScreen();

    // Check if player wants to restart
    if (IsKeyPressed(KEY_R)) {
      gameRunning = true;
      InitClouds(); // Reset cloud system
    }

    EndDrawing();
  } else {
    // Active Game State
    // Update
    //----------------------------------------------------------------------------------

    // Toggle debug mode with '/' key
    if (IsKeyPressed(KEY_SLASH)) {
      debugMode = !debugMode; // Simplified toggle
    }

    // Update game systems
    UpdateHappiness();    // Handle player interaction and cloud mood changes
    UpdateHappinessBar(); // Update the UI happiness indicator
    UpdateCloudSprites(); // Update cloud animations

    // Cloud spawning system
    spawnTimer++;
    if (spawnTimer >= spawnTimeMax * 60) { // Convert seconds to frames (60 FPS)
      // Generate random position within spawn bounds
      int randomX = minX + rand() % (maxX - minX + 1);
      int randomY = minY + rand() % (maxY - minY + 1);
      AddCloud((Vector2){randomX, randomY});
      spawnTimer = 0; // Reset spawn timer
      score++;        // Increment player score
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(SKYBLUE); // Sky blue background

    // Render all game objects
    DrawClouds();

    // Show debug information if enabled
    if (debugMode) {
      UpdateDebugText();
    }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }
}

//----------------------------------------------------------------------------------
// Cloud Management Functions
//----------------------------------------------------------------------------------

// Initialize cloud system - allocate memory for cloud array
static void InitClouds(void) {
  if (clouds == NULL) {
    clouds = (Cloud *)malloc(maxClouds * sizeof(Cloud));
  }
  // set all cloud and game specific variables to 0
  activeClouds = 0;
  numberOfClouds = 0;
  score = 0;
  spawnTimer = 0;
  totalHappiness = 0;
  averageHappiness = 0;
}

// Add a new cloud to the game at specified position
static void AddCloud(Vector2 position) {
  if (activeClouds < maxClouds) {
    // Set cloud position
    clouds[activeClouds].position = position;

    // Set up sprite frame rectangle (first frame, neutral happiness row)
    clouds[activeClouds].frameRec =
        (Rectangle){0.0f, 0.0f, (float)spriteSheet.width / 8,
                    (float)spriteSheet.height / rows};

    // Initialize with neutral happiness
    clouds[activeClouds].happiness = 0.5f;

    activeClouds++;                // Increment active cloud count
    numberOfClouds = activeClouds; // Update display counter
  }
}

// Update animation frames for all clouds
static void UpdateCloudSprites(void) {
  framesCounter++;

  // Check if it's time to advance to next animation frame
  if (framesCounter >= (60 / framesSpeed)) {
    framesCounter = 0; // Reset frame counter
    currentFrame++;    // Advance to next frame

    // Loop back to first frame after last frame (8 frames total: 0-7)
    if (currentFrame > 7)
      currentFrame = 0;

    // Update frame rectangle X position for all active clouds
    for (int i = 0; i < activeClouds; i++) {
      clouds[i].frameRec.x = (float)currentFrame * clouds[i].frameRec.width;
    }
  }
}

// Draw all active clouds to the screen
static void DrawClouds(void) {
  for (int i = 0; i < activeClouds; i++) {
    // Select sprite row based on happiness level
    // Spritesheet has 3 rows: happy (row 2), neutral (row 0), sad (row 1)
    if (clouds[i].happiness > 0.7f) {
      clouds[i].frameRec.y = 64.0f; // Happy cloud sprites (row 2)
    } else if (clouds[i].happiness < 0.3f) {
      clouds[i].frameRec.y = 32.0f; // Sad cloud sprites (row 1)
    } else {
      clouds[i].frameRec.y = 0.0f; // Neutral cloud sprites (row 0)
    }

    // Draw the cloud sprite
    DrawTextureRec(spriteSheet, clouds[i].frameRec, clouds[i].position, WHITE);

    // Draw debug information if debug mode is enabled
    if (debugMode) {
      const char *text = "Happiness: %.2f";
      int fontSize = 10;
      int textWidth = MeasureText(text, fontSize);

      // Display happiness value above cloud
      DrawText(TextFormat("Happiness: %.2f", clouds[i].happiness),
               clouds[i].position.x - (float)textWidth / 3,
               clouds[i].position.y - 15, fontSize, DARKGRAY);

      // Draw cloud hitbox for debugging collision detection
      clouds[i].hitbox =
          (Rectangle){clouds[i].position.x, clouds[i].position.y,
                      clouds[i].frameRec.width, clouds[i].frameRec.height};
      DrawRectangleLines(clouds[i].hitbox.x, clouds[i].hitbox.y,
                         clouds[i].hitbox.width, clouds[i].hitbox.height, RED);
    }
  }
}

//----------------------------------------------------------------------------------
// UI and Debug Functions
//----------------------------------------------------------------------------------

// Display debug information on screen
static void UpdateDebugText(void) {
  DrawFPS(10, 10); // Show current FPS
  DrawText("Sprite Sheet Animation", 10, 40, 20, DARKGRAY);
  DrawText(TextFormat("Frame: %d", currentFrame), 10, 70, 20, DARKGRAY);
  DrawText(TextFormat("Number of clouds: %d", numberOfClouds), 10, 100, 20,
           DARKGRAY);
  DrawText("Press ENTER to add cloud, / for debug", 10, 130, 12, DARKGRAY);
  DrawText("Click and drag clouds to make them happy!", 10, 145, 12, DARKGRAY);
}

// Handle player interaction with clouds and update happiness levels
static void UpdateHappiness(void) {
  Vector2 mousePos = GetMousePosition();   // Get current mouse position
  int pettingCloud = GetGestureDetected(); // Detect if player is dragging

  // Update happiness for all active clouds
  for (int i = 0; i < activeClouds; i++) {
    // Create hitbox for collision detection
    clouds[i].hitbox =
        (Rectangle){clouds[i].position.x, clouds[i].position.y,
                    clouds[i].frameRec.width, clouds[i].frameRec.height};

    // Check if mouse cursor is over this cloud
    if (CheckCollisionPointRec(mousePos, clouds[i].hitbox)) {
      if (pettingCloud == GESTURE_DRAG) {
        // Player is petting the cloud - increase happiness
        clouds[i].happiness += 0.02f;
      }
    } else {
      // Cloud is not being petted - slowly decrease happiness over time
      clouds[i].happiness -= 0.0001f;
    }

    // Keep happiness within valid range [0.0, 1.0]
    if (clouds[i].happiness > 1.0f)
      clouds[i].happiness = 1.0f;
    if (clouds[i].happiness < 0.0f)
      clouds[i].happiness = 0.0f;
  }

  // Calculate average happiness across all clouds
  totalHappiness = 0;
  for (int i = 0; i < activeClouds; i++) {
    totalHappiness += clouds[i].happiness;
  }
  averageHappiness = totalHappiness / activeClouds;

  // End game if there are 4 or more clouds and average happiness of the clouds
  // are less then 25%
  if (activeClouds > 3) {
    if (averageHappiness <= 0.25f) {
      gameRunning = false;
    }
  }
}

// Display game over screen
static void DrawGameOverScreen(void) {
  const char *text = "GAME OVER";
  int fontSize = 40;
  int textWidth = MeasureText(text, fontSize);

  // Center the game over text on screen
  DrawText(text, screenWidth / 2 - textWidth / 2,
           screenHeight / 2 - fontSize / 2, fontSize, RED);
  text = "Score: %d";
  fontSize = 12;
  textWidth = MeasureText(text, fontSize);
  DrawText(TextFormat(text, score), screenWidth / 2 - textWidth / 2,
           screenHeight / 2 - fontSize / 2 + 20, fontSize, ORANGE);
}

// Draw happiness bar UI element showing average cloud happiness
static void UpdateHappinessBar(void) {
  if (activeClouds == 0)
    return; // Don't draw bar if no clouds exist

  // Define happiness bar dimensions and position
  int barX = 15;                      // X position (left side of screen)
  int barY = (screenHeight / 2) - 50; // Y position (centered vertically)
  int barWidth = 20;                  // Bar width in pixels
  int barHeight = 100;                // Bar height in pixels

  // Draw bar background and border
  DrawRectangle(barX - 2, barY - 2, barWidth + 4, barHeight + 4,
                DARKGRAY);                               // Border
  DrawRectangle(barX, barY, barWidth, barHeight, BLACK); // Background

  // Calculate how much of the bar to fill based on happiness
  int fillHeight = (int)(averageHappiness * barHeight);

  // Choose bar color based on average happiness level
  Color fillColor;
  if (averageHappiness > 0.7f) {
    fillColor = PINK; // Happy - pink color
  } else if (averageHappiness > 0.3f) {
    fillColor = YELLOW; // Neutral - yellow color
  } else {
    fillColor = RED; // Sad - red color
  }

  // Draw the filled portion of the bar (fills from bottom upward)
  DrawRectangle(barX, barY + (barHeight - fillHeight), barWidth, fillHeight,
                fillColor);

  // Draw text labels for the happiness bar
  const char *text = "Happiness";
  int fontSize = 12;
  int textWidth = MeasureText(text, fontSize);

  DrawText("Happiness", barX - fontSize / 2, barY - 10 - fontSize / 2, fontSize,
           DARKGRAY); // Title

  text = "%.1f%%";
  fontSize = 10;
  textWidth = MeasureText(text, fontSize);

  DrawText(TextFormat("%.1f%%", averageHappiness * 100),
           barX - fontSize / 2, // Percentage
           barY + barHeight + fontSize / 2, fontSize, DARKGRAY);
}