// RaylibApp.cpp : Defines the entry point for the application.
//

#include "chip8.h"

#include <chrono>
#include <fstream>

#include "raylib.h"
#include "log/log.h"




chip8::chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	// Initialize the Chip-8 system
	pc = entryPoint; // Program counter starts at 0x200
	I = 0;			 // Index register
	sp = 0;			 // Stack pointer
	draw_flag = false;
	delay_timer = 0;
	sound_timer = 0;

	randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(memory, 0, sizeof(memory));

	// Load the font set into memory at the specified address
	for (unsigned int i = 0; i < sizeof(fontSet); ++i)
	{
		memory[fontSetStartAddress + i] = fontSet[i];
	}

	LOG("Font loaded into memory starting at address 0x%U", fontSetStartAddress);

	// resetting display and keypad
	memset(gfx, 0, sizeof(display));
	memset(keypad, 0, sizeof(keypad));
}

chip8::chip8(const config& cfg) : randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	// Initialize the Chip-8 system with configuration
	pc = 0x200; // Program counter starts at 0x200
	I = 0;		// Index register
	sp = 0;		// Stack pointer
	draw_flag = false;
	delay_timer = 0;
	sound_timer = 0;

	randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

	memset(V, 0, sizeof(V));
	memset(stack, 0, sizeof(stack));
	memset(memory, 0, sizeof(memory));

	// Load the font set into memory at the specified address
	for (unsigned int i = 0; i < sizeof(fontSet); ++i)
	{
		memory[fontSetStartAddress + i] = fontSet[i];
	}

	LOG("Font loaded into memory starting at address 0x&U", fontSetStartAddress);

	// resetting display and keypad
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
			LOG("Paused the emulator");
		}
		else if (state == chip8States::PAUSED)
		{
			state = chip8States::RUNNING;
			LOG("Resumed the emulator");
		}
	}

	// debug options
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
			emulateCycle();
			break;
		}
		default:
		{
			LOG_ERR("Invalid state: %i", static_cast<int>(state));
			break;
		}
	}
}

void chip8::loadRom(const std::string& filepath)
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
			LOG("Loaded ROM: %s (%f.3 bytes)", filepath.c_str(), (float)file.tellg());
			file.close();
			// Free the buffer
			delete[] buffer;
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERR("Failed to load ROM: %s", e.what());
	}
}

void chip8::emulateCycle()
{
	uint16_t opcode = fetchInstruction();

	pc += 2; // Move to the next instruction
	executeInstruction(opcode);
}

void chip8::draw() {}

void chip8::setKeys() {}

uint16_t chip8::fetchInstruction()
{
	uint16_t opcode = (memory[pc] << 8) | (memory[pc + 1]);
	LOG("Opcode: 0x&u", opcode);
	opcode_history.push_back(opcode);
	return opcode;
}

