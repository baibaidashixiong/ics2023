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

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display(char *arg) {
  if(!arg){
    for(int i=0;i<32;i++){
      printf("%-3s: 0x%08x  |  ",regs[i],cpu.gpr[i]);
    if( (i+1) % 8 == 0) printf("\n");
    }
    printf("pc : 0x%08x\n",cpu.pc);
    printf("mstatus : 0x%08x\n",cpu.csr.mstatus);
    printf("mtvec : 0x%08x\n",cpu.csr.mtvec);
    printf("mcause : 0x%08x\n",cpu.csr.mcause);
    printf("mepc : 0x%08x\n",cpu.csr.mepc);
  }
  else {
    if (strcmp(arg, "pc") == 0) {
      printf("pc : 0x%08x\n",cpu.pc);
      return;
    }
    if (strcmp(arg, "mcause") == 0) {
      printf("mcause : 0x%08x\n",cpu.csr.mcause);
      return;
    }
    if (strcmp(arg, "mstatus") == 0) {
      printf("mstatus : 0x%08x\n",cpu.csr.mstatus);
      return;
    }
    if (strcmp(arg, "mepc") == 0) {
      printf("mepc : 0x%08x\n",cpu.csr.mepc);
      return;
    }
    if (strcmp(arg, "mtvec") == 0) {
      printf("mtvec : 0x%08x\n",cpu.csr.mtvec);
      return;
    }
    for (int i = 0; i < 32; i ++) {
      if (strcmp(arg, regs[i]) == 0) {
        printf("%-3s: 0x%08x\n",regs[i],gpr(i));
        break;
      }
    }
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  for (int i = 0; i < 32; i ++) {
    if (strcmp(s, regs[i]) == 0) {
      return gpr(i);
    }
  }
  if (strcmp(s, "pc") == 0) {
    return cpu.pc;
  }
  return 1;
}
