#ifndef FCE_H
#define FCE_H

#include "common.h"

#define FPS          60
#define SCR_W       256
#define SCR_H       240

// HAS_GUI selects between framebuffer (AM_GPU) and character output.
// On platforms with a real GPU (NEMU, native), define HAS_GUI to use the
// 256x240 RGB framebuffer. On bare-metal targets like ysyxSoC where only a
// serial UART is wired up (no framebuffer in the AM IOE), leave HAS_GUI
// undefined: fce.c will then downsample the 256x240 NES canvas to a small
// ASCII grid and print it via putch() with ANSI cursor controls.
//
// The npc platform stub reports __am_gpu_config(present=false). We gate
// HAS_GUI on the platform here so minirv-nemu / riscv*-nemu (which all
// have a real GPU through platform/nemu/ioe/gpu.c) keep the framebuffer
// path, while the ysyxSoC + NPC backend uses ASCII art.
//
// NOTE: abstract-machine/Makefile spells the platform macro without a
// trailing `__`: see the line that emits `-D__PLATFORM_$(PLATFORM_UPPER)`
// (no extra underscores). Earlier versions of this file used
// __PLATFORM_NPC__ which is never defined, so the binary fell back to the
// AM_GPU framebuffer path and panicked with "screen too small" on NPC.
#ifndef __PLATFORM_NPC
# define HAS_GUI
#endif

#ifdef HAS_GUI
# define FRAME_SKIP   1
#else
// On the slow ysyxSoC verilator harness one NES frame already needs many
// minutes of wall-clock simulation (the 6502 emulator + AXI/SPI/UART
// latency dominate, not the renderer), so FRAME_SKIP doesn't actually
// save wall time -- the LSU is the bottleneck, not the framebuffer. Keep
// FRAME_SKIP at 0 so the very first frame the PPU completes is also the
// first frame we render to the terminal; otherwise the user waits an
// extra (FRAME_SKIP) * hours-per-frame to see anything at all.
# define FRAME_SKIP   0
#endif

void fce_update_screen();
int fce_load_rom(char *rom);
void fce_init();
void fce_run();
void draw(int x, int y, int idx);

static const uint32_t palette[64] = {
  0x808080, 0x0000BB, 0x3700BF, 0x8400A6, 0xBB006A, 0xB7001E, 0xB30000, 0x912600,
  0x7B2B00, 0x003E00, 0x00480D, 0x003C22, 0x002F66, 0x000000, 0x050505, 0x050505,
  0xC8C8C8, 0x0059FF, 0x443CFF, 0xB733CC, 0xFF33AA, 0xFF375E, 0xFF371A, 0xD54B00,
  0xC46200, 0x3C7B00, 0x1E8415, 0x009566, 0x0084C4, 0x111111, 0x090909, 0x090909,
  0xFFFFFF, 0x0095FF, 0x6F84FF, 0xD56FFF, 0xFF77CC, 0xFF6F99, 0xFF7B59, 0xFF915F,
  0xFFA233, 0xA6BF00, 0x51D96A, 0x4DD5AE, 0x00D9FF, 0x666666, 0x0D0D0D, 0x0D0D0D,
  0xFFFFFF, 0x84BFFF, 0xBBBBFF, 0xD0BBFF, 0xFFBFEA, 0xFFBFCC, 0xFFC4B7, 0xFFCCAE,
  0xFFD9A2, 0xCCE199, 0xAEEEB7, 0xAAF7EE, 0xB3EEFF, 0xDDDDDD, 0x111111, 0x111111
};

#endif
