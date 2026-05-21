#include <amtest.h>

void (*entry)() = NULL; // mp entry

static const char *tests[256] = {
  ['h'] = "hello",
  ['H'] = "display this help message",
  ['i'] = "interrupt/yield test",
  ['d'] = "scan devices",
  ['m'] = "multiprocessor test",
  ['t'] = "real-time clock test",
  ['k'] = "readkey test",
  ['v'] = "display test",
  ['a'] = "audio test",
  ['p'] = "x86 virtual memory test",
  ['s'] = "IOE self test (prints AM PASS)",
};

static void ioe_self_test(void) {
  printf("===== IOE self test =====\n");

  AM_TIMER_UPTIME_T t1 = io_read(AM_TIMER_UPTIME);
  for (volatile int i = 0; i < 200000; i++) ;
  AM_TIMER_UPTIME_T t2 = io_read(AM_TIMER_UPTIME);
  uint32_t delta_us = (uint32_t)(t2.us - t1.us);
  printf("timer: t1=%u us, t2=%u us, delta=%u us\n",
         (uint32_t)t1.us, (uint32_t)t2.us, delta_us);
  bool timer_ok = (t2.us > t1.us);

  AM_GPU_CONFIG_T gpu = io_read(AM_GPU_CONFIG);
  printf("gpu: present=%d, %d x %d, vmem=%d\n",
         gpu.present, gpu.width, gpu.height, gpu.vmemsz);
  bool gpu_ok = gpu.present && gpu.width > 0 && gpu.height > 0;

  AM_INPUT_CONFIG_T inp = io_read(AM_INPUT_CONFIG);
  AM_INPUT_KEYBRD_T kbd = io_read(AM_INPUT_KEYBRD);
  printf("input: present=%d, polled keycode=%d keydown=%d\n",
         inp.present, kbd.keycode, kbd.keydown);

  if (gpu_ok) {
    uint32_t block[64];
    for (int i = 0; i < 64; i++) block[i] = 0x00336699;
    io_write(AM_GPU_FBDRAW, 0, 0, block, 8, 8, true);
    AM_GPU_STATUS_T st = io_read(AM_GPU_STATUS);
    printf("gpu fbdraw done, ready=%d\n", st.ready);
  }

  if (timer_ok && gpu_ok) {
    printf("AM PASS\n");
  } else {
    printf("AM FAIL (timer_ok=%d gpu_ok=%d)\n", timer_ok, gpu_ok);
  }
}

int main(const char *args) {
  switch (args[0]) {
    CASE('h', hello);
    CASE('i', hello_intr, IOE, CTE(simple_trap));
    CASE('d', devscan, IOE);
    CASE('m', mp_print, MPE);
    CASE('t', rtc_test, IOE);
    CASE('k', keyboard_test, IOE);
    CASE('v', video_test, IOE);
    CASE('a', audio_test, IOE);
    CASE('p', vm_test, CTE(vm_handler), VME(simple_pgalloc, simple_pgfree));
    case 's':
    case 0: {
      ioe_init();
      ioe_self_test();
      break;
    }
    case 'H':
    default:
      printf("Usage: make run mainargs=*\n");
      for (int ch = 0; ch < 256; ch++) {
        if (tests[ch]) {
          printf("  %c: %s\n", ch, tests[ch]);
        }
      }
      ioe_init();
      ioe_self_test();
  }
  return 0;
}
