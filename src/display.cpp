#include "display.h"
#include <raylib.h>

#include "chip8.h"

display::display(int width, int height, int scale)
{
	this->width = width;
	this->height = height;
	this->scale = scale;
}

display::display() {}

void display::draw()
{
	for (int x = 0; x < cols; x++)
	{
		for (int y = 0; y < rows; y++)
		{
			if (x < 64 && y < 32 && chip8::Get().screen[x][y])
			{
				drawPixel(x, y);
			}
		}
	}
}

void display::drawPixel(const int x, const int y)
{
	DrawRectangle(x * scale, y * scale,scale,scale, WHITE);
}

void display::updateDisplay()
{
	ClearBackground(BLACK);

	draw();
}

void display::setTitle(const std::string& title) {}

void display::setScale(int scale)
{
	this->scale = scale;
}

void display::setSize(int width, int height)
{
	this->width = width;
	this->height = height;
}

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