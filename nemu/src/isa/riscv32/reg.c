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
#include "local-include/reg.h"
#include "include/isa-def.h"

#if defined(CONFIG_RV64)
iscv64_CPU_state My_Reg;
riscv64_ISADecodeInfo Reg_Val;
#else
riscv32_CPU_state My_Reg;
riscv32_ISADecodeInfo Reg_Val;
#endif

#if defined(CONFIG_RVE)
int REG_CNT = 16;
#else
int REG_CNT = 32;
#endif

// extern MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state) myreg;

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {

  for (int i = 0;i<REG_CNT;i++)
  {
    printf("%s: 0x%08u\n", regs[i], My_Reg.gpr[i]);
  }
     // 打印程序计数器 pc 的值
    printf("pc: 0x%08u\n", My_Reg.pc);
  }

word_t isa_reg_str2val(const char *s, bool *success) {
    for (int i = 0;i<REG_CNT;i++)
    {
    if(strcmp(s,regs[i]) == 0)
    {
      *success = true;
      
      return My_Reg.gpr[i];
    }

    }   
    
      *success = false;
      printf("no that register name :%s \n",s);

      return 0;
    

  }
  



// typedef struct {
//   word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
//   vaddr_t pc;
// } MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);