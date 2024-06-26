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

#define INITIAL_TOKEN_SIZE 32
#define DEREF 265



static bool check_parentheses(int p, int q);
static int find_main_operator(int p, int q);
static uint32_t eval(int p, int q, bool *success);
extern void init_regex();
extern void init_tokens();
extern void free_tokens();
// extern uint32_t vaddr_read(uint32_t addr, int len);
extern word_t expr(char *e, bool *success);

enum {
  TK_NOTYPE = 256, 
  TK_EQ=257,
  TK_NUM=258,//10 INT
  TK_PLUS=259,
  TK_MINUS=260,
  TK_MUL=261,
  TK_DIV=262,
  TK_LPAREN=263,//(
  TK_RPAREN=264,//)
  TK_AND = 265,
  TK_NEQ = 266,
  TK_POINTER = 267,
  TK_HEX = 268,
  TK_REG_NAME = 269
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"[0-9]+",TK_NUM},
  {"\\+",TK_PLUS},         // plus
  {"\\-",TK_MINUS},  //
  {"\\*",TK_MUL},
  {"\\/",TK_DIV},
  {"\\(",TK_LPAREN}, 
  {"\\)",TK_RPAREN},
  {"\\&",TK_AND},
  {"\\!=",TK_NEQ},
  {"\\*",TK_POINTER},
  {"\\0X",TK_HEX},
  {"\\$",TK_REG_NAME}

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
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    // Log("ret%d\n",ret);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

// static Token tokens[32] __attribute__((used)) = {};
// static int nr_token __attribute__((used))  = 0;

static Token *tokens = NULL;
static int nr_token = 0;
static int max_tokens = INITIAL_TOKEN_SIZE;


// void init_tokens() {
//   tokens = (Token *)malloc(max_tokens * sizeof(Token));
//   if (tokens == NULL) {
//     fprintf(stderr, "Memory allocation failed\n");
//     exit(1);
//   }
// }


void ensure_token_capacity() {
  if (nr_token >= max_tokens) {
    max_tokens *= 2; // 将数组大小翻倍
    tokens = (Token *)realloc(tokens, max_tokens * sizeof(Token));
    if (tokens == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      exit(1);
    }
  }
}


// void free_tokens() {
//   if (tokens != NULL) {
//     free(tokens);
//   }
// }

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if ((regexec(&re[i], e + position, 1, &pmatch, 0) == 0) && (pmatch.rm_so == 0)) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            // i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        // switch (rules[i].token_type) {
    
        //   default: 
        //   TODO();
      // }7
        if (rules[i].token_type != TK_NOTYPE) 
        {
    
          ensure_token_capacity();
          tokens[nr_token].type = rules[i].token_type;
          strncpy(tokens[nr_token].str,       substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
        }

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

  // if (!make_token(e)) {
  //   *success = false;
  //   return 0;
  // }

  // /* TODO: Insert codes to evaluate the expression. */
  // // TODO();
  // uint32_t result   = eval(0,nr_token-1,success);

  // return result;
if (tokens == NULL) {
    tokens = (Token *)malloc(max_tokens * sizeof(Token));
    if (tokens == NULL) {
      fprintf(stderr, "Memory allocation failed\n");
      exit(1);
    }
  }

  nr_token = 0;

  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  for (int i = 0; i < nr_token; i++) 
  {
    if ((tokens[i].type == '*' )&& ((i == 0) || ((tokens[i - 1].type != TK_NUM )&& (tokens[i - 1].type != TK_RPAREN))) )
    {
      tokens[i].type = DEREF;
    }
    else if (tokens[i].type == TK_REG_NAME) 
    {
      bool reg_success;
      uint32_t reg_val = isa_reg_str2val(tokens[i].str, &reg_success);
      if (!reg_success) 
      {
        *success = false;
        return 0;
      }
      tokens[i].type = TK_NUM;
      snprintf(tokens[i].str, sizeof(tokens[i].str), "%u", reg_val);
    }
  }
  uint32_t fuck = eval(0, nr_token - 1, success);

  return fuck;

  // return 0;
}

