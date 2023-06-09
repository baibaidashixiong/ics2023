#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

enum SDL_EventType SDLD_TYPE_CHOOSE(const char* str) {
  for(int i = 0; i < sizeof(keyname); i++){
    if(strcmp(str, keyname[i]) == 0){
      return i;
    }
  }
  return SDLK_NONE;
}

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

/* event distribution */
int SDL_PollEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  event->type = SDL_KEYUP;/* refresh the status of event.type */
  char buf[16];
  int ret = NDL_PollEvent(buf, sizeof(buf));/* obtain key information */
  char *key_type = strtok(buf, " ");
  if(!ret || (strcmp(key_type, "kd") != 0)){
    /* returns 1 on success or 0 if there was an error while waiting for events */
    return 0;
  }
  /* be careful, there will be a `\n` written into buf, so it should be split here */
  char *key_value = strtok(NULL, "\n");
  event->type = SDL_KEYDOWN;
  // sprintf(key_value, "%s", key_value);
  event->key.keysym.sym = SDLD_TYPE_CHOOSE(key_value);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  assert(0);
  return NULL;
}
