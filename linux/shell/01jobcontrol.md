# 你在终端启动的进程，最后都是什么下场？

## 前言

在本篇文章当中，主要给大家介绍我们在终端启动的进程都是怎么结束的，在我们登录终端和退出终端都发生了什么？

## 基本介绍

首先我们需要了解的概念就是当我们在终端启动一个程序之后会创建一个新的进程组，进程组的首进程为要执行的程序，这个进程组可以是一个进程也可以是多个进程，整个进程组在 shell 看来也是一个作业（Job）。

比如我们看下面的程序：

```c

#include <stdio.h>
#include <unistd.h>

int main()
{
  // 打印进程的 id 号
  printf("process id = %d\n", getpid());
  // 打印进程的进程组号 0 表示返回当前进程的进程组号
  printf("process group id = %d\n", getpgid(0));
  // 打印进程的父进程号
  printf("parent process id = %d\n", getppid());
  // 打印父进程的进程组号
  printf("parent process group id = %d\n", getpgid(getppid()));
  return 0;
}
```

上面的程序的输出结果如下所示：
```shell
➜  daemon git:(master) ✗ ./job1.out 
process id = 3773445
process group id = 3773445
parent process id = 3766993
parent process group id = 3766993
```

从上面程序的输出结果我们可以看到程序的进程组号和父进程的进程组号是不一样的，我们需要了解到的是，job1 的父进程就是 shell ，但是他与 shell 的进程组是不一样的，shell 在执行新的程序的时候会创建一个子进程，然后修改子进程的进程组号，而且新的进程组的组号为子进程的进程号。

或者我们直接在终端输入命令也可以发现 shell 的子进程的进程组号和 shell 的进程组号是不一样的：

```shell
➜  daemon git:(master) ✗ ps -o pid,ppid,pgid,tty,cmd
    PID    PPID    PGID TT       CMD
3766993 3757891 3766993 pts/1    /usr/bin/zsh -i
3772829 3766993 3772829 pts/1    ps -o pid,ppid,pgid,tty,cmd
```

在上面的输出当中，PID，PPID，PGID 分别表示进程的进程号，父进程号和进程组号，CMD 表示执行程序时候的命令。首先我们知道的是 ps 命令进程是 shell 进程的子进程（上面的进程号等于 3766993 的进程就是 shell 进程），从上面的输出结果也可以得知这一点（ ps 的 PPID 就是 shell 的 PID）。

通过上面两个例子我们可以知道，确实当我们执行程序的时候 shell 会创建一个新的进程组，事实上只要是在终端里面执行的程序，都会创建一个新的进程组。如果你熟悉 linux 的话，那么肯定用过 & 符号，这个符号就是将任务放在后台执行，这样创建的进程组就是后台进程组。

## 终端进程的下场

### 预备知识

为了后面我们进行验证的时候大家能够了解清楚程序的行为，我们首先介绍一下信号处理函数的使用，所谓信号处理函数就是，当进程收到一个由其他进程或者操作系统内核发送的信号的时候，我们可以定义一个函数去处理信号，也就是定义收到信号的行为：

```c

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void sig(int signo)
{
  char* s = "received a signal\n";
  write(STDOUT_FILENO, s, strlen(s));
}

int main()
{
  // 注册收到 SIGINT 信号的时候，我们应该使用什么处理函数
  // 当进程收到 SIGINT 信号的时候，会调用函数 sig 
  signal(SIGINT, sig);
  while(1);
  return 0;
}
```

上面的程序的输出结果如下所示：

```shell
➜  daemon git:(master) ✗ ./job4.out 
^Creceived a signal
^Creceived a signal
^Creceived a signal
^Creceived a signal
^Creceived a signal
^Creceived a signal
^Creceived a signal
```

从上面的终端的输出结果我们可以知道，当我们在终端输入 SIGINT 的时候，进程会收到一个 SIGINT 信号，然后会调用信号处理函数 sig ，并且执行函数体。



