#include "font.h"

void font::loadFont(uint8_t(&memory)[4096])
{
	// Load the font set into memory at the specified address
	for (unsigned int i = 0; i < sizeof(fontSet); ++i)
	{
		memory[fontSetStartAddress + i] = fontSet[i];
	}

	LOG_INFO("Font loaded into memory starting at address 0x{:X}", fontSetStartAddress);
}
