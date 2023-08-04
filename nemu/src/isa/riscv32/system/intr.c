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

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  // set MPP to 11 to stay in machine-mode, NEMU will not switch privilege level
  cpu.csr.mstatus |= ((1<<11)+(1<<12));
  cpu.csr.mstatus &= ((((cpu.csr.mstatus >> 3) & 0x01) << 7) | ~(1 << 7));/* mpie = mie */
  cpu.csr.mcause = NO;
  /* store the code which indicating the event that caused the trap
   * Interrupt   Exception Code        Description
   *    1              11           Machine external interrupt
   *    1              7            Machine timer interrupt
   *    1              3            Machine software interrupt
   */
  cpu.csr.mepc = epc;

  return cpu.csr.mtvec;/* trap */
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
