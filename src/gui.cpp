#include "gui.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "chip8.h"
#include "log/log.h"

#include <string>
#include <sstream>
#include <nfd.h>


gui::gui()
{
	// chip8Instance = chip8::getInstance();
}

void gui::run(chip8* instance)
{
	if (instance->state == MENU)
	{

		mainMenuResult menuResult = drawMainMenu(showFileDialog, instance->filepath);

		if (menuResult == MENU_LOAD && !instance->filepath.empty())
		{
			instance->loadRom(instance->filepath);
			instance->state = chip8States::RUNNING; // Set state to RUNNING after loading ROM
		}
		if (menuResult == MENU_QUIT)
		{
			// break;
		}
	}
}

mainMenuResult gui::drawMainMenu(bool& showFileDialog, std::string& selectedFile)
{
	ClearBackground(RAYWHITE);
	mainMenuResult result = MENU_NONE;
	bool showSettingsMenu = false;
	float temp = 0.0f; // Temporary variable for settings

	// Center menu on screen
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();
	Rectangle menuBox = { screenWidth / 2.0f - 120, screenHeight / 2.0f - 120, 240, 240 };

	GuiPanel(menuBox, "Main Menu");

	float btnY = menuBox.y + 40;
	if (GuiButton({ menuBox.x + 60, btnY, 120, 30 }, "Load ROM"))
	{
		showFileDialog = true;
		result = MENU_LOAD;
	}
	btnY += 40;
	if (GuiButton({ menuBox.x + 60, btnY, 120, 30 }, "Settings"))
	{
		result = MENU_SETTINGS;
		bool showSettingsMenu = false;
		// drawSettingsMenu(showSettingsMenu, temp);
	}
	btnY += 40;
	if (GuiButton({ menuBox.x + 60, btnY, 120, 30 }, "About"))
	{
		result = MENU_ABOUT;
	}
	btnY += 40;
	if (GuiButton({ menuBox.x + 60, btnY, 120, 30 }, "Quit"))
	{
		result = MENU_QUIT;
	}

	// Use filedialog.h for file selection
	static char filePath[1024] = { 0 };
	static bool fileDialogOpen = false;

	if (showFileDialog)
	{
		fileDialogOpen = true;
		showFileDialog = false; // Only open once per button press
	}

	if (fileDialogOpen)
	{

		NFD_Init();

		nfdu8char_t* outPath;
		nfdu8filteritem_t filters[1] = { { "Roms", "ch8" } };
		nfdopendialogu8args_t args = { 0 };
		args.filterList = filters;
		args.filterCount = 1;
		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
		if (result == NFD_OKAY)
		{
			LOG("Rom Path: &s", outPath);
			selectedFile = outPath; // Store the selected file path
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL)
		{
			LOG("User pressed cancel.");
			puts("User pressed cancel.");
		}
		else
		{
			LOG_ERR("gui::drawMainMenu Error: &s", NFD_GetError());
		}

		showFileDialog = false;
		fileDialogOpen = false;

		NFD_Quit();
	}

	return result;
}

