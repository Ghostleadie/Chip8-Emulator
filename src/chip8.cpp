// RaylibApp.cpp : Defines the entry point for the application.
//

#include "chip8.h"
#include "raylib.h"

using namespace std;

Chip8::Chip8()
{
	// Initialize the Chip-8 system
	pc = 0x200; // Program counter starts at 0x200
	I = 0;     // Index register
	sp = 0;    // Stack pointer
	draw_flag = false;
	delay_timer = 0;
	sound_timer = 0;

	fontSet.loadFont(memory);
}

void Chip8::load_rom(const std::string& filename)
{
}

void Chip8::emulate_cycle()
{
}

void Chip8::draw()
{
}

void Chip8::set_keys()
{
}
