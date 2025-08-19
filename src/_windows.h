//
// Instead of calling windows.h call this
// Windows header is very heavy and this really speeds up compile time
//

#ifndef CHIP8_EMULATOR__WINDOWS_H
#define CHIP8_EMULATOR__WINDOWS_H

#define _NO_CRT_STDIO_INLINE

// So this really speeds up compile time
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN // Could be useless
#define NOGDICAPMASKS
#define NOGDI
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOUSER
#include <windows.h>

#endif // CHIP8_EMULATOR__WINDOWS_H