void gui::drawChip8DebugWindow(const chip8& cpu, bool* showWindow)
{
	if (!showWindow || !(*showWindow))
		return;

	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();
	Rectangle debugBox = { 0, 0, 400, 500 };

	GuiPanel(debugBox, "Chip-8 Debug");

	float y = debugBox.y + 30;
	float x = debugBox.x + 20;

	// Main registers
	GuiLabel({ x, y, 180, 20 }, TextFormat("PC: 0x%04X", cpu.pc));
	GuiLabel({ x + 200, y, 180, 20 }, TextFormat("I:  0x%04X", cpu.I));
	y += 25;
	GuiLabel({ x, y, 180, 20 }, TextFormat("SP: 0x%02X", cpu.sp));
	GuiLabel({ x + 200, y, 180, 20 }, TextFormat("Delay Timer: %d", cpu.delay_timer));
	y += 25;
	GuiLabel({ x, y, 180, 20 }, TextFormat("Sound Timer: %d", cpu.sound_timer));

	y += 30;
	GuiLabel({ x, y, 360, 20 }, "V Registers:");
	y += 20;
	for (int i = 0; i < 16; ++i)
	{
		int col = i % 8;
		int row = i / 8;
		GuiLabel({ x + col * 45, y + row * 20, 45, 20 }, TextFormat("V%X: %02X", i, cpu.V[i]));
	}

	y += 50;
	GuiLabel({ x, y, 360, 20 }, "Stack:");
	y += 20;
	for (int i = 0; i < 16; ++i)
	{
		int col = i % 8;
		int row = i / 8;
		GuiLabel({ x + col * 45, y + row * 20, 45, 20 }, TextFormat("%X: %04X", i, cpu.stack[i]));
	}

	y += 50;
	GuiLabel({ x, y, 360, 20 }, "Memory [PC-4 ... PC+4]:");
	y += 20;
	for (int i = -4; i <= 4; ++i)
	{
		int addr = cpu.pc + i;
		if (addr >= 0 && addr < 4096)
		{
			GuiLabel({ x + (i + 4) * 40, y, 40, 20 }, TextFormat("%03X: %02X", addr, cpu.memory[addr]));
		}
	}

	// Opcode history list
	float listY = y + 80;
	GuiLabel({ x, listY, 360, 20 }, "Opcode History:");
	listY += 20;
	int maxOpcodes = 10; // Show last 10 opcodes
	int start = std::max(0, (int)cpu.opcode_history.size() - maxOpcodes);
	for (int i = start; i < cpu.opcode_history.size(); ++i)
	{
		GuiLabel({ x, listY + (i - start) * 20, 180, 20 }, TextFormat("%04X", cpu.opcode_history[i]));
	}

	// Optional: Close button
	if (GuiButton({ debugBox.x + debugBox.width - 90, debugBox.y + debugBox.height - 40, 80, 30 }, "Close"))
	{
		*showWindow = false;
	}
}

void gui::fileDialogBox(bool& showFileDialog, std::string& selectedFile)
{
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();
	static char filePath[256] = { 0 };
	Rectangle dialogBox = { screenWidth / 2.0f - 150, screenHeight / 2.0f - 80, 300, 160 };
	GuiPanel(dialogBox, "Open .ch8 ROM");
	GuiTextBox({ dialogBox.x + 20, dialogBox.y + 40, 260, 30 }, filePath, 255, true);
	if (GuiButton({ dialogBox.x + 40, dialogBox.y + 90, 80, 30 }, "Open"))
	{
		selectedFile = filePath;
		showFileDialog = false;
	}
	if (GuiButton({ dialogBox.x + 180, dialogBox.y + 90, 80, 30 }, "Cancel"))
	{
		showFileDialog = false;
	}
}

void gui::drawSettingsMenu(bool& showSettingsMenu, float& volume)
{
	if (!showSettingsMenu)
		return;

	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();
	Rectangle settingsBox = { screenWidth / 2.0f - 150, screenHeight / 2.0f - 100, 300, 200 };

	GuiPanel(settingsBox, "Settings");

	float sliderY = settingsBox.y + 50;
	float sliderX = settingsBox.x + 30;
	float sliderWidth = 240;

	GuiLabel({ sliderX, sliderY - 30, sliderWidth, 20 }, "Audio Settings");
	GuiLabel({ sliderX, sliderY, 80, 20 }, "Volume");

	// Raygui slider expects float* for value, min, max
	volume = GuiSlider({ sliderX + 90, sliderY, 120, 20 }, "0", "1", &volume, 0.0f, 1.0f);

	// Optional: Close button
	if (GuiButton({ settingsBox.x + settingsBox.width - 90, settingsBox.y + settingsBox.height - 40, 80, 30 }, "Back"))
	{
		showSettingsMenu = false;
	}
}