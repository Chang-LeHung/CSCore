# gcc 好玩的 builtin 函数

## 前言

在本篇文章当中主要想给大家介绍一些在 gcc 编译器当中给我们提供的一些好玩的内嵌函数 (builtin function)🤣🤣🤣 。

## __builtin_frame_address

### 使用内嵌函数实现

```c
__builtin_frame_address(x) // 其中 x 一个整数
```

 这个函数主要是用于得到函数的栈帧的，更具体的来说是得到函数的 rbp （如果是 x86_64 的机器，在 32 位系统上就是 ebp）的值，也就是栈帧的栈底的值。

![03](../../images/programming/03.png)

我们现在使用一个例子来验证测试一下：

```c

#include <stdio.h>

void func_a()
{
  void* p = __builtin_frame_address(0);
  printf("fun_a frame address = %p\n", p);
}


int main()
{
  void* p = __builtin_frame_address(0);
  printf("main frame address = %p\n", p);
  func_a();
  return 0;
}
```

上面的程序的输出结果如下所示：

```
main frame address = 0x7ffcecdd7a00
fun_a frame address = 0x7ffcecdd79d0
```

上面输出的结果就是每个函数的栈帧中栈底 rbp/ebp 寄存器的值，可能你会有疑问，凭什么说这个值就是 rbp 的值😂😂😂。我们现在来证明一下，我们可以使用代码获取得到 rbp 的值。

### 使用内敛汇编实现

```c


#include <stdio.h>
#include <sys/types.h>

u_int64_t rbp;

#define frame_address                   \
        asm volatile(                   \
          "movq %%rbp, %0;"             \
          :"=m"(rbp)::                  \
        );                              \
        printf("rbp = %p from inline assembly\n", (void*) rbp);

void bar()
{
  void* rbp = __builtin_frame_address(0);
  printf("rbp = %p\n", rbp);
  frame_address
}

int main()
{
  bar();
  return 0;
}
```

在上面的程序当中，我们使用一段宏可以得到寄存器 rbp 的值（在上面的代码当中，我们使用内敛汇编得到 rbp 的值，并且将这个值存储到变量 rbp 当中），我们将这个值和 builtin 函数的返回值进行对比，我们就可以知道返回的是不是寄存器 rbp 的值了，上面的程序执行结果如下所示：

```
rbp = 0x7ffe9676ac00
rbp = 0x7ffe9676ac00 from inline assembly
```

从上面的结果我们可以知道，内置函数返回的确实是寄存器 rbp 的值。

事实上我们除了可以获取当前函数的栈帧之外，我们还可以获取调用函数的栈帧，具体根据 x 的值进行确定：

- x = 0 : 获取当前函数的栈帧，也就是栈底的位置。
- x = 1 : 获取调用函数的栈帧。
- x = 2 : 获取调用函数的调用函数的栈帧。
- ......

比如说下面的程序：

```c
#include <stdio.h>

void func_a()
{
  void* p = __builtin_frame_address(1);
  printf("caller frame address = %p\n", p);
}


int main()
{
  void* p = __builtin_frame_address(0);
  printf("main frame address = %p\n", p);
  func_a();
  return 0;
}
```

上面程序的输出结果如下所示：

```
main frame address = 0x7ffda7a4b460
caller frame address = 0x7ffda7a4b460
```

从上面的输出结果我们可以看到当参数的值等于 1 的时候，返回的是调用函数的栈帧。

```c

#include <stdio.h>

void func_a()
{
  printf("In func_a\n");
  void* p = __builtin_frame_address(2);
  printf("caller frame address = %p\n", p);
}

void func_b()
{
  printf("In func_b\n");
  void* p = __builtin_frame_address(1);
  printf("caller frame address = %p\n", p);

  func_a();
}


int main()
{
  void* p = __builtin_frame_address(0);
  printf("main frame address = %p\n", p);
  func_b();
  return 0;
}
```

上面的程序的输出结果如下所示：

```
main frame address = 0x7ffdadbe6ff0
In func_b
caller frame address = 0x7ffdadbe6ff0
In func_a
caller frame address = 0x7ffdadbe6ff0
```

