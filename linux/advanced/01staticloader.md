# 静态加载器

通常我们程序的加载时通过fork execve系统调用进行加载的。然后在execve当中会对elf文件进行解析，将程序头表当中指定的需要加载的数据，使用mmap将数据加载到内存当中。如果我们想要不使用系统调用去加载程序的话，也就是说在用户态加载程序的话，我们就需要模拟execve的功能。

程序的执行就是不断的从内存当中加载指令然后译码执行，如果想要完成我们自己的加载器我们只需要给进程创建它需要的初始状态即可（execve也是给进程创建一个初始状态，然后将程序的第一条指令地址放入rip，程序就执行起来了），程序的初始状态主要有两个部分一个是栈的状态，另外一个是寄存器的状态。

一个进程的初始状态在手册当中是确定下来的，我们只需要了解手册当中对程序初始状态的规定，然后将代码和数据映射到地址空间当中即可，而代码和数据映射到地址空间中的什么位置在程序头表当中已经设置好了，我们只需要从elf文件当中读入程序头表然后得到虚拟地址空间和elf文件之间的映射关系，然后将elf文件需要加载的位置，加载到对应的虚拟地址空间即可，这里主要是为程序的执行准备好了代码和数据，下一步就是需要准备寄存器状态。

上面是手册对于一个进程初始状态的规定，寄存器rsp指向的位置存放的是命令行参数的个数，也就是argc，之后从低地址向高地址变化的依次为每一个argv指针，在argv的结束位置放入一个8字节的0，也就是NULL，然后继续放入系统环境变量的值，最后在系统环境变量结束后的位置，再放入一个8字节的0，最后在放入一些辅助向量的结构体，并且最后一个辅助向量为 AT_NULL，表示辅助向量的结束（后面没有辅助向量了）。整个栈的空间变化大致如下（下面是32位的情况，64为也大致相同，只是占用的字节大小发生了变化）

```
position            content                     size (bytes) + comment
  ------------------------------------------------------------------------
  stack pointer ->  [ argc = number of args ]     4
                    [ argv[0] (pointer) ]         4   (program name)
                    [ argv[1] (pointer) ]         4
                    [ argv[..] (pointer) ]        4 * x
                    [ argv[n - 1] (pointer) ]     4
                    [ argv[n] (pointer) ]         4   (= NULL)

                    [ envp[0] (pointer) ]         4
                    [ envp[1] (pointer) ]         4
                    [ envp[..] (pointer) ]        4
                    [ envp[term] (pointer) ]      4   (= NULL)

                    [ auxv[0] (Elf32_auxv_t) ]    8
                    [ auxv[1] (Elf32_auxv_t) ]    8
                    [ auxv[..] (Elf32_auxv_t) ]   8
                    [ auxv[term] (Elf32_auxv_t) ] 8   (= AT_NULL vector)

```

寄存器的初始状态如下：

- rip 指向第一条指令
- rsp 指向栈顶（上面所谈到的进程的初始栈）。
- rdx 赋值为0即可。



静态加载器的代码：

```c
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>

#define STK_SZ           (1 << 20) // 设置程序执行栈的大小
#define ROUND(x, align)  (void *)(((uintptr_t)x) & ~(align - 1)) // 使用mmap函数的时候需要保证4kb对其 这个宏主要是用于定义 x 按照 align 字节对齐 如果没有对齐则返回能够按照align对齐的位置最大值
#define MOD(x, align)    (((uintptr_t)x) & (align - 1))
// 下面这个宏主要是往栈上存入数据 并且将 sp 的值加上对应数据的大小 以便后续继续给栈赋值
#define push(sp, T, ...) ({ *((T*)sp) = (T)__VA_ARGS__; sp = (void *)((uintptr_t)(sp) + sizeof(T)); })

void execve_(const char *file, char *argv[], char *envp[]) {
  // WARNING: This execve_ does not free process resources.
  int fd = open(file, O_RDONLY);
  assert(fd > 0);
  // 读入 elf 文件
  Elf64_Ehdr *h = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(h != (void *)-1);
  // 确保是静态加载的可执行文件
  assert(h->e_type == ET_EXEC && h->e_machine == EM_X86_64);
	// 得到程序头表
  Elf64_Phdr *pht = (Elf64_Phdr *)((char *)h + h->e_phoff);
  // 遍历程序头表当中的每一项然后将需要加载的项（PT_LOAD）加载进入内存
  for (int i = 0; i < h->e_phnum; i++) {
    Elf64_Phdr *p = &pht[i];
    if (p->p_type == PT_LOAD) {
      int prot = 0;
      // 设置读写执行权限
      if (p->p_flags & PF_R) prot |= PROT_READ;
      if (p->p_flags & PF_W) prot |= PROT_WRITE;
      if (p->p_flags & PF_X) prot |= PROT_EXEC;
      // 将数据从磁盘当中加载进入内存
      // 这里需要注意对齐的问题 尤其是加载进入内存的长度
      // elf 满足一个 assertion MOD(p->p_vaddr, p->p_align) == MOD(p->p_offest, p->p_align)
      void *ret = mmap(
        ROUND(p->p_vaddr, p->p_align),              // addr, rounded to ALIGN
        p->p_memsz + MOD(p->p_vaddr, p->p_align),   // length
        prot,                                       // protection
        MAP_PRIVATE | MAP_FIXED,                    // flags, private & strict
        fd,                                         // file descriptor
        (uintptr_t)ROUND(p->p_offset, p->p_align)); // offset
      assert(ret != (void *)-1);
      // 将多余的空间复制为0
      memset((void *)(p->p_vaddr + p->p_filesz), 0, p->p_memsz - p->p_filesz);
    }
  }
  close(fd);
	// 申请栈空间
  static char stack[STK_SZ], rnd[16];
  void *sp = ROUND(stack + sizeof(stack) - 4096, 16);
  void *sp_exec = sp;
  int argc = 0;

  // argc
  while (argv[argc]) argc++;
  // 根据手册将 argc 放入栈中
  push(sp, intptr_t, argc);
  // argv[], NULL-terminate
  // 根据手册将 argv 当中所所有的参数传入栈中
  for (int i = 0; i <= argc; i++)
    push(sp, intptr_t, argv[i]);
	// 根据手册 结束的位置需要传入一个0
  push(sp, intptr_t, 0);

  // glibc 将会使用到 AT_RANDOM 主要是用于避免栈溢出攻击 因此下面这行代码一定不能够少
  push(sp, Elf64_auxv_t, { .a_type = AT_RANDOM, .a_un.a_val = (uintptr_t)rnd } );
  push(sp, Elf64_auxv_t, { .a_type = AT_NULL } ); // 这一行表示辅助向量的结束

  asm volatile(
    "mov $0, %%rdx;" // 手册规定
    "mov %0, %%rsp;" // 将栈顶 sp_exec 传给 rsp 寄存器
    "jmp *%1" : : "a"(sp_exec), "b"(h->e_entry)); // 跳到程序的入口地址
}

int main(int argc, char *argv[], char *envp[]) {
  if (argc < 2) {
    // argv[1] 是需要加载的程序的名字
    exit(1);
  }
  execve_(argv[1], argv + 1, envp);
}

```
