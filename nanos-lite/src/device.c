#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  /* ignore offset */
  for(int i = 0; i < len; i++)
      putch(*((const char*)buf + i));
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD); /* AM_INPUT_KEYBRD_T defined in amdev.h */
  if (ev.keycode == AM_KEY_NONE) {
    /* clear buffer
         question: is clear buffer necessary? why?
     */
    *(char*)buf = '\0';
    return 0;
  }
  int ret = snprintf(buf, len, "%s %s\n", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
  // printf("ret is %d\n", ret);
  return ret;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  /* doesn't support lseek, ignore offset */
  AM_GPU_CONFIG_T ev = io_read(AM_GPU_CONFIG);
  int width = ev.width;
  int height = ev.height;
  int ret = snprintf(buf, len, "WIDTH : %d\nHEIGHT : %d", width, height);
  // printf("%s\n", buf);
  return ret;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