在上方的程序当中我们在主函数调用函数 func_b ，然后在函数 func_b 当中调用函数 func_a ，我们可以看到根据参数 x 的不同，返回的栈帧的层级也是不同的，根据前面参数 x 的意义我们可以知道，他们得到的都是主函数的栈帧。

## __builtin_return_address

### 使用内嵌函数实现

这个内嵌函数的主要作用就是得到函数的返回地址，首先我们需要知道的是，当我们进行函数调用的时候我们需要知道当这个函数执行完成之后返回到什么地方，因为 cpu 只会一条指令一条指令的执行，我们需要告诉 cpu 下一条指令的位置，因此当我们进行函数调用的时候需要保存调用函数的 call 指令下一条指令的位置，并且将它保存在栈上，当被调用函数执行完成之后继续回到调用函数的下一条指令的位置执行，因为我们已经将这个下一条指令的地址放到栈上了，当调用函数执行完成之后直接从栈当中取出这个值即可。

__builtin_return_address 的签名如下：

```c
__builtin_return_address(x) // x 是一个整数
```

其中 x 和前面的 __builtin_frame_address 含义相似：

- x = 0 : 表示当前函数的返回地址。
- x = 1 : 表示当前函数的调用函数的返回地址，比如说 main 函数调用 func_a 如果在 func_a 里面调用这个内嵌方法，那么返回的就是 main 函数的返回值。
- x = 2 : 表示当前函数的调用函数的调用函数的返回地址。

```c

#include <stdio.h>

void func_a()
{
  void* p = __builtin_return_address(0);
  printf("fun_a return address = %p\n", p);

  p = __builtin_return_address(1);
  printf("In func_a main return address = %p\n", p);
}


int main()
{
  void* p = __builtin_return_address(0);
  printf("main return address = %p\n", p);
  func_a();
  return 0;
}
```

上面的程序输出的结果如下：

```
main return address = 0x7fc5c57c90b3
fun_a return address = 0x400592
In func_a main return address = 0x7fc5c57c90b3
```

从上面的输出结果我们可以知道

### 使用内敛汇编实现

如果我们调用一个函数的时候（在x86里面执行 call 指令）首先会将下一条指令的地址压栈（在 32 位系统上就是将 eip 压栈，在 64 位系统上就是将 rip 压栈），然后形成调用函数的栈帧。然后将 rbp 寄存器的值指向下图当中的位置。

![01](../../images/pl/01.png)

```c

#include <stdio.h>
#include <sys/types.h>

#define return_address            \
    u_int64_t rbp;                \
    asm volatile(                 \
      "movq %%rbp, %0":"=m"(rbp)::\
    );                            \
    printf("From inline assembly return address = %p\n", (u_int64_t*)*(u_int64_t*)(rbp + 8));

void func_a()
{
  printf("In func_a\n");
  void* p = __builtin_return_address(0);
  printf("fun_a return address = %p\n", p);
  return_address
}

int main()
{
  printf("In main function\n");
  void* p = __builtin_return_address(0);
  printf("main return address = %p\n", p);
  return_address
  func_a();
  return 0;
}
```

上面的程序的输出结果如下所示：

```c
In main function
main return address = 0x7fe6a7b050b3
From inline assembly return address = 0x7fe6a7b050b3
In func_a
fun_a return address = 0x4005d2
From inline assembly return address = 0x4005d2
```

从上面的输出结果我们可以看到，我们自己使用内敛汇编直接得到寄存器 rbp 的和内嵌函数返回的值是一致的，这也从侧面反映出来了内嵌函数的作用。

在上面的代码当中定义定义的宏 return_address 的作用就是将寄存器 rbp 的值保存到变量 rbp 当中。

```c


#include <stdio.h>
#include <sys/types.h>

#define return_address            \
    u_int64_t rbp;                \
    asm volatile(                 \
      "movq %%rbp, %%rcx;"        \
      "movq (%%rcx), %%rcx;"      \
      "movq %%rcx, %0;"           \
      :"=m"(rbp)::"rcx"           \
    );                            \
    printf("From inline assembly return address = %p\n", (u_int64_t*)*(u_int64_t*)(rbp + 8));

void func_a()
{
  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  void* p = __builtin_return_address(1);
  printf("fun_a return address = %p\n", p);
  return_address
  printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}


int main()
{
  func_a();
  void* p = __builtin_return_address(0);
  printf("main function return address = %p\n", p);
  return 0;
}
```



