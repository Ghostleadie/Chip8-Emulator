#include "display.h"
#include <raylib.h>

display::display(int width, int height, int scale) {}

display::display() {}

void display::drawPixel(int x, int y, unsigned char color) {}

void display::updateDisplay()
{
	ClearBackground(RAYWHITE);

	DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
}

void display::setTitle(const std::string& title) {}

void display::setScale(int scale) {}

void display::setSize(int width, int height) {}

void display::setFullscreen(bool fullscreen)
{
	if (fullscreen)
	{
		ToggleFullscreen();
	}
	else
	{
		//SetWindowedMode();
	}
}