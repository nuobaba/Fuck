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

#include "sdb.h"
#include "expr.h"

#define NR_WP 32



typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[256];
  uint32_t last_value;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP *new_wp()
{
 if(free_ == NULL)
 {
  assert(0 && "NO FREE WATCGPOINTS AVAILABLE");
  return NULL;
 } 
  WP* wp = free_;
  free_ = free_->next;
  
  wp->next = head;
  head = wp;

  return wp;

}

void free_wp(WP *wp)
{
  WP *prev = NULL;
  WP *cur = head;

  while(cur != NULL)
  {
    if(cur == wp)
    {
      if(prev == NULL)
      {
        head = cur->next;
      }
      else
      {
        prev->next = cur->next;
      }
      wp->next = free_;
      free_ = wp;

      return;
    }
    prev = cur;
    cur = cur->next;
  }
  assert(0 && "Watchpoint not found in the active list!");
}

void add_watchpoint(char *expr_input) {
  WP *wp = new_wp();
  if (wp == NULL) {
    printf("No free watchpoints available.\n");
    return;
  }
  strncpy(wp->expr, expr_input, sizeof(wp->expr));
  bool success;
  wp->last_value = expr(expr_input, &success);
  if (!success) {
    printf("Failed to evaluate the expression: %s\n", expr_input);
    free_wp(wp);
  }
}

void remove_watchpoint(int no) {
  WP *wp = head;
  while (wp != NULL) {
    if (wp->NO == no) {
      free_wp(wp);
      return;
    }
    wp = wp->next;
  }
  printf("Watchpoint number %d not found.\n", no);
}

void check_watchpoints() {
  WP *wp = head;
  while (wp != NULL) {
    bool success;
    uint32_t new_value = expr(wp->expr, &success);
    if (!success) {
      printf("Failed to evaluate the expression: %s\n", wp->expr);
    } else if (new_value != wp->last_value) {
      printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
      printf("Old value: %u\nNew value: %u\n", wp->last_value, new_value);
      wp->last_value = new_value;
    }
    wp = wp->next;
  }
}

void print_watchpoints() {
  WP *wp = head;
  if (wp == NULL) {
    printf("No watchpoints.\n");
    return;
  }
  while (wp != NULL) {
    printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
    wp = wp->next;
  }
}

void delete_watchpoint(int no) {
  WP *wp = head;
  while (wp != NULL) {
    if (wp->NO == no) {
      free_wp(wp);
      printf("Watchpoint %d deleted.\n", no);
      return;
    }
    wp = wp->next;
  }
  printf("Watchpoint %d not found.\n", no);
}