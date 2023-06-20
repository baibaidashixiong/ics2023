#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>
#include <sys/time.h>

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  return 1;
}

uint32_t SDL_GetTicks() {
  /*
   * Returns an unsigned 32-bit value representing the number
   *  of milliseconds since the SDL library initialized.
   */
  struct timeval tv;
  gettimeofday(&tv, NULL);
  /* 1 second = 1000 millisecond(ms) = 1000,000 microsecond */
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/* Wait a specified number of milliseconds before returning. */
void SDL_Delay(uint32_t ms) {
  uint32_t start = SDL_GetTicks();
  while (SDL_GetTicks() - start <= ms){}
}
