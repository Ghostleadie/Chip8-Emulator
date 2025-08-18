#pragma once

#include <iostream>
#include <chrono>
#include <random>
#include "display.h"
#include "gui.h"

struct config
{
	const int windowScale = 30;
	const int chip8Width = 64;
	const int chip8Height = 32;
	const std::string name = "Chip-8 Emulator";
};

struct settings
{
	const int fps = 60;
	const int soundFrequency = 44100;
	const int soundBufferSize = 512;
	const int soundChannels = 2; // Stereo
	const int soundSampleSize = 16; // 16-bit samples
};

enum chip8States {
	MENU = 0,
	RUNNING,
	PAUSED,
	QUIT
};

class chip8 {
public:
	chip8();
	chip8(const config& cfg);
	static chip8* getInstance();
	static chip8* getInstance(const config& cfg);
	void run();
	void load_rom(const std::string& filepath);
	void emulate_cycle();
	void draw();
	void set_keys();

	chip8(const chip8 &obj) = delete;

	uint16_t fetchinstruction();
	void decodeinstruction(uint16_t opcode);
	void executeinstruction(uint16_t opcode);

public:
	static chip8* instancePTR;
	chip8States state = MENU;

    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint16_t stack[16];
    uint8_t sp;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t gfx[64 * 32];
    bool draw_flag;
    uint8_t keypad[16];
	std::vector<uint16_t> opcode_history;

	display display;
	gui gui;

	// Font set for CHIP-8, each character is 5x5 pixels
	uint8_t fontSet[80] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	// font start point
	const unsigned int fontSetStartAddress = 0x50;

	const int entryPoint = 0x200;

	std::string filepath;
	bool showDebugWindow = false; // Toggle for debug window
private:
	static chip8* instance;


};

// TODO: Reference additional headers your program requires here.
