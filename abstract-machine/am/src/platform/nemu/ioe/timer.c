#include <am.h>
#include <nemu.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  /*
   * questions to be resolved: if change the order of inl will cause score high in dhrystone benchmark? 
   *  Answer: rtc_port_base first get the high 32 bits(because offset = 4) of time reg, then low 32 bits.
   *          this will cause grade wrong
   */
  uptime->us = (uint64_t)inl(RTC_ADDR + 0x4) << 32 | (uint64_t)inl(RTC_ADDR)  ;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
