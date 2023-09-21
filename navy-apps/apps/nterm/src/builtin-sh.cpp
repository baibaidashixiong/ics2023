#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

#define fname_buf_size 64
static char fname[64];
char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(const char *cmd) {
  char buf[64];
  strcpy(buf, cmd);
  char *command = strtok(buf, " ");
  char *argv = strtok(NULL," ");
  if(strcmp(buf, "echo") == 0){
    /* fulfill the simplest echo command */
    char *value = strtok(NULL, "\n");
    sh_printf("%s\n", value);
  } else {
        if (strlen(cmd) > fname_buf_size) {
            sh_printf("command too long\n");
            return;
        }
        strncpy(fname, cmd, strlen(cmd) - 1);
        printf("zqz  %s\n",fname);
        // execve(fname, NULL, NULL);/* exec the whole input file */
        execvp(fname, NULL);/* exec env cat input file name */
    }
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  assert(setenv("PATH", "/bin", 0) == 0);
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
