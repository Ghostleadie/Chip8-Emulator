#pragma once
#include "raygui.h"

#include <string>

class chip8;
// Simple main menu state
enum mainMenuResult
{
	MENU_NONE,
	MENU_LOAD,
	MENU_SETTINGS,
	MENU_ABOUT,
	MENU_QUIT
};

class gui
{
public:
	gui();
	void run(chip8* instance);
	mainMenuResult drawMainMenu(bool& showFileDialog, std::string& selectedFile);
	void drawChip8DebugWindow(const chip8& cpu, bool* showWindow);
	void fileDialogBox(bool& showFileDialog, std::string& selectedFile);
	void drawpauseMenu(chip8* instance);

private:
	bool showFileDialog = false; // Toggle for file dialog
	bool menuDrawn = false;
};