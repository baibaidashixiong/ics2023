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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

word_t expr(char *e, bool *success);
typedef struct watchpoint {
  int NO;
  char addr[32];
  /*
   *   here must be the char array char [], but not the string char *,
   *   because when use the char *, the address of args will be stored in char *,
   *   next time input command [wd], args turn to null, so the output of wp->addr 
   *   will be null. 
   */
  int value;
  struct watchpoint *next;
} WP;
WP* new_wp(char *args);
void free_wp(int NO);
void wp_dis();
void wp_check();
void printf_bin();


#endif
