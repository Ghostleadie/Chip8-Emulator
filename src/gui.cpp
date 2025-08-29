#include "gui.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "chip8.h"

#include <string>
#include <sstream>
#include <nfd.h>

gui::gui()
{
}

void gui::run(chip8* instance)
{
	ClearBackground(BLACK);
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
			instance->state = chip8States::QUIT;
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
	Rectangle menuBox = { screenWidth / 2.0f - 100, screenHeight / 2.0f - 100, 240, 180 };

	GuiPanel(menuBox, "Main Menu");

	float btnY = menuBox.y + 40;
	if (GuiButton({ menuBox.x + 60, btnY, 120, 30 }, "Load ROM"))
	{
		showFileDialog = true;
		result = MENU_LOAD;
		ClearBackground(BLACK);
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
			const char* path = outPath;
			LOG("Rom Path: %s", path);
			selectedFile = outPath; // Store the selected file path
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL)
		{
			LOG("User pressed cancel.");
		}
		else
		{
			LOG_ERROR("gui::drawMainMenu Error: &s", NFD_GetError());
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
	GuiLabel({ x + 200, y, 180, 20 }, TextFormat("Delay Timer: %d", cpu.delayTimer));
	y += 25;
	GuiLabel({ x, y, 180, 20 }, TextFormat("Sound Timer: %d", cpu.soundTimer));

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

	// Opcode history (scrollable)
	float listY = y + 20;
	GuiLabel({ x, listY, 360, 20 }, "Opcode History:");
	listY += 24;

	// Scroll panel setup
	static Vector2 scroll = { 0, 0 }; // persists across frames
	static int lastCount = 0;		  // for auto-scroll to bottom on new items
	const float rowHeight = 20.0f;

	Rectangle panelBounds = { x, listY, debugBox.width - 40, 180 }; // visible area
	int count = (int)cpu.opcode_history.size();
	float contentHeight = count * rowHeight;
	// Leave some room for the vertical scrollbar
	Rectangle content = { 0, 0, panelBounds.width - 14, contentHeight };
	Rectangle view = { 0 };

	GuiScrollPanel(panelBounds, "test", content, &scroll, &view);

	// Autoscroll to bottom when new opcodes were appended
	if (count > lastCount)
	{
		float maxScrollY = -(contentHeight - panelBounds.height);
		if (contentHeight > panelBounds.height)
			scroll.y = maxScrollY;
		lastCount = count;
	}

	// Clip to the scroll panel view and draw inner labels offset by scroll
	BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);
	{
		float innerX = panelBounds.x + scroll.x + 6; // small left padding
		float innerY = panelBounds.y + scroll.y;

		// Optionally compute a visible range to avoid drawing everything
		int firstVisible = (int)std::floorf((-scroll.y) / rowHeight) - 1;
		firstVisible = std::max(0, firstVisible);
		int lastVisible = (int)std::ceilf((-scroll.y + panelBounds.height) / rowHeight) + 1;
		lastVisible = std::min(count, lastVisible);

		for (int i = firstVisible; i < lastVisible; ++i)
		{
			GuiLabel({ innerX, innerY + i * rowHeight, content.width - 8, rowHeight },
				TextFormat("%04X", cpu.opcode_history[i]));
		}
	}
	EndScissorMode();
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

void gui::drawpauseMenu(chip8* instance)
{
	// Centered pause panel
	int sw = GetScreenWidth();
	int sh = GetScreenHeight();
	Rectangle box = { sw / 2.0f - 120, sh / 2.0f - 90, 240, 160 };
	GuiPanel(box, "Paused");

	static bool showFileDialog = false;
	static bool fileDialogOpen = false;

	float btnY = box.y + 40;
	// Load ROM button
	if (GuiButton({ box.x + 60, btnY, 120, 30 }, "Load ROM"))
	{
		showFileDialog = true;
	}
	btnY += 40;
	// return to menu button
	if (GuiButton({ box.x + 60, btnY, 120, 30 }, "Return To Menu"))
	{
		instance->resetChip8();
		instance->state = chip8States::MENU;
	}
	btnY += 40;
	// Quit button
	if (GuiButton({ box.x + 60, btnY, 120, 30 }, "Quit"))
	{
		instance->state = chip8States::QUIT;
	}

	// Handle file dialog
	if (showFileDialog)
	{
		fileDialogOpen = true;
		showFileDialog = false;
	}

	if (fileDialogOpen)
	{
		NFD_Init();
		nfdu8char_t* outPath = nullptr;
		nfdu8filteritem_t filters[1] = { { "Roms", "ch8" } };
		nfdopendialogu8args_t args = { 0 };
		args.filterList = filters;
		args.filterCount = 1;
		nfdresult_t r = NFD_OpenDialogU8_With(&outPath, &args);
		if (r == NFD_OKAY)
		{
			instance->filepath = outPath;
			NFD_FreePathU8(outPath);
			// Reset, load, and resume
			instance->resetChip8();
			instance->loadRom(instance->filepath);
			instance->state = chip8States::RUNNING;
		}
		else if (r == NFD_ERROR)
		{
			LOG_ERROR("PauseMenu: %s", NFD_GetError());
		}
		fileDialogOpen = false;
		NFD_Quit();
	}
}
