// static bool check_parentheses(int p, int q);
// static int find_main_operator(int p, int q);
// static uint32_t eval(int p, int q, bool *success);
extern void init_regex();
extern void init_tokens();
extern void free_tokens();
// extern uint32_t vaddr_read(uint32_t addr, int len);
extern word_t expr(char *e, bool *success);