static bool check_parentheses(int p, int q)
{
  // Log("cheack_parentheses,flag is %d\n",flag);
  if ((tokens[p].type ==TK_LPAREN)&&(tokens[q].type == TK_RPAREN))
  {
    /* code */
    int balance = 0;
    for(int i = p;i<=q;i++)
    {
      if(tokens[i].type == TK_LPAREN)
        {
          balance++;
        }
      if(tokens[i].type == TK_RPAREN)
        {
          balance--;
        }
      if((balance == 0)&&(i<q))
        {
          return false;
        }
    }
    return balance == 0;
  }
  return false;
  
}

uint32_t eval(int p, int q, bool *success)
{   
  if(p > q)
  {
    *success = false;
    // assert(0);
    return 0;
  }
  else if(p == q)
  {
    uint32_t val;
    // sscanf(tokens[p].str,"%u",&val);
    // return val;
      if (tokens[p].type == TK_NUM) {
      sscanf(tokens[p].str, "%u", &val);
      return val;
    } else if (tokens[p].type == TK_HEX) {
      sscanf(tokens[p].str, "%x", &val);
      return val;
    } else if (tokens[p].type == TK_REG_NAME) {
      return isa_reg_str2val(tokens[p].str, success);
    } else {
      *success = false;
      return 0;
    }


  }
  else if(check_parentheses(p,q)==true)
  {
    return eval(p + 1, q - 1,success);
  }
  else
  {
    int op = find_main_operator(p,q); 

  if (tokens[op].type == DEREF) {
      uint32_t addr = eval(op + 1, q, success);
      if (!*success) return 0;
      // Simulate dereferencing an address (should be replaced with actual memory access)
    
      // return vaddr_read(addr,4);
      return isa_reg_str2val((const char*)&addr, success);
    }


    int val1 = eval(p, op - 1,success);
    int val2 = eval(op + 1, q,success);

    // if(*success == false) 
    // {
    //   printf("enter fail branch kkkkkkkkkkkkkkkkkkkkkk");
    //   return 0;
    // }
    switch(tokens[op].type)
    {
      case TK_PLUS: 
        return val1 +val2;
      case TK_MINUS: 
        return val1 - val2;
      case TK_MUL: 
      {
        // printf("enter multiply");
        return val1 * val2;
      }
      case TK_DIV: 
        {
          if(val2 == 0)
          {
            *success = false;
            return 0;
          }
        return val1 / val2;
        }
      case TK_EQ:
        return val1 == val2;
      case TK_NEQ:
        return val1 != val2;
      case TK_AND:
        return val1 && val2;
      default: 
      {
      // assert(0);
      *success = false;
      return 0;
      }
    }

  }
}


int find_main_operator(int p, int q) {
  int min_priority = 3; // Start with a number higher than any operator priority
  int main_op = -1;
  int parentheses_count = 0;
     
  // Log("enter find main operator func,\n");
  // printf("enter find main operator func,\n");
  // printf("p is %d,q is %d\n",p,q);

  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_LPAREN) {
      parentheses_count++;
    }
    else if (tokens[i].type == TK_RPAREN) {
      parentheses_count--;
    }
    else if (parentheses_count == 0) {
      int priority = -1;
      switch (tokens[i].type) {
        case TK_PLUS:
        {
          priority = 1;
            break;
        }
        case TK_MINUS:
        {
          priority = 1;
          break;
        }
        case TK_MUL:
        {
          priority = 2;
          break;
        }
        case TK_DIV:
        {
          priority = 2;
          break;
        }
        case TK_EQ:
        {
          priority = 0;
          break;
        }
        case TK_NEQ:
        {
          priority = 0;
          break;
        }
        case TK_AND:
        {
          priority = -1;
          break;
        }
      }
      if ((priority > 0) && (priority <= min_priority)) {
        min_priority = priority;
        main_op = i;
       
      } 

    }
  }

  assert(main_op != -1); // There should always be a valid operator
  return main_op;
}