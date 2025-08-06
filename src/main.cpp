

#include "chip8.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
using namespace std;

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    config cfg;
    Log::Init();
	Chip8 chip8;
    InitWindow(cfg.chip8Width * cfg.windowScale, cfg.chip8Height * cfg.windowScale, "raylib [core] example - basic window");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    bool showMessageBox = false;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        if (showMessageBox)
            // Replace this line:
            // if (GuiButton((Rectangle) { 24, 24, 120, 30 }, "#191#Show Message")) showMessageBox = true;

            // With this fix:
            Rectangle btnRect;
        // Add this before using btnRect in the main loop
        Rectangle btnRect;
        btnRect.x = 24;
        btnRect.y = 24;
        btnRect.width = 120;
        btnRect.height = 30;
        if (GuiButton(btnRect, "#191#Show Message")) showMessageBox = true;
            btnRect.x = 24;
            btnRect.y = 24;
            btnRect.width = 120;
            btnRect.height = 30;
            if (GuiButton(btnRect, "#191#Show Message")) showMessageBox = true;
        {
           
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    LOG_WARN("test");
    return 0;
}