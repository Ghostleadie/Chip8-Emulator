// RaylibApp.cpp : Defines the entry point for the application.
//

#include "chip8.h"
#include "raylib.h"
#include <fstream>


chip8::chip8()
{
	// Initialize the Chip-8 system
	pc = entryPoint; // Program counter starts at 0x200
	I = 0;           // Index register
	sp = 0;          // Stack pointer
	draw_flag = false;
	delay_timer = 0;
	sound_timer = 0;

	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(memory, 0, sizeof(memory));

	// Load the font set into memory at the specified address
	for (unsigned int i = 0; i < sizeof(fontSet); ++i)
	{
		memory[fontSetStartAddress + i] = fontSet[i];
	}

	LOG_INFO("Font loaded into memory starting at address 0x{:X}", fontSetStartAddress);

	//resetting display and keypad
	memset(gfx, 0, sizeof(display));
	memset(keypad, 0, sizeof(keypad));

}

chip8::chip8(const config& cfg)
{
	// Initialize the Chip-8 system with configuration
	pc = 0x200; // Program counter starts at 0x200
	I = 0;      // Index register
	sp = 0;     // Stack pointer
	draw_flag = false;
	delay_timer = 0;
	sound_timer = 0;

	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(memory, 0, sizeof(memory));

	// Load the font set into memory at the specified address
	for (unsigned int i = 0; i < sizeof(fontSet); ++i)
	{
		memory[fontSetStartAddress + i] = fontSet[i];
	}

	LOG_INFO("Font loaded into memory starting at address 0x{:X}", fontSetStartAddress);

	//resetting display and keypad
	memset(gfx, 0, sizeof(display));
	memset(keypad, 0, sizeof(keypad));

	disp.setTitle("CHIP-8 Emulator");
	disp.setSize(cfg.chip8Width * cfg.windowScale, cfg.chip8Height * cfg.windowScale);
	disp.setFullscreen(false);
}

chip8* chip8::getInstance()
{
	if (instance == nullptr)
	{
		instance = new chip8();
	}
	return instance;
}

chip8* chip8::getInstance(const config& cfg)
{
	if (instance == nullptr)
	{
		instance = new chip8(cfg);
	}
	return instance;
}

void chip8::run()
{
	if (IsKeyPressed(KEY_SPACE))
	{
		if (state == chip8States::RUNNING)
		{
			state = chip8States::PAUSED;
			LOG_INFO("Paused the emulator");
		}
		else if (state == chip8States::PAUSED)
		{
			state = chip8States::RUNNING;
			LOG_INFO("Resumed the emulator");
		}
	}

	//debug options
	if (IsKeyPressed(KEY_GRAVE))
	{
		showDebugWindow = !showDebugWindow;
	}

	guiInstance.drawChip8DebugWindow(*instance, &showDebugWindow);

	switch (state)
	{
		case chip8States::MENU:
		{
			guiInstance.run(instance);
			break;
		}
		case chip8States::RUNNING:
		{
			// Fetch the next instruction
			emulate_cycle();
			break;
		}
		default:
		{
			LOG_ERROR("Invalid state: {}", static_cast<int>(state));
			break;
		}
	}
}

void chip8::load_rom(const std::string& filepath)
{
	try
	{
		// Open the file as a stream of binary and move the file pointer to the end
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);

		if (file.is_open())
		{
			// Get size of file and allocate a buffer to hold the contents
			std::streampos size = file.tellg();
			char* buffer = new char[size];

			// Go back to the beginning of the file and fill the buffer
			file.seekg(0, std::ios::beg);
			file.read(buffer, size);

			// Load the ROM contents into the Chip8's memory, starting at 0x200
			for (long i = 0; i < size; ++i)
			{
				memory[entryPoint + i] = buffer[i];
			}
			file.seekg(0, file.end);
			LOG_INFO("Loaded ROM: {} ({} bytes)", filepath, (float)file.tellg());
			file.close();
			// Free the buffer
			delete[] buffer;
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Failed to load ROM: {}", e.what());
	}
}

void chip8::emulate_cycle()
{
	uint16_t opcode = fetchinstruction();

	pc += 2; // Move to the next instruction
	executeinstruction(opcode);
}

void chip8::draw() {}

void chip8::set_keys() {}

uint16_t chip8::fetchinstruction()
{
	uint16_t opcode = (memory[pc] << 8) | (memory[pc + 1]);
	LOG_INFO("Opcode: 0x{:X}", opcode);
	opcode_history.push_back(opcode);
	return opcode;
}

void chip8::executeinstruction(uint16_t opcode)
{
	// Decode and execute the opcode
	switch (opcode & 0xF000)
	{
		case 0x0000: // 0x00E0, 0x00EE
			switch (opcode & 0x00FF)
			{
				case 0x00E0:
				{
					//Clears the screen.
					memset(gfx, 0, sizeof(display));
					break;
				}
				case 0x00EE:
				{
					break;
				}
				default: /* SYS addr */ break;
			}
			break;
		case 0x1000: /* JP addr */ break;
		case 0x2000: /* CALL addr */ break;
		case 0x3000: /* SE Vx, byte */ break;
		case 0x4000: /* SNE Vx, byte */ break;
		case 0x5000: /* SE Vx, Vy */ break;
		case 0x6000: /* LD Vx, byte */ break;
		case 0x7000: /* ADD Vx, byte */ break;
		case 0x8000:
			switch (opcode & 0x000F)
			{
				case 0x0: /* LD Vx, Vy */ break;
				case 0x1: /* OR Vx, Vy */ break;
				case 0x2: /* AND Vx, Vy */ break;
				case 0x3: /* XOR Vx, Vy */ break;
				case 0x4: /* ADD Vx, Vy */ break;
				case 0x5: /* SUB Vx, Vy */ break;
				case 0x6: /* SHR Vx */ break;
				case 0x7: /* SUBN Vx, Vy */ break;
				case 0xE: /* SHL Vx */ break;
				default:
					break;
			}
			break;
		case 0x9000: /* SNE Vx, Vy */ break;
		case 0xA000: /* LD I, addr */ break;
		case 0xB000: /* JP V0, addr */ break;
		case 0xC000: /* RND Vx, byte */ break;
		case 0xD000: /* DRW Vx, Vy, nibble */ break;
		case 0xE000:
			switch (opcode & 0x00FF)
			{
				case 0x9E: /* SKP Vx */ break;
				case 0xA1: /* SKNP Vx */ break;
				default:
					break;
			}
			break;
		case 0xF000:
			switch (opcode & 0x00FF)
			{
				case 0x07: /* LD Vx, DT */ break;
				case 0x0A: /* LD Vx, K */ break;
				case 0x15: /* LD DT, Vx */ break;
				case 0x18: /* LD ST, Vx */ break;
				case 0x1E: /* ADD I, Vx */ break;
				case 0x29: /* LD F, Vx */ break;
				case 0x33: /* LD B, Vx */ break;
				case 0x55: /* LD [I], Vx */ break;
				case 0x65: /* LD Vx, [I] */ break;
				default:
					break;
			}
			break;
		default:
			LOG_ERROR("{} Unknown opcode: {:#04X}", __FUNCTION__, opcode);
			break;
	}
}