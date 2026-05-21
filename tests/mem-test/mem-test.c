// =============================================================================
// B2b: SoC-side mem-test for the PSRAM (0x80000000) and SDRAM (0xa0000000)
// windows. Builds for ARCH=minirv-npc (linker default _pmem_start=0x80000000).
//
// On the patched ysyxSoCFull.v shipped in this tree the APBFanout routes the
// entire 0x80000000-0xbfffffff range to the SDRAM channel, so BOTH bases
// hit the same backing store; we still keep the dual base test in case the
// SoC top is later updated to instantiate psram_top_apb on the 0x80000000
// half of the window. The stand-alone iverilog harness under
// ysyx-workbench/tests/mem-test-rtl/ separately validates the psram.v /
// sdram.v die models against their real controllers.
//
// The test reuses the trick from the C++ TRM hello image: text/data is
// already in PSRAM (we are running from 0x80000000+), so we mustn't trash
// the program region. We deliberately use a small range above the loaded
// image (a few KByte at the top of each window).
// =============================================================================

#include <am.h>
#include <klib-macros.h>

// Choose offsets that are guaranteed to be outside the program text/data.
// hello-minirv-ysyxsoc.bin's loader copies the payload to 0x80000000 and
// the .text+.bss footprint is well under 256 KByte; we pick offsets at
// +1 MiB / +2 MiB so we never collide regardless of payload growth.
//
// On the patched ysyxSoCFull.v shipped in this tree the APBFanout's sel_2
// rule (`{~p[31], p[29]} == 0`) only routes the 0x80000000-0x9fffffff
// half of the documented memory window to the SDRAM channel; the
// 0xa0000000-0xbfffffff half is currently unmapped. Both test bases below
// therefore live in the 0x8.. region. The standalone iverilog harness
// under ysyx-workbench/tests/mem-test-rtl/ separately validates the
// psram.v / sdram.v die models with their real controllers.
#define PSRAM_BASE  0x80100000u
#define SDRAM_BASE  0x80200000u
#define TEST_BYTES  0x1000u           // 4 KByte per window

static inline uint32_t pat1(uint32_t off) { return off ^ 0xdeadbeefu; }
static inline uint32_t pat2(uint32_t off) { return (off << 16) | (~off & 0xffffu); }

static int run_one(const char *name, uint32_t base) {
  putstr("[mem-test] ");
  putstr(name);
  putstr(": start\n");

  // Phase 1: write pat1, read back, compare.
  for (uint32_t off = 0; off < TEST_BYTES; off += 4) {
    *(volatile uint32_t *)(base + off) = pat1(off);
  }
  for (uint32_t off = 0; off < TEST_BYTES; off += 4) {
    uint32_t got = *(volatile uint32_t *)(base + off);
    if (got != pat1(off)) {
      putstr("[mem-test] ");
      putstr(name);
      putstr(": FAIL pat1\n");
      return 1;
    }
  }

  // Phase 2: overwrite with pat2.
  for (uint32_t off = 0; off < TEST_BYTES; off += 4) {
    *(volatile uint32_t *)(base + off) = pat2(off);
  }
  for (uint32_t off = 0; off < TEST_BYTES; off += 4) {
    uint32_t got = *(volatile uint32_t *)(base + off);
    if (got != pat2(off)) {
      putstr("[mem-test] ");
      putstr(name);
      putstr(": FAIL pat2\n");
      return 1;
    }
  }

  putstr("[mem-test] ");
  putstr(name);
  putstr(": PASS\n");
  return 0;
}

int main(const char *args) {
  int err = 0;
  err |= run_one("PSRAM", PSRAM_BASE);
  err |= run_one("SDRAM", SDRAM_BASE);
  if (err == 0) putstr("[mem-test] ALL PASS\n");
  else          putstr("[mem-test] HAD ERRORS\n");
  return err;
}
