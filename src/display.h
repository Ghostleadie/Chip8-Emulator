#pragma once

#include <string>

enum screens
{
	SCREEN_MAIN,
	SCREEN_SETTINGS,
	SCREEN_GAME,
	SCREEN_EXIT
};

class display
{
public:
	display();
	display(int width, int height, int scale);
	//~Display() {};
	void clear();
	void draw();
	void drawPixel(const int x,const int y);
	void updateDisplay();
	void setTitle(const std::string& title);
	void setScale(int scale);
	void setSize(int width, int height);
	void setFullscreen(bool fullscreen);

private:
	int cols = 64;
	int rows = 32;
	int scale = 20;
	int width = 64;
	int height = 32;
};