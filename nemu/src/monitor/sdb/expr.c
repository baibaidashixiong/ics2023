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
#include <memory/paddr.h>

enum {
  TK_NOTYPE = 256, TK_HEX, TK_DEX, TK_REG, TK_EQ, TK_DEREF, TK_NEG

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
  {"\\$[a-z0-9]+", TK_REG, 9},       // reg
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
    if(tokens[i].type ==TK_HEX || tokens[i].type == TK_DEX || tokens[i].type == TK_EQ || tokens[i].type == TK_NEG)
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
    if(tokens[i].priority >= pri_min){ 
    /* 加等于号是因为同等优先级下最后被结合的才是主运算符 */
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
    if((i == end && bra_num != 1) || (i != end && bra_num == 0))
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
    if(tokens[start].type != TK_DEX && tokens[start].type != TK_HEX && tokens[start].type != TK_REG){/* 判断单个字符是否合法 */
      //*success = false;
      return 0;
    }
    else  return atoi(tokens[start].str);
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
    int val1 = 0, val2 = 0;
    if (op_position != start){
      val1 = eval(start , op_position - 1);
      val2 = eval(op_position + 1, end);
    }
    else if (tokens[op_position].type == TK_NEG || tokens[op_position].type == TK_DEREF){
      val2 = eval(op_position + 1, end);
    }
    switch (tokens[op_position].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': return val1 / val2;
      case TK_NEG : return 0 - val2;
      case TK_DEREF : return paddr_read(val2, 1);
      default: {
        printf("no match!");
        assert(0);}
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

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(rules[i].token_type == TK_NOTYPE) break;
        switch (rules[i].token_type){
          
          case TK_HEX:
            uint32_t hex_val = (uint32_t)strtol(substr_start, NULL, 16);
            sprintf(tokens[nr_token].str, "%d", hex_val);
          break;
          case TK_DEX: case TK_EQ:
            strcpy(tokens[nr_token].str, substr_start);
          break;
          case TK_REG:
            char *s =  substr_start + 1;
            bool *flag = false;
            uint32_t reg_val = isa_reg_str2val(s, flag);
            sprintf(tokens[nr_token].str, "%d", reg_val);
          break;
          default:
            strcpy(tokens[nr_token].str, substr_start);
          break;
        }
        tokens[nr_token].priority = rules[i].priority;
        tokens[nr_token++].type = rules[i].token_type;
        break;
      }
    }
     

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '*' && (i == 0 || 
      (tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_DEX 
        && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')') )) {
      tokens[i].type = TK_DEREF;
      tokens[i].priority = 7;
    }
    if (tokens[i].type == '-' && (i == 0 || 
      (tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_DEX 
        && tokens[i - 1].type != TK_REG && tokens[i - 1].type != ')') )) {
      tokens[i].type = TK_NEG;
      tokens[i].priority = 7;
    }
  }
  int value = eval(0, nr_token-1);
  //printf("the hex of expr value is 0x%08x, dex is %d\n", value, value);

  return value;
}
