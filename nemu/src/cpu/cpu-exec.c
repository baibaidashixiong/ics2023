/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <common.h>
#include "../monitor/sdb/sdb.h" /* anyway better? */

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 300000

/*
 * datastruct for itrace
 */
#ifdef CONFIG_ITRACE_COND
#define MAX_IRINGBUF 8
typedef struct {
  char logbuf[64];
} ItraceNode;

ItraceNode iringbuf[MAX_IRINGBUF];
uint32_t inode = 0;
#endif
/* 
 *  for riscv32, initialize mstatus as 0x1800 to support difftest 
 *   MPP(Machine Previous Privilege mode)[12,11] in mstatus
 *   It stores the previous privilege mode that the processor was in before entering machine mode.
 *   11: Machine mode, the most privileged mode and is typically used by 
 *      the operating system kernel or hypervisor to manage system resources.
 */
CPU_state cpu = {.csr.mstatus = 0x1800};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { 
    log_write("%s\n", _this->logbuf);
    int idx = inode++ % MAX_IRINGBUF;
    strcpy(iringbuf[idx].logbuf, _this->logbuf);
  }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
#ifdef CONFIG_WATCHPOINT
  wp_check(dnpc, &nemu_state.state);
  bp_check(dnpc, &nemu_state.state);
#endif
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
}

#ifdef CONFIG_FTRACE
/* frace:function trace */
int ftrace_dep = 0;
char a[2] = "  ";
extern FTRACE_FUNC ftrace_func[FUNC_NUM1];
char name[10] = "";
void ftrace_print(uint32_t inst_pc, uint32_t inst_des, uint32_t inst_val){
  /*
   *  jr  <-> jalr x0, 0(rs1)
   *  0x00078067 | 00000000 00000111 10000000 01100111
   *  t =pc+4; pc=(x[15](a5)+offset(0))&∼1; x[0]=t
   *  here return address aborted due to `tail-recursive`
   */
  if(inst_val == 0x78067) return;
  char s[50] = "";
  /*
   *  0x00008067 | 00000000 00000000 10000000 01100111
   *  t =pc+4; pc=(x[1]+offset(0))&∼1; x[0]=t
   *  &~1(1111 1110 in binary) 
   *  will effectively sets the least significant bit of the result to 0. 
   *    for word-aligned addresses.
   *  the least significant bit of a memory address in RISC-V architecture 
   *    is always 0 for word-aligned addresses. 
   *  x[1] will store the return address.
   */
  if(inst_val == 0x00008067) ftrace_dep--;
  for(int i = 0; i < FUNC_NUM1; i++){
    if(((ftrace_func[i].type & 0xf) == 2) && (inst_des == ftrace_func[i].addr)){
      strcpy(name, ftrace_func[i].func_name);
      break;
    }
  }

  for(int i = 0; i < ftrace_dep; i++){
      strcat(s, a);
  }
  if(inst_val != 0x00008067){
    ftrace_dep++;
    printf("0x%x: %scall [%s@%x]\n", inst_pc, s, name, inst_des);
  }
  else
    printf("0x%x: %sret [\33[1;31mTODO:FUNC_NAME\33[0m]\n",inst_pc, s);
  /*
   *  TODO : realize tail-recursive function name print
   */
}
#endif

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#endif
}

static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_ABORT:
      #ifdef CONFIG_ITRACE_COND
      puts("FAILED! instruction trace:");
      for(int i = 0; i< MAX_IRINGBUF; i++){
        if(unlikely(i == ((inode % MAX_IRINGBUF == 0) ? MAX_IRINGBUF - 1 : inode % MAX_IRINGBUF - 1)))
          printf("\33[1;31m --> %s\n\33[0m", iringbuf[i].logbuf);
        else
          printf("     %s\n", iringbuf[i].logbuf);
      }
      #endif
    case NEMU_END: 
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
