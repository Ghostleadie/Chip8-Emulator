#include "chip8.h"
#include "raylib.h"
#include "gui.h"
using namespace std;

bool showDebugWindow = false; // Toggle as needed
bool showFileDialog = false;
std::string selectedFile;

chip8* chip8::instance = nullptr;

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    config cfg;
    Log::Init();
	chip8* chip8 = chip8::getInstance(cfg);
    InitWindow(cfg.chip8Width * cfg.windowScale, cfg.chip8Height * cfg.windowScale, cfg.name.c_str());
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
	//chip8.load_rom("F:\\Git Projects\\Chip8-Emulator\\rom\\IBM Logo.ch8");


    // Main game loop
    while (!WindowShouldClose() || chip8->state == QUIT)    // Detect window close button or ESC key
    {

        BeginDrawing();
        chip8->run();
        // debug window end
        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}