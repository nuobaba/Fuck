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
#include "common.h"
// #include "vaddr.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
  int N=1;
  sscanf(args,"%d",&N);
  for(int i = 0;i < N; i++)
  {  
    cpu_exec(1);
  }

  return 0;
}

static int cmd_info(char *args)
{
  // char *tmp = strtok(args, " ");

  // printf("tmp: %s\n",tmp);

  char *wr_sel;

  wr_sel = (char*)malloc(sizeof(char));

  sscanf(args,"%s",wr_sel);
  
  if(strcmp(wr_sel,"r")==0)
  {
    isa_reg_display();
  }
  else if(strcmp(wr_sel,"w") == 0)
  {
    printf("Do nothing.");
    TODO(); 
  }
  free(wr_sel);
  return 0;
}

static int cmd_x(char *args)
{
  int N_cnt;
  vaddr_t Address;

  sscanf(args,"%d %x",&N_cnt,&Address);

  for(int i = 0; i< N_cnt; i++)
  {
    word_t mem_data = paddr_read(Address+i,N_cnt);
    printf("0x%08x: 0x%02x\n", Address + i, mem_data & 0xFF);  
  }

  return 0;
}
static int cmd_cal(char *args)
{
  char *EXPR;
  bool *suc_flag;
  unsigned int result = 0;

  EXPR = (char*)malloc(sizeof(char));
  suc_flag = (bool*)malloc(sizeof(bool));
  // EXPR_RAND =

  sscanf(args,"%s",EXPR);

  // Log("EXPR IS %s\n",EXPR);

  result = expr(EXPR,suc_flag);
  Log("calculate result is %d",result);
  printf("calculate result is %d",result);

  free(EXPR);
  free(suc_flag);
  return 0;
}


static int cmd_v(char *args)
{
  char *EXPR;
  bool *suc_flag;
  // unsigned int result = 0;
  char filename[30] = "tools/gen-expr/expr";

  EXPR = (char*)malloc(sizeof(char));
  suc_flag = (bool*)malloc(sizeof(bool));

  FILE *file = fopen(filename,"r");
  if(file == NULL)
  {
    perror("Fail to open file");
    // return EXIT_FAILURE;
    return 0;
  }


  char line[1024];
    while (fgets(line, sizeof(line), file)) {
        int file_result;
        char expression[1024];

        // Read the result and expression from the file line
        if (sscanf(line, "%d %[^\n]", &file_result, expression) != 2) {
            fprintf(stderr, "Failed to parse line: %s", line);
            continue;
        }

        bool success;
        int computed_result = expr(expression, &success);

        if (!success) {
            fprintf(stderr, "Failed to evaluate expression: %s\n", expression);
            continue;
        }

        if (computed_result != file_result) {
            printf("Mismatch: computed_result = %d, file_result = %d, expression = %s\n",
                   computed_result, file_result, expression);
        } else {
            printf("Match: result = %d, expression = %s\n", file_result, expression);
        }
    }

    fclose(file);
  

  free(EXPR);
  free(suc_flag);
  return 0;
}




static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} 
cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si", "Single step, please enter in 'si N' format, N is the step conut.",cmd_si},
  { "info", "Print the rigester, please enter in 'info r' or 'info w'.",cmd_info},
  { "x", "Scan the memory, enter in 'x N EXPR' format,N is the continue bytes count,risv32 only=1/2/4, EXPR is an expression, now can be a adress.",cmd_x},
  { "cal","expression calculate.",cmd_cal},
  { "v" ,"check the value",cmd_v}, 
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
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
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

//NUO: read cmd
  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    /*NUO: space split*/
    char *cmd = strtok(str, " ");
    // /*NUO*/char *cmd = strtok(str, "enter in the command ");
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
        if (cmd_table[i].handler(args) < 0) { printf("NUO POINTS");return; }
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
