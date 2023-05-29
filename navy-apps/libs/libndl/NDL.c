#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

uint32_t NDL_GetTicks() {
  /* according to SDL_GetTicks
   * Returns an unsigned 32-bit value representing the number
   *  of milliseconds since the NDL library initialized.
   */
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fp = open("/dev/events", O_RDONLY);
  int ret = read(fp, buf, len);
  assert(close(fp) == 0);
  return ret == 0 ? 0 : 1;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (*w == 0){
    *w = screen_w;
  }else if(*w > screen_w){
    assert(0);
  }
  if (*h == 0){
    *h = screen_h;
  }else if(*h > screen_h){
    assert(0);
  }
  printf("opencanvas screen_w is %d, screen_h is %d\n", screen_w, screen_h);

  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

static void read_key_value(char *str, char *key, int* value){
  char buffer[128];
  int len = 0;
  for (char* c = str; *c; ++c){
    if(*c != ' '){
      buffer[len++] = *c;
    }
  }
  buffer[len] = '\0';
  /* %[a-zA-Z] matches any sequence of alphabetical characters */
  sscanf(buffer, "%[a-zA-Z]:%d", key, value);
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }

  char info[128], key[64];
  int value;

  int dispinfo = open("/proc/dispinfo", 0);
  /* The entire call process:
      navy:read -> navy:syscall -> nanos:fs_read -> callback func:nanos:dispinfo_read
            then return here and set screen size information to buf
     question: What is the conversion process of navy:read to navy:syscall(fs_read)?
   */
  read(dispinfo, info, sizeof(info)); /* read screen size information through callback function dispinfo_read to info */
  close(dispinfo);

  /* get string format WIDTH : %d */
  char *token = strtok(info, "\n");
  /* get remain string */
  while( token != NULL ) {
    read_key_value(token, key, &value);
    if(strcmp(key, "WIDTH") == 0){
      screen_w = value;
    } else if(strcmp(key, "HEIGHT") == 0) {
      screen_h = value;
    }
    token = strtok(NULL, "\n");
  }
  printf("width = %d, height = %d.\n", screen_w, screen_h);
  return 0;
}

void NDL_Quit() {
}
