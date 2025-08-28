// RaylibApp.cpp : Defines the entry point for the application.
//

#include "chip8.h"

#include <chrono>
#include <fstream>

#include "raylib.h"

chip8::chip8()
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	// Initialize the Chip-8 system
	pc = entryPoint; // Program counter starts at 0x200
	I = 0;			 // Index register
	sp = 0;			 // Stack pointer
	draw_flag = false;
	delayTimer = 0;
	soundTimer = 0;
	instructionsPerSecond = 700;

	randByte = std::uniform_int_distribution<int>(0, 255);

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
	memset(screen, 0, sizeof(screen));
	memset(keypad, 0, sizeof(keypad));

	beep = LoadSound("sound\\beep.wav");
}

chip8::chip8(const config& cfg)
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	// Initialize the Chip-8 system with configuration
	pc = 0x200; // Program counter starts at 0x200
	I = 0;		// Index register
	sp = 0;		// Stack pointer
	draw_flag = false;
	delayTimer = 0;
	soundTimer = 0;
	instructionsPerSecond = cfg.cpuHz;

	randByte = std::uniform_int_distribution<int>(0, 255);

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
	memset(screen, 0, sizeof(screen));
	memset(keypad, 0, sizeof(keypad));

	// this->cfg = cfg;

	disp.setTitle("CHIP-8 Emulator");
	disp.setSize(cfg.chip8Width * cfg.windowScale, cfg.chip8Height * cfg.windowScale);
	disp.setFullscreen(false);

	beep = LoadSound("sound\\beep.wav");
}

chip8::~chip8()
{
	UnloadSound(beep);
}

// Internal storage for the global chip8 instance (shared by all getters)
static chip8*& chip8_instance_slot()
{
	static chip8* p = nullptr;
	return p;
}

// Preferred reference getters
chip8& chip8::Get()
{
	chip8*& p = chip8_instance_slot();
	if (!p)
		p = new chip8();
	return *p;
}

chip8& chip8::Get(const config& cfg)
{
	chip8*& p = chip8_instance_slot();
	if (!p)
		p = new chip8(cfg);
	return *p;
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

	switch (state)
	{
		case chip8States::MENU:
		{
			guiInstance.run(this);
			break;
		}
		case chip8States::RUNNING:
		{

			emulateCycle();
			if (draw_flag == true)
			{
				disp.updateDisplay();
				draw_flag = false;
			}
			

			break;
		}
		case chip8States::PAUSED:
		{
			// Do nothing, just wait for unpause
			break;
		}
		default:
		{
			LOG_ERROR("Invalid state: %i", static_cast<int>(state));
			break;
		}
	}

	guiInstance.drawChip8DebugWindow(*this, &showDebugWindow);
}

void chip8::loadRom(const std::string& romFilepath)
{
	try
	{
		// Open the file as a stream of binary and move the file pointer to the end
		std::ifstream file(romFilepath, std::ios::binary | std::ios::ate);

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
			const char* pathname = romFilepath.c_str();
			LOG("Loaded ROM: %s (%f.3 bytes)", pathname, static_cast<float>(file.tellg()));
			file.close();
			// Free the buffer
			delete[] buffer;
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Failed to load ROM: %s", e.what());
	}
}

void chip8::emulateCycle()
{
	updateKeys();
	// loop to emulate the number of cycles per frame
	for (int i = 0; i < instructionsPerSecond / 60; ++i)
	{
		uint16_t opcode = fetchInstruction();

		pc += 2; // Move to the next instruction
		executeInstruction(opcode);
	}
	// Decrement the delay timer if it's been set
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	// Decrement the sound timer if it's been set
	if (soundTimer > 0)
	{
		if (soundTimer == 1)
		{
			PlaySound(beep);
		}
		--soundTimer;
	}
}

void chip8::updateKeys()
{
	// CHIP-8 keypad layout:
	// 1 2 3 C
	// 4 5 6 D
	// 7 8 9 E
	// A 0 B F

	int keyMap[16] = {
		KEY_X,	   // 0
		KEY_ONE,   // 1
		KEY_TWO,   // 2
		KEY_THREE, // 3
		KEY_Q,	   // 4
		KEY_W,	   // 5
		KEY_E,	   // 6
		KEY_A,	   // 7
		KEY_S,	   // 8
		KEY_D,	   // 9
		KEY_Z,	   // A
		KEY_C,	   // B
		KEY_FOUR,  // C
		KEY_R,	   // D
		KEY_F,	   // E
		KEY_V	   // F
	};

	for (int i = 0; i < 16; ++i)
	{
		keypad[i] = IsKeyDown(keyMap[i]);
	}
}

