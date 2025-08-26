#pragma once

#include <random>
#include "display.h"
#include "gui.h"

struct config
{
	config() {}
	config(const int width, const int height, const int scale)
		: windowScale(scale), chip8Width(width), chip8Height(height) {}
	const int windowScale = 20;
	const int chip8Width = 64;
	const int chip8Height = 32;
	const std::string name = "Chip-8 Emulator";
};

struct settings
{
	const int fps = 60;
	const int soundFrequency = 44100;
	const int soundBufferSize = 512;
	const int soundChannels = 2;	// Stereo
	const int soundSampleSize = 16; // 16-bit samples
};

enum chip8States
{
	MENU = 0,
	RUNNING,
	PAUSED,
	QUIT
};

class chip8
{
public:
	chip8();
	chip8(const config& cfg);
	static chip8& Get();
	static chip8& Get(const config& cfg);
	void run();
	void loadRom(const std::string& filepath);
	void emulateCycle();
	void updateKeys();

	chip8(const chip8& obj) = delete;

	uint16_t fetchInstruction();
	void executeInstruction(uint16_t opcode);

public:
	static chip8* instancePTR;

	chip8States state = MENU;

	uint8_t memory[4096];

	// General purpose registers (V0-VF)
	uint8_t V[16];

	// Index register
	uint16_t I;

	// Program counter
	uint16_t pc;

	// Stack for subroutine calls
	uint16_t stack[16];

	// Stack pointer
	uint8_t sp;

	// Delay timer
	uint8_t delayTimer;

	// Sound timer
	uint8_t soundTimer;

	// Graphics buffer (64x32 pixels)
	uint8_t screen[64][32];

	// Flag to indicate if the screen needs to be redrawn
	bool draw_flag;

	// Keypad state (hex-based input)
	uint8_t keypad[16];
	std::vector<uint16_t> opcode_history;

	display disp;
	gui guiInstance;

	// Font set for CHIP-8, each character is 5x5 pixels
	uint8_t fontSet[80] = {
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
	config cfg;
	std::default_random_engine randGen;
	std::uniform_int_distribution<int> randByte;

	inline uint8_t getVxRegistry(const uint16_t opcode)
	{
		// And bitwise operation to extract the Vx register from the opcode then bit shifting right by 8 bits
		return (opcode & 0x0F00u) >> 8u; // Extract Vx from opcode
	}

	inline uint8_t getVyRegistry(const uint16_t opcode)
	{
		// And bitwise operation to extract the Vy register from the opcode then bit shifting right by 4 bits
		return (opcode & 0x00F0u) >> 4u; // Extract Vx from opcode
	}
};