#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

const char *eventname[] = {
  "kd",
  "ku",
};

/* key press event choose */
enum SDL_EventType SDLEvent_TYPE_CHOOSE(const char* str) {
  for(int i = 0; i < sizeof(eventname) / sizeof(char *); i++){
    if(str && strcmp(str, eventname[i]) == 0){
      return i;
    }
  }
  return -1;
}

/* key_DOWN type choose */
enum SDL_EventType SDLD_TYPE_CHOOSE(const char* str) {
  for(int i = 0; i < sizeof(keyname) / sizeof(char *); i++){
    if(str && strcmp(str, keyname[i]) == 0){
      return i;
    }
  }
  return SDLK_NONE;
}

/* event distribution for SDL Keyboard events */
int SDLK_PollEvent(SDL_Event *ev, uint8_t SDL_EventType) {
  char buf[16];
  /* obtain key information, return 1 if keyevent happend */
  int ret = NDL_PollEvent(buf, sizeof(buf));
  char *key_type = strtok(buf, " ");
  if(ret) {
    ev->type = SDLEvent_TYPE_CHOOSE(key_type);
    // printf("evt type is %d, \n", ev->type);
    /* be careful, there will be a `\n` written into buf, so it should be split here */
    char *key_value = strtok(NULL, "\n");
    ev->key.keysym.sym = SDLD_TYPE_CHOOSE(key_value);
    return 1;
  }
  ev->type = SDL_USEREVENT;
  return 0;
}

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

/* event distribution */
int SDL_PollEvent(SDL_Event *ev) {
  return SDLK_PollEvent(ev, SDL_USEREVENT);
}

int SDL_WaitEvent(SDL_Event *event) {
  return SDLK_PollEvent(event, SDL_USEREVENT);
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  assert(0);
  return NULL;
}
