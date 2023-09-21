#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  // printf("sys_sbrk: %d\n", n);
  addr = myproc()->sz;
  // printf("myproc()->sz %p\n", addr);
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  // argaddr();
  struct proc* p = myproc();
  uint64 va_begin, va_bitmask;
  int num;
  argaddr(0, &va_begin);
  argint(1, &num);
  argaddr(2, &va_bitmask);
  uint32 mask = 0;
  // printf("num = %d\n", num);
  uint64 complement = PTE_A;
  complement = ~complement;
  // printf("complement = %p\n", complement);
  for (int i = 0; i < num; ++i) {
    uint64 va = va_begin + i * PGSIZE;
    // printf("va = %p\n", va);
    pte_t* pte = walk(p->pagetable, va, 0);
    if (PTE_FLAGS(*pte) & PTE_A) {
      mask |= (1 << i);
      printf("sys_pgaccess i = %d\n", i);
      *pte &= complement; // equal *pte &= 0xffffffffffffffbf;
    }
  }
  copyout(p->pagetable, va_bitmask, (char*)&mask, sizeof(mask));
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
