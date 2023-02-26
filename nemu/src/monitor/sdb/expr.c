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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_HEX, TK_DEX, TK_EQ,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
  int priority;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE, 9},    // spaces
  {"0[xX][0-9a-fA-F]+", TK_HEX, 9}, //hex
  {"[0-9]+", TK_DEX, 9},   // dex
  {"==", TK_EQ, 7},        // equal
  {"\\+", '+', 4},         // plus, ascii = 43
  {"\\-", '-', 4},         // sub, ascii = 45
  {"\\*", '*', 3},         // mul, ascii = 42
  {"\\/", '/', 3},         // div, ascii = 47
  {"\\(", '(', 1},         // left bracket, ascii = 40
  {"\\)", ')', 1},         // right bracket,ascii = 41
  
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED); // 编译正则表达式
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
  int priority;
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

/*
 *    确定主运算符 
 */
static int determine_operator(int start, int end){
  int position = start, pri_min = 1, bra_count = 0;
  for(int i = start; i < end; i++){
    if(tokens[i].type == TK_NOTYPE || tokens[i].type ==TK_HEX || tokens[i].type == TK_DEX || tokens[i].type == TK_EQ)
      continue;
		bool flag = true;
		for (int j = i; j >= start; j--) { 
			if (tokens[j].type == '(') {
        if (bra_count == 0) {
          flag = false;
          break;
        }
        bra_count--;
      }
			if (tokens[j].type == ')')
        bra_count++; 
		}
		if (!flag)
      continue;
    if(tokens[i].priority > pri_min){
      position = i;
      pri_min = tokens[i].priority;
    }
  }
  return position;
}

/*
 *    判断表达式中括号数目是否匹配
 */
static bool check_parentheses(int start, int end){
  int bra_num = 0;
  if(tokens[start].type != '(')
    return false;
  for(int i = start; i <= end; i++){
    if(tokens[i].type == '('){
      bra_num++;
    }
    if(i == end && bra_num != 1)
      return false;
    else if(tokens[i].type == ')'){
      bra_num--;
    }
  }
  return bra_num == 0 ? true : false;
}

/*
 *    通过递归实现表达式求值
 */
static int eval(int start, int end) {
  if(start > end) assert(start < end);
  else if(start == end){
    return atoi(tokens[start].str);
  }
  else if(check_parentheses(start, end) == true){ // 
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     * 去除所有括号
     */
    return eval(start + 1, end - 1);
  }
  else {
    int op_position = determine_operator(start, end);
    int val1 = eval(start , op_position - 1);
    int val2 = eval(op_position + 1, end);
    switch (tokens[op_position].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      default: assert(0);
    }
  }
  return 0;
}


static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position; // matched token value
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type){
          case TK_NOTYPE: break;
          case 257: case 258: case 259:
            strcpy(tokens[nr_token].str, substr_start);
            tokens[nr_token].priority = rules[i].priority;
            tokens[nr_token++].type = rules[i].token_type;
          break;
          default:
            tokens[nr_token].priority = rules[i].priority;
            tokens[nr_token++].type = rules[i].token_type;
          break;
        }
        break;
      }
    }
     

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  int value = eval(0, nr_token-1);
  printf("the expr value is %d\n", value);

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();

  return 0;
}