```c
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
fun_a return address = 0x7fb35c1a70b3
From inline assembly return address = 0x7fb35c1a70b3
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
main function return address = 0x7fb35c1a70b3
```

至此我们已经知道了，__builtin_return_address 的返回结果是当前函数的返回地址，也就是当前函数执行完成返回之后执行的下一条指令，我们可以利用这一点做出一个非常好玩的东西，直接跳转到返回地址执行不执行当前函数的后续代码：

```c

#include <stdio.h>

void func_a()
{
  void* p        = __builtin_return_address(0); // 得到当前函数的返回地址
  void* rbp      = __builtin_frame_address(0);  // 得到当前函数的栈帧的栈底
  void* last_rbp = __builtin_frame_address(1);	// 得到调用函数的栈帧的栈底
  asm volatile(
    "leaq 16(%1), %%rsp;" // 恢复 rsp 寄存器的值
    "movq %2, %%rbp;"     // 恢复 rbp 寄存器的值
    "jmp *%0;"            // 直接跳转
    ::"r"(p), "r"(rbp), "r"(last_rbp): 
  );
  printf("finished in func_a\n"); // ①
}


int main()
{
  void* p = __builtin_return_address(0);
  printf("main return address = %p\n", p);
  func_a(); // ②
  printf("finished in main function \n");
  // 打印九九乘法表
  int i, j;
  for(i = 1; i < 10; ++i) 
  {
    for(j = 1; j <= i; ++j) {
      printf("%d x %d = %d\t", i, j, i * j);
    }
    printf("\n");
  }
  return 0;
}
```

上面的程序的输出结果如下所示：

```c
main return address = 0x7f63e05c60b3
finished in main function 
1 x 1 = 1
2 x 1 = 2       2 x 2 = 4
3 x 1 = 3       3 x 2 = 6       3 x 3 = 9
4 x 1 = 4       4 x 2 = 8       4 x 3 = 12      4 x 4 = 16
5 x 1 = 5       5 x 2 = 10      5 x 3 = 15      5 x 4 = 20      5 x 5 = 25
6 x 1 = 6       6 x 2 = 12      6 x 3 = 18      6 x 4 = 24      6 x 5 = 30      6 x 6 = 36
7 x 1 = 7       7 x 2 = 14      7 x 3 = 21      7 x 4 = 28      7 x 5 = 35      7 x 6 = 42      7 x 7 = 49
8 x 1 = 8       8 x 2 = 16      8 x 3 = 24      8 x 4 = 32      8 x 5 = 40      8 x 6 = 48      8 x 7 = 56      8 x 8 = 64
9 x 1 = 9       9 x 2 = 18      9 x 3 = 27      9 x 4 = 36      9 x 5 = 45      9 x 6 = 54      9 x 7 = 63      9 x 8 = 72      9 x 9 = 81
```

从上面程序的输出结果来看，上面的程序并没有执行语句 ① ，但是却执行了主函数 ② 之后的程序，并且正确输出字符串和九九乘法表。这就相当于我们提前进行了跳转。要想得到这样的结果，我们只需要在函数 func_a 内部恢复上一个函数的栈帧，并且将 rip 指向函数 func_a 的返回地址即可。

上方的程序发生转移的代码就是那段内敛汇编代码，在内敛汇编代码当中我们首先恢复 main 函数的栈帧（主要是正确恢复寄存器 rbp 和 rsp ）的值，然后直接跳转到返回地址继续执行，所以才正确执行了主函数后续的代码。

恢复主函数的 rbp 寄存器的值很好理解，因为我们只需要通过内嵌函数直接得到即可，但是主函数的 rsp 寄存器的值可能有一点复杂，s首先我们需要知道，主函数和 func_a 的两个与栈帧有关的寄存器的指向，他们的指向如下图所示：

![01](../../images/pl/02.png)