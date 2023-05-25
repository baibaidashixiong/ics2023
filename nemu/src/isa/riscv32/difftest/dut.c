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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  /* 
   * difftest: check with right register and pc for every instruction
   *  for riscv, the absolutely correct implementation tool is Spike
   */
  int reg_num = ARRLEN(cpu.gpr);
  for (int i = 0; i < reg_num; i++) {
    if (ref_r->gpr[i] != cpu.gpr[i]) {
      //printf("\33[1;31rigster[%d] gpr wrong!right gpr is 0x%x, wrong gpr is 0x%x\n\33[0m", i, ref_r->gpr[i] , cpu.gpr[i]);
      return false;
    }
  }
  if (ref_r->pc != cpu.pc) {
    printf("\33[1;31mpc wrong!right pc is 0x%x, wrong pc is 0x%x\n\33[0m",ref_r->pc, cpu.pc);
    return false;
  }
  if(ref_r->csr.mstatus != cpu.csr.mstatus) {
    printf("mstatus wrong!right mstatus is 0x%x, wrong status is 0x%x\n",ref_r->csr.mstatus, cpu.csr.mstatus);
    return false;
  }
  if(ref_r->csr.mtvec != cpu.csr.mtvec) {
    panic("mtvec wrong!right mtvec is 0x%x, wrong mtvec is 0x%x\n",ref_r->csr.mtvec, cpu.csr.mtvec);
    return false;
  }
  if(ref_r->csr.mcause != cpu.csr.mcause) {
    panic("mcause wrong!right mcause is 0x%x, wrong mcause is 0x%x\n",ref_r->csr.mcause, cpu.csr.mcause);
    return false;
  }
  if(ref_r->csr.mepc != cpu.csr.mepc) {
    panic("mepc wrong!right mepc is 0x%x, wrong mepc is 0x%x\n",ref_r->csr.mepc, cpu.csr.mepc);
    return false;
  }
  return true;
}

void isa_difftest_attach() {
}
