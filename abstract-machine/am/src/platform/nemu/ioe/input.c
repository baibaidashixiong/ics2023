#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t k = inl(KBD_ADDR);/* default is AM_KEY_NONE */
  /*  
   *  the most significant bit of k is set to 1 
   *  indicating that the key is being pressed down
   */
  kbd->keydown = (k & KEYDOWN_MASK ? true : false);
  /* 
   *  the value of each keycode can according to x86/qemu/ioe.c
   */
  kbd->keycode = k & ~KEYDOWN_MASK;
}
