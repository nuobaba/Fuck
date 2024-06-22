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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
// "  unsigned result = %s; "
"  int result = %s; "
// "  printf(\"%%u\", result); "
"  printf(\"%%d\", result); "
"  return 0; "
"}";

static int choose(int n)
{
  int kkk = 0;
  kkk = rand() % n;
  // printf("kkk is %d\n",kkk);

  return kkk;
}

static void gen_num()
{
  int num = rand() % 100;
  char *num_str;
// char num_str[12];
  int num_len = snprintf(NULL, 0, "%d", num);
  num_str = (char*)malloc(num_len + 1); 

  sprintf(num_str,"%d",num);
  strcat(buf,num_str);

  free(num_str);
}

static void gen(char operator)
{
  int len = strlen(buf);
  buf[len] = operator;
  buf[len + 1] = '\0';
}

static void gen_rand_op()
{
  switch(choose(4))
  {
    case 0:
    {
      gen('+');
      break;
    }
      case 1:
    {
      gen('-');
      break;
    }
      case 2:
    {
      gen('*');
      break;
    }
      case 3:
    {
      gen('/');
      break;
    }
  }

}


static void gen_rand_expr(int depth) {
    // buf[0] = '\0';
    if(depth > 10)
    {
      gen_num();
      return;
    }
    switch (choose(3)) {
    case 0: 
    
      gen_num();
      break;
    
    case 1:
    
      gen('('); 
      gen_rand_expr(depth); 
      gen(')'); 
      break;
    
    default: 
    
      gen_rand_expr(depth+1); 
      gen_rand_op(); 
      gen_rand_expr(depth+1); 
      break;
    
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  int main_depth = 3;
  srand(seed);
  int loop = 1;
 
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';
    gen_rand_expr(main_depth);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