void chip8::executeInstruction(uint16_t opcode)
{
	// Decode and execute the opcode
	switch (opcode & 0xF000)
	{
		case 0x0000: // 0x00E0, 0x00EE
			switch (opcode & 0x00FF)
			{
				case 0x00E0:
				{
					// Clears the screen.
					memset(gfx, 0, sizeof(display));
					break;
				}
				case 0x00EE:
				{
					--sp; // Decrement stack pointer
					pc = stack[sp]; // Set program counter to the address at the top of the stack
					break;
				}
				default: /* SYS addr */
					break;
			}
			break;
		/* JP addr */
		case 0x1000:
		{
			//gets the address from the opcode (the lowest 12 bits) and sets the program counter to that address. stack is not required for this operation.
			uint16_t address = opcode & 0x0FFFu;
			pc = address;
			break;
		}
		/* CALL addr */
		case 0x2000:
		{
			uint16_t address = opcode & 0x0FFFu;

			stack[sp] = pc;
			++sp;
			pc = address;
			break;
		}
		/* SE Vx, byte */
		case 0x3000:
		{
			const uint8_t Vx = getVxRegistry(opcode);
			uint8_t byte = opcode & 0x00FFu;

			if (V[Vx] == byte)
			{
				pc += 2; // Skip the next instruction if Vx == byte
			}

			break;
		}
		/* SNE Vx, byte */
		case 0x4000:
		{
			const uint8_t Vx = getVxRegistry(opcode);
			uint8_t byte = opcode & 0x00FFu;

			if (V[Vx] != byte)
			{
				pc += 2; // Skip the next instruction if Vx != byte
			}
			break;
		}
		/* SE Vx, Vy */
		case 0x5000:
		{
			const uint8_t Vx = getVxRegistry(opcode);
			const uint8_t Vy = getVyRegistry(opcode);

			if (V[Vx] == V[Vy])
			{
				pc += 2; // Skip the next instruction if Vx == Vy
			}
			break;
		}
		/* LD Vx, byte */
		case 0x6000:
		{
			const uint8_t Vx = getVxRegistry(opcode);
			uint8_t byte = opcode & 0x00FFu;
			V[Vx] = byte; // Load byte into Vx
			break;
		}
		/* ADD Vx, byte */
		case 0x7000:
		{
			const uint8_t Vx = getVxRegistry(opcode);
			uint8_t byte = opcode & 0x00FFu;
			V[Vx] += byte; // Add byte to Vx
			break;
		}
		case 0x8000:
			switch (opcode & 0x000F)
			{
				/* LD Vx, Vy */
				case 0x0:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);
					V[Vx] = V[Vy]; // Load value of Vy into Vx
					break;
				}
				/* OR Vx, Vy */
				case 0x1:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);
					V[Vx] |= V[Vy]; // Bitwise OR Vx and Vy
					break;
				}
				/* AND Vx, Vy */
				case 0x2:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);
					V[Vx] &= V[Vy]; // Bitwise AND Vx and Vy
					break;
				}
				/* XOR Vx, Vy */
				case 0x3:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);
					V[Vx] ^= V[Vy]; // Bitwise XOR Vx and Vy
					break;
				}
				/* ADD Vx, Vy */
				case 0x4:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);
					uint16_t sum = V[Vx] + V[Vy];
					V[0xF] = (sum > 0xFF) ? 1 : 0; // Set carry flag if overflow occurs
					V[Vx] = sum & 0xFF; // Store the result in Vx, keeping it within 8 bits
					break;
				}
				/* SUB Vx, Vy */
				case 0x5:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);

					V[0xF] = (Vx > Vy) ? 1 : 0; // Set the carry flag if Vx > Vy
					V[Vx] -= V[Vy];// Subtract Vy from Vx
					break;
				}
				/* SHR Vx */
				case 0x6:
				{
					const uint8_t Vx = getVxRegistry(opcode);

					V[0xF] = V[Vx] & 0x1u; // Set the carry flag to the least significant bit
					V[Vx] >>= 1; // Shift Vx right by 1 bit (division by 2)

					break;
				}
				/* SUBN Vx, Vy */
				case 0x7:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);
					V[0xF] = (Vy > Vx) ? 1 : 0; // Set the carry flag if Vy > Vx
					V[Vx] = V[Vy] - V[Vx]; // Subtract Vx from Vy
					break;
				}
				/* SHL Vx */
				case 0xE:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					V[0xF] = (V[Vx] & 0x80u) >> 7u; // Set the carry flag to the most significant bit
					V[Vx] <<= 1; // Shift Vx left by 1 bit (multiplication by 2)
					break;
				}
				default:
					break;
			}
			break;
		/* SNE Vx, Vy */
		case 0x9000:
		{
			const uint8_t Vx = getVxRegistry(opcode);
			const uint8_t Vy = getVyRegistry(opcode);
			if (V[Vx] != V[Vy])
			{
				pc += 2; // Skip the next instruction if Vx != Vy
			}
			break;
		}
		/* LD I, addr */
		case 0xA000:
		{
			const uint16_t address = opcode & 0x0FFFu;
			I = address; // Load the address into the index register I
			break;
		}
		/* JP V0, addr */
		case 0xB000:
		{
			const uint16_t address = opcode & 0x0FFFu;
			pc = address + V[0]; // Jump to the address plus the value of V0
			break;
		}
		case 0xC000: /* RND Vx, byte */ break;
		case 0xD000: /* DRW Vx, Vy, nibble */ break;
		case 0xE000:
			switch (opcode & 0x00FF)
			{
				case 0x9E: /* SKP Vx */
					break;
				case 0xA1: /* SKNP Vx */
					break;
				default:
					break;
			}
			break;
		case 0xF000:
			switch (opcode & 0x00FF)
			{
				case 0x07: /* LD Vx, DT */
					break;
				case 0x0A: /* LD Vx, K */
					break;
				case 0x15: /* LD DT, Vx */
					break;
				case 0x18: /* LD ST, Vx */
					break;
				case 0x1E: /* ADD I, Vx */
					break;
				case 0x29: /* LD F, Vx */
					break;
				case 0x33: /* LD B, Vx */
					break;
				case 0x55: /* LD [I], Vx */
					break;
				case 0x65: /* LD Vx, [I] */
					break;
				default:
					break;
			}
			break;
		default:
			LOG_ERR("chip8::executeinstruction Unknown opcode: &u", opcode);
			break;
	}
}