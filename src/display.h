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
	void drawPixel(int x, int y, unsigned char color);
	void updateDisplay();
	void setTitle(const std::string& title);
	void setScale(int scale);
	void setSize(int width, int height);
	void setFullscreen(bool fullscreen);

private:

};