uint16_t chip8::fetchInstruction()
{
	const uint16_t opcode = (memory[pc] << 8u) | (memory[pc + 1]);
	opcode_history.push_back(opcode);
	return opcode;
}

void chip8::executeInstruction(uint16_t opcode)
{
	LOG("Opcode: 0x%x", opcode);
	// Decode and execute the opcode
	switch (opcode & 0xF000)
	{
		case 0x0000: // 0x00E0, 0x00EE
			switch (opcode & 0x00FF)
			{
				case 0x00E0:
				{
					// Clears the screen.
					memset(screen, 0, sizeof(screen));
					draw_flag = true;
					break;
				}
				case 0x00EE:
				{
					--sp;			// Decrement stack pointer
					pc = stack[sp]; // Set program counter to the address at the top of the stack
					break;
				}
				default:
					LOG_ERROR("Unknown opcode [0x0000]: 0x%X", opcode);
					/* SYS addr */
					break;
			}
			break;
		/* JP addr */
		case 0x1000:
		{
			// gets the address from the opcode (the lowest 12 bits) and sets the program counter to that address. stack is not required for this operation.
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
					uint8_t carry = (sum > 0xFF) ? 1 : 0;

					V[Vx] = static_cast<uint8_t>(sum & 0xFF); // Store the result in Vx, keeping it within 8 bits
					V[0xF] = carry;							  // Set carry flag if overflow occurs
					break;
				}
				/* SUB Vx, Vy */
				case 0x5:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);
					const uint8_t origX = V[Vx];
					const uint8_t origY = V[Vy];
					const uint8_t carry = (origY > origX) ? 0 : 1;
					V[Vx] = origX - origY; // Subtract Vy from Vx#
					V[0xF] = carry;		   // Set the carry flag if Vx > Vy

					break;
				}
				/* SHR Vx */
				case 0x6:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t origX = V[Vx];

					const uint8_t carry = V[Vx] & 0x1u;
					V[Vx] = origX >> 1; // Shift Vx right by 1 bit (division by 2)
					V[0xF] = carry;		// Set the carry flag to the least significant bit

					break;
				}
				/* SUBN Vx, Vy */
				case 0x7:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t Vy = getVyRegistry(opcode);
					const uint8_t carry = (V[Vx] > V[Vy]) ? 0 : 1;

					V[Vx] = static_cast<uint8_t>(V[Vy] - V[Vx]); // Subtract Vx from Vy
					V[0xF] = carry;								 // Set the carry flag if Vy > Vx
					break;
				}
				/* SHL Vx */
				case 0xE:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					const uint8_t carry = (V[Vx] & 0x80u) >> 7u;

					V[Vx] <<= 1;	// Shift Vx left by 1 bit (multiplication by 2)
					V[0xF] = carry; // Set the carry flag to the most significant bit
					break;
				}
				default:
					LOG_ERROR("Unknown opcode [0x8000]: 0x%X", opcode);
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
		/* RND Vx, byte */
		case 0xC000:
		{
			const uint8_t Vx = getVxRegistry(opcode);
			uint8_t byte = opcode & 0x00FFu;
			V[Vx] = static_cast<uint8_t>(randByte(randGen)) & byte; // Generate a random byte and AND it with the byte from the opcode
			break;
		}
		/* DRW Vx, Vy, nibble */
		case 0xD000:
		{
			uint8_t Vx = getVxRegistry(opcode);
			uint8_t Vy = getVyRegistry(opcode);
			uint8_t height = opcode & 0x000Fu; // Get the height of the sprite to draw

			V[0xF] = 0; // Clear the collision flag
			for (uint8_t row = 0; row < height; ++row)
			{
				uint8_t spriteRow = memory[I + row]; // Get the sprite row from memory
				for (uint8_t col = 0; col < 8; ++col)
				{
					if ((spriteRow & (0x80 >> col)) != 0) // Check if the pixel is set
					{
						uint8_t x = (V[Vx] + col) % 64; // Wrap around the screen width
						uint8_t y = (V[Vy] + row) % 32; // Wrap around the screen height

						if (screen[x][y] == 1) // Check for collision
						{
							V[0xF] = 1; // Set collision flag
						}
						screen[x][y] ^= 1; // Toggle the pixel on the display
					}
				}
			}
			draw_flag = true; // Indicate that the screen needs to be redrawn
			break;
		}
		case 0xE000:
			switch (opcode & 0x00FF)
			{
				/* SKP Vx */
				case 0x9E:
				{
					const uint8_t key = V[getVxRegistry(opcode)];
					if (keypad[key])
					{
						pc += 2; // Skip the next instruction if the key in Vx is pressed
					}
					break;
				}
				/* SKNP Vx */
				case 0xA1:
				{
					const uint8_t key = V[getVxRegistry(opcode)];
					if (!keypad[key])
					{
						pc += 2; // Skip the next instruction if the key in Vx is pressed
					}
					break;
				}
				default:
					LOG_ERROR("Unknown opcode [0xE000]: 0x%X", opcode);
					break;
			}
			break;
		case 0xF000:
			switch (opcode & 0x00FF)
			{
				/* LD Vx, DT */
				case 0x07:
				{
					V[getVxRegistry(opcode)] = delayTimer; // Load the value of the delay timer into Vx
					break;
				}
				/* LD Vx, K */
				case 0x0A:
				{
					uint8_t Vx = getVxRegistry(opcode);
					if (keypad[0])
					{
						V[Vx] = 0;
					}
					else if (keypad[1])
					{
						V[Vx] = 1;
					}
					else if (keypad[2])
					{
						V[Vx] = 2;
					}
					else if (keypad[3])
					{
						V[Vx] = 3;
					}
					else if (keypad[4])
					{
						V[Vx] = 4;
					}
					else if (keypad[5])
					{
						V[Vx] = 5;
					}
					else if (keypad[6])
					{
						V[Vx] = 6;
					}
					else if (keypad[7])
					{
						V[Vx] = 7;
					}
					else if (keypad[8])
					{
						V[Vx] = 8;
					}
					else if (keypad[9])
					{
						V[Vx] = 9;
					}
					else if (keypad[10])
					{
						V[Vx] = 10;
					}
					else if (keypad[11])
					{
						V[Vx] = 11;
					}
					else if (keypad[12])
					{
						V[Vx] = 12;
					}
					else if (keypad[13])
					{
						V[Vx] = 13;
					}
					else if (keypad[14])
					{
						V[Vx] = 14;
					}
					else if (keypad[15])
					{
						V[Vx] = 15;
					}
					else
					{
						pc -= 2;
					}
					break;
				}
				/* LD DT, Vx */
				case 0x15:
				{
					delayTimer = V[getVxRegistry(opcode)]; // Load the value of Vx into the delay timer
					break;
				}
				/* LD ST, Vx */
				case 0x18:
				{
					soundTimer = V[getVxRegistry(opcode)];
					break;
				}
				/* ADD I, Vx */
				case 0x1E:
				{
					I += V[getVxRegistry(opcode)]; // Add the value of Vx to the index register I
					break;
				}
				/* LD F, Vx */
				case 0x29:
				{
					uint8_t key = V[getVxRegistry(opcode)];

					I = fontSetStartAddress + (key * 5); // Set I to the address of the font character corresponding to Vx
					break;
				}
				/* LD B, Vx */
				case 0x33:
				{
					// takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
					uint8_t value = V[getVxRegistry(opcode)];
					memory[I + 2] = value % 10;
					value /= 10;
					memory[I + 1] = value % 10;
					value /= 10;
					memory[I] = value % 10;
					break;
				}
				/* LD [I], Vx */
				case 0x55:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					for (uint8_t i = 0; i <= Vx; ++i)
					{
						memory[I + i] = V[i]; // Store the values of V0 to Vx in memory starting at address I
					}
					break;
				}
				/* LD Vx, [I] */
				case 0x65:
				{
					const uint8_t Vx = getVxRegistry(opcode);
					for (uint8_t i = 0; i <= Vx; ++i)
					{
						V[i] = memory[I + i]; // Store the values of V0 to Vx in memory starting at address I
					}
					break;
				}
				default:
				{
					LOG_ERROR("Unknown opcode: 0x%x", opcode);
					break;
				}
			}
			break;
		default:
			LOG_ERROR("Unknown opcode: 0x%x", opcode);
			break;
	}
}