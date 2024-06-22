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



static bool check_parentheses(int p, int q);
static int find_main_operator(int p, int q);
static uint32_t eval(int p, int q, bool *success);
extern void init_regex();
int flag = 0 ;

enum {
  TK_NOTYPE = 256, 
  TK_EQ=257,
  TK_NUM=258,//10 INT
  TK_PLUS=259,
  TK_MINUS=260,
  TK_MUL=261,
  TK_DIV=262,
  TK_LPAREN=263,//(
  TK_RPAREN=264//)
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
  {"\\)",TK_RPAREN}


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

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

Token *tokens = NULL; 
int nr_token = 0;     
int max_tokens = 32;

void init_tokens() {
    tokens = (Token *)malloc(max_tokens * sizeof(Token));
    if (tokens == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
}

void free_tokens() {
    if (tokens != NULL) {
        free(tokens);
    }
}

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

void add_token(int type, const char *str) {
    ensure_token_capacity();
    tokens[nr_token].type = type;
    strncpy(tokens[nr_token].str, str, sizeof(tokens[nr_token].str) - 1);
    tokens[nr_token].str[sizeof(tokens[nr_token].str) - 1] = '\0'; // 确保字符串以空字符结尾
    nr_token++;
}










static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;
  flag++;
  // Log("NR_REGEX val is %d\n, flag = %d ",NR_REGEX,flag);  
  // Log("i val is %d\n ",i);  
  //  Log("e is %s\n",e);
  //  Log("E[position] is %c\n",e[position]);
  
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      // Log("enter in for,i = %d\n",i);
      // int reghhhh = regexec(&re[i], e + position, 1, &pmatch, 0); 
      // Log("compare seq = %d\n",reghhhh);

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
          // flag++;
          // Log("enter in TK_NOTYPE,flag is %d",flag);
          tokens[nr_token].type = rules[i].token_type;
          strncpy(tokens[nr_token].str,       substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      flag++;
      // Log("enter in NR_REGEX,flag is %d",flag);
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  // Log("enter expr func");
  // Log("make_token(e) = %d\n",make_token(e));
  if (!make_token(e)) {
    flag++;
    // Log("enter in fail banch");
    // Log("e is %s\n,flag is %d",e,flag);
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  return eval(0,nr_token-1,success);
  // return 0;
}

static bool check_parentheses(int p, int q)
{
      flag++;
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
{   flag++;
    // Log("enter eval func\n");
    // printf("success flag is %s\n",success);
    // printf("enter eval func\n");
    // printf("p is %d,q is %d\n",p,q); 
  if(p > q)
  {
    *success = false;
    // assert(0);
    return 0;
  }
  else if(p == q)
  {
    uint32_t val;
    sscanf(tokens[p].str,"%u",&val);
    return val;
  }
  else if(check_parentheses(p,q)==true)
  {
    return eval(p + 1, q - 1,success);
  }
  else
  {
    int op = find_main_operator(p,q); 
    int val1 = eval(p, op - 1,success);
    int val2 = eval(op + 1, q,success);
  // printf("val1 = %d,val2 = %d\n",val1,val2);
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
     
  flag++;
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
      }
      if ((priority > 0) && (priority <= min_priority)) {
        min_priority = priority;
        main_op = i;
       
      } 
      // printf("min_priority = %d,main_op = %d\n",min_priority,main_op);
    }
  }

  assert(main_op != -1); // There should always be a valid operator
  return main_op;
}