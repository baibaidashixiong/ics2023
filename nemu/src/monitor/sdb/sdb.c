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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/paddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
/* print binary numbers */
void printf_bin(int num)
{
	int i, j, k;
	char *p = (char*)&num + 3;/* p指向num的最高字节地址 */
	for (i = 0; i < 4; i++){
		j = *(p - i); //取每个字节的首地址，从高位字节到低位字节，即p p-1 p-2 p-3地址处
		for (k = 7; k >= 0; k--){
			if (j & (1 << k))
				printf("1");
			else
				printf("0");
		}
		printf(" ");
	}
	printf("\r\n");
}

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(zqz-nemu): ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) cpu_exec(-1);
  else if((arg[0]-48) > 0) {
    for(int i = 0; i < atoi(arg); i++) {
      cpu_exec(-1);
    }
  }
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) cpu_exec(1);
  else if ((arg[0]-48) > 0)// '0'的ascii是48
    cpu_exec(atoi(arg));
  else 
    return 1;
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  isa_reg_display(arg);
  return 0;
}

static int cmd_x(char *args) {
  int len = atoi(strtok(NULL, " "));
  char *addr = strtok(NULL, " ");
  uint32_t addr_t, addr_p;
  sscanf(addr, "%x",&addr_t);
  //uint32_t content ;
  printf("\t     high addr  <--  low addr\n");
  printf("\t     ------------------------\n");
  for( int i =0; i < len; i++ ){
    addr_p = paddr_read(addr_t, 4);
    printf("\033[0;34m0x%x: ", addr_t);
    printf("\033[0m 0x%08x | ", addr_p);
    printf_bin(addr_p);
    // for(int k = 3; k >= 0; k--){ /* 打印十进制 */
    //   printf("\033[0m %03d ", paddr_read(addr_t + (k * 1), 1));
    // }
    //printf("\n");
    addr_t += 4;
  }
  printf("\n");
  return 0;
}

static int cmd_p(char *args) {
  bool *success = false;
  int value = expr(args, success);
  printf("the hex of expr value is 0x%08x, dex is %d\n", value, value);
  return 0;
}

static int cmd_w(char *args) {
  char *arg = strtok(NULL, " "); 
  new_wp(arg);
  return 0;
}

static int cmd_b(char *args) {
  char *arg = strtok(NULL, " ");
  new_bp(arg);
  return 0;
}

static int cmd_d(char *args) {
  int NO = 0 ;
  if(!args) {
    printf("please input the number of watchpoint!\n");
    wp_dis();
    return 0;
  }
  NO = (int)strtol(args, NULL, 10);
  free_wp(NO);
  return 0;
}

static int cmd_wd(char *args) {
  wp_dis();
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program, c N to execute N times in debug mode.", cmd_c },
  { "si", "Execute N instructions, default N is 1. usage: si N", cmd_si },
  { "info", "Print register status and monitor information. usage: info reg_name", cmd_info },
  { "x", "x N EXPR(addr). Find the value of the expression EXPR and use the result as the starting memory address, output N consecutive 4 bytes in hexadecimal format", cmd_x },
  { "p", "p the value of expr. usage: p <expr>($reg_name or *addr)", cmd_p },
  { "w", "pause program execution when the value of the expression EXPR changes.\n \
         usage: w $reg_name or w *addr", cmd_w },
  { "b", "add a breakpoint. usage: b address ", cmd_b },
  { "d", "delete the number of the watchpoint", cmd_d },
  { "wd", "display watchpoint", cmd_wd },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%-4s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
