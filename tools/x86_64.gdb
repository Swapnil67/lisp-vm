break _start
run
tui enable
layout regs
display *(unsigned long long int*)&stack@10
display *(unsigned long long int **)&stack_top - (unsigned long long int *)&stack