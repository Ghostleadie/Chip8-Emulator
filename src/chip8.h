// RaylibApp.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include "font.h"

struct config
{
	const int windowScale = 10;
	const int chip8Width = 64;
	const int chip8Height = 32;
};

class Chip8 {   
public:
    
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

	font fontSet;
public:
    Chip8();
    void load_rom(const std::string& filename);
    void emulate_cycle();
    void draw();
    void set_keys();
};

// TODO: Reference additional headers your program requires here.
