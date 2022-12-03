# 你在终端启动的进程，最后都是什么下场？（下）

在上期文章[你在终端启动的进程，最后都是什么下场？（上）](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487367&idx=1&sn=430b3afe8399ed32cf832d942b305a5d&chksm=cf0c938ef87b1a9803b2db922463612e5729579802d733ad6fd778d0c0d8439766b43af2801e&token=1948688554&lang=zh_CN#rd)当中我们介绍了前台进程最终结束的几种情况，在本篇文章当中主要给大家介绍后台进程号可能被杀死的几种情况。

## 揭秘nohup——后台进程的死亡

如果大家有过让程序在后台持续的运行，当你退出终端之后想让你的程序继续在后台运行，我们通常会使用命令 nohup。那么现在问题来了，为什么我们让程序在后台运行需要 nohup 命令，nohup 命令又做了什么？

在前面的文章[你在终端启动的进程，最后都是什么下场？（上）](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487367&idx=1&sn=430b3afe8399ed32cf832d942b305a5d&chksm=cf0c938ef87b1a9803b2db922463612e5729579802d733ad6fd778d0c0d8439766b43af2801e&token=1948688554&lang=zh_CN#rd)当中我们已经谈到了，当你退出终端之后 shell 会发送 SIGHUP 信号给前台进程组的所有进程，然后这些进程在收到这个信号之后如果没有重写 SIGHUP 信号的 handler 或者也没有忽略这个信号，那么就会执行这个信号的默认行为，也就是退出程序的执行。

事实上当你退出终端之后 shell 不仅给前台进程组的所有进程发送 SIGHUP 信号，而且也会给所有的后台进程组发送 SIGHUP 信号，因此当你退出终端之后你启动的所有后台进程都会收到一个 SIGHUP 信号，注意 shell 是给所有的后台进程组发送的信号，因此如果你的后台进程是一个多进程的程序的话，那么你这个多进程程序的每一个进程都会收到这个信号。

根据上面的分析我们就可以知道了当我们退出终端之后，shell 会给后台进程发送一个 SIGHUP 信号。在我们了解了 shell 的行为之后我们应该可以理解为什么我么需要 nohup 命令，因为我们正常的程序是没有处理这个 SIGHUP 信号的，因此当我们退出终端之后所有的后台进程都会收到这个信号，然后终止执行。

看到这里你应该能够理解 nohup 命令的原理和作用了，这个命令的作用就是让程序忽略 SIGHUP 这个信号，我们可以通过 nohup 的源代码看出这一点。

nohup 的核心代码如下所示：

```c
int
main(int argc, char *argv[])
{
	int exit_status;

	while (getopt(argc, argv, "") != -1)
		usage();
	argc -= optind;
	argv += optind;
	if (argc < 1)
		usage();

	if (isatty(STDOUT_FILENO))
		dofile();
	if (isatty(STDERR_FILENO) && dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
		/* may have just closed stderr */
		err(EXIT_MISC, "%s", argv[0]);

	(void)signal(SIGHUP, SIG_IGN); // 在这里忽略 SIGHUP 这个信号

	execvp(*argv, argv); // 执行我们在命令行当中指定的程序
	exit_status = (errno == ENOENT) ? EXIT_NOTFOUND : EXIT_NOEXEC;
	err(exit_status, "%s", argv[0]);
}
```

在上面的程序当中我们可以看到，在 main 函数当中，nohup 首先创建使用 signal 忽略了 SIGHUP 信号，SIG_IGN 就是忽略这个信号，然后使用 execvp 执行我们在命令行当中指定的程序。

这里需要注意一点的是关于 execvp 函数，也就是 execve 这一类系统调用，只有当我们使用 SIG_IGN 忽略信号的时候，才会在 execvp 系列函数当中起作用，如果是我们自己定义的信号处理器 (handler)，那么在我们执行完 execvp 这个系统调用之后，所有的我们自己定义的信号处理器的行为都将失效，所有被重新用新的函数定义的信号都会恢复成信号的默认行为。

比如说下面这个程序：

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

void sig(int no)
{
  char* s = "Hello World\n";
  write(STDOUT_FILENO, s, strlen(s));
  sync();
}

int main(int argc, char* argv[], char* argvp[])
{

  signal(SIGINT, sig);
  execvp(argv[1], argv);
}
```

在上面的程序当中我们定义了一个信号处理器 sig 函数，如果接受到 SIGINT 信号那么就会执行 sig 函数，但是我们前面说了，因为只有 SIG_IGN 才能在 execvp 函数执行之后保持，如果是自定函数的话，那么这个信号的行为就会被重置成默认行为，SIGINT 的默认行为是退出程序，现在我们使用上面的程序去加载执行一个死循环的程序，执行结果如下：

![6](../../images/linux/shell/6.png)

从上面的程序的输出结果我们就可以知道，在我们按下 ctrl + c 之后进程会收到一个来自内核的 SIGINT 信号，但是并没有执行我们设置的函数 sig ，因此验证了我们在上文当中谈到的结论！

有心的同学可能会发现当我们在终端使用 nohup 命令的时候会生成一个 "nohup.out" 文件，记录我们的程序的输出内容，我们可以在 nohup 的源代码当中发现一点蛛丝马迹，我们可以看一下 nohup 命令的完整源代码：

```c
#if 0
#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)nohup.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("FreeBSD");

#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void dofile(void);
static void usage(void);

#define	FILENAME	"nohup.out" // 定义输出文件的文件名
/*
 * POSIX mandates that we exit with:
 * 126 - If the utility was found, but failed to execute.
 * 127 - If any other error occurred. 
 */
#define	EXIT_NOEXEC	126
#define	EXIT_NOTFOUND	127
#define	EXIT_MISC	127

int
main(int argc, char *argv[])
{
	int exit_status;

	while (getopt(argc, argv, "") != -1)
		usage();
	argc -= optind;
	argv += optind;
	if (argc < 1)
		usage();

	if (isatty(STDOUT_FILENO))
		dofile();
	if (isatty(STDERR_FILENO) && dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
		/* may have just closed stderr */
		err(EXIT_MISC, "%s", argv[0]);

	(void)signal(SIGHUP, SIG_IGN);

	execvp(*argv, argv);
	exit_status = (errno == ENOENT) ? EXIT_NOTFOUND : EXIT_NOEXEC;
	err(exit_status, "%s", argv[0]);
}

static void
dofile(void)
{
	int fd;
	char path[MAXPATHLEN];
	const char *p;

	/*
	 * POSIX mandates if the standard output is a terminal, the standard
	 * output is appended to nohup.out in the working directory.  Failing
	 * that, it will be appended to nohup.out in the directory obtained
	 * from the HOME environment variable.  If file creation is required,
	 * the mode_t is set to S_IRUSR | S_IWUSR.
	 */
	p = FILENAME;
  // 在这里打开 nohup.out 文件
	fd = open(p, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (fd != -1)
    // 如果文件打开成功直接进行文件描述符的替代，将标准输出重定向到文件 nohup.out 
		goto dupit;
	if ((p = getenv("HOME")) != NULL && *p != '\0' &&
	    (size_t)snprintf(path, sizeof(path), "%s/%s", p, FILENAME) <
	    sizeof(path)) {
		fd = open(p = path, O_RDWR | O_CREAT | O_APPEND,
		    S_IRUSR | S_IWUSR);
		if (fd != -1)
			goto dupit;
	}
	errx(EXIT_MISC, "can't open a nohup.out file");

dupit:
	if (dup2(fd, STDOUT_FILENO) == -1)
		err(EXIT_MISC, NULL);
	(void)fprintf(stderr, "appending output to %s\n", p);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: nohup [--] utility [arguments]\n");
	exit(EXIT_MISC);
}
```

在源代码当中的宏 FILENAME 定义的文件名就是 nohup.out，在上面的代码当中，如果判断当前进程的标准输出是一个终端设备就会打开文件 nohup.out 然后将进程的标准输出重定向到文件 nohup.out ，因此我们在程序当中使用 printf 的输出就都会被重定向到文件 nohup.out 当中，看到这里就破案了，原来如此。

## 后台进程和终端的纠缠

后台进程是不能够从终端读取内容的，当我们从终端当中读的时候内核就会给这个后台进程发送一个 SIGTTIN 信号，这个条件主要是避免多个不同的进程都读终端。如果后台进程从终端当中进行读，那么这个进程就会收到一个 SIGTTIN 信号，这个信号的默认行为就是退出程序。

我们可以使用下面的程序进程测试：

```c

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>


void sig(int no, siginfo_t* si, void* ucontext)
{
  char s[1024];
  sprintf(s, "signal number = %d sending pid = %d\n", no, si->si_pid);
  write(STDOUT_FILENO, s, strlen(s));
  sync();
  _exit(0);
}

int main()
{
  struct sigaction action;
  action.sa_flags |= SA_SIGINFO;
  action.sa_sigaction = sig;
  action.sa_flags &= ~(SA_RESETHAND);
  sigaction(SIGTTIN, &action, NULL);
  while(1)
  {
    char c = getchar();
  }
  return 0;
}
```

然后我们在终端输入命令，并且对应的输出如下：

```shell
➜  daemon git:(master) ✗ ./job11.out&
[1] 47688
signal number = 21 sending pid = 0                                                                                
[1]  + 47688 done       ./job11.out
```

从上面程序的输出结果我们可以知道，当我们在程序当中使用函数 getchar 读入字符的时候，程序就会收到来自内核的信号 SIGTTIN，根据下面的信号名和编号表可以知道，内核发送的信号位 SIGTTIN。

![6](../../images/linux/shell/8.png)

当我们在终端当中进行写操作的时候会收到信号 SIGTTOU，但是默认后台进程是可以往终端当中写的，如果我们想要进程不能够往终端当中写，当进程往终端当中写数据的时候就收到信号 SIGTTOU，我们可以使用命令 stty 进行设置。我们使用一个例子看看具体的情况：

```c

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>


void sig(int no, siginfo_t* si, void* ucontext)
{
  char s[1024];
  sprintf(s, "signal number = %d sending pid = %d\n", no, si->si_pid);
  write(STDOUT_FILENO, s, strlen(s));
  sync();
  _exit(0);
}

int main()
{
  struct sigaction action;
  action.sa_flags |= SA_SIGINFO;
  action.sa_sigaction = sig;
  action.sa_flags &= ~(SA_RESETHAND);
  sigaction(SIGTTOU, &action, NULL);
  while(1)
  {
    sleep(1);
    printf("c");
    fflush(stdout);
  }
  return 0;
}
```

上面是一个比较简单的信号程序，不断的往终端当中输出字符 `c`，我们可以看一下程序的执行情况（job12 就是上面的代码）：

```shell
➜  daemon git:(master) ✗ stty tostop 
➜  daemon git:(master) ✗ ./job12.out&
[1] 48467
➜  daemon git:(master) ✗ signal number = 22 sending pid = 0

[1]  + 48467 done       ./job12.out
```

在上面的输出结果当中我们使用命令 `stty tostop` 主要是用于启动当有后台进程往终端当中写内容的时候，向这个进程发送 SIGTTOU 信号，这个信号的默认行为也是终止进程的执行。

首先看一下当我们没有使用 `stty tostop` 命令的时候程序的行为。

![jobcontrol](../../vedio/job3.gif)

现在我们使用 `stty tostop` 命令重新设置一下终端的属性，然后重新进程测试：

![jobcontrol](../../vedio/job2.gif)

从上面的输出结果我们可以看到当我们在终端当中，默认是允许进程往终端当中进行输出的，但是当我们使用命令 `stty tostop` 之后，如果还有后台进程往终端当中进行输出，那么这个进程就会收到一个 SIGTTOU 信号。

## 后台进程和终端的命令交互

在前文当中我们谈到了当我们在一条命令后面加上 & 的话，那么这个程序将会变成后台进程。那么有没有办法将一个后台进程变成前台进程呢？

当然有办法，我们可以使用 fg ——一个 shell 的内置命令，将一个后台进程变成前台进程。在正式进行验证之前我们需要来了解三个命令：

- jobs 这条命令主要是用于查看当前所有的后台进程组，也就是所有的后台作业,。
- fg  这条命令主要是将一个后台进程放到前台来运行。
- bg  这条命令主要是让一个终端的后台程序继续执行。

具体的例子如下所示：

```shell
➜  daemon git:(master) ✗ sleep 110 & # 创建一个后台进程 每当创建一个后台作业 shell 都会给这个作业分配一个作业号 就是 [] 当中的数字，从 1 开始
[1] 7467
➜  daemon git:(master) ✗ sleep 111 & # 创建一个后台进程
[2] 7485
➜  daemon git:(master) ✗ sleep 112 & # 创建一个后台即成
[3] 7503
➜  daemon git:(master) ✗ jobs # 查看所有的后台进程 其中 + 表示当前作业 可以认为是最近一次使用 & 生成的作业 - 表示上一个作业 可以认为是倒数第二个使用 & 生成的作业
[1]    running    sleep 110
[2]  - running    sleep 111
[3]  + running    sleep 112
➜  daemon git:(master) ✗ fg # fg 的使用方式为 fg %num 如果不指定 %num 的话，默认就是将当前作业放到前台 饿我们在上面已经谈到了 当前作业为 sleep 112 因此将这个进程恢复到前台
[3]  - 7503 running    sleep 112
^C # 终止这个作业
➜  daemon git:(master) ✗ jobs  # 因为终止了作业 sleep 112 因此后台进程组只剩下两个了
[1]    running    sleep 110
[2]  + running    sleep 111
➜  daemon git:(master) ✗ fg  # 在将最近一次提交的作业放到前台
[2]  - 7485 running    sleep 111
^C # 终止这个任务的执行
➜  daemon git:(master) ✗ sleep 112 &
[2] 7760
➜  daemon git:(master) ✗ jobs 
[1]  - running    sleep 110
[2]  + running    sleep 112
➜  daemon git:(master) ✗ sleep 112 &
[3] 7870
➜  daemon git:(master) ✗ sleep 112 &
[4] 7888
➜  daemon git:(master) ✗ jobs 
[1]    running    sleep 110
[2]    running    sleep 112
[3]  - running    sleep 112
[4]  + running    sleep 112
➜  daemon git:(master) ✗ fg %1 
[1]    7467 running    sleep 110
^C
➜  daemon git:(master) ✗ 
```

接下来我们使用下面的程序进行验证，下面的程序的主要目的就是判断当前进程是否是前台进程，如果是则打印消息，如果不是那么就一直进行死循环：

```c

#include <stdio.h>
#include <unistd.h>

int main()
{
  while(1)
  {
    sleep(1);
    // tcgetpgrp 返回前台进程组的进程组号
    // getpgid(0) 得到当前进程组的进程组号
    // 如果两个结果相等则说明当前进程组是前台进程组
    // 反之则是后台进程组
    if(getpgid(0) == tcgetpgrp(STDOUT_FILENO))
    {
      printf("I am a process of foregroup process\n");
    }
  }
  return 0;
}
```

然后我们在终端当中执行这个程序，对应的几个结果如下所示：

```shell
➜  daemon git:(master) ✗ ./job13.out& # 先将这个程序放到后台运行，因为不是前台程序因此不会打印消息
[1] 5832
➜  daemon git:(master) ✗ fg    # 将这个程序放到前台执行，因为到了前台因此上面的程序会输出消息
[1]  + 5832 running    ./job13.out
I am a process of foregroup process
I am a process of foregroup process
I am a process of foregroup process
^Z
[1]  + 5832 suspended  ./job13.out # 在这里我们按下 ctrl + z 给进程发送 SIGTSTP 信号 让进程暂停执行
➜  daemon git:(master) ✗ bg %1 # bg 命令默认是给进程发送一个 SIGCONT 因为在上一行当中信号 SIGTSTP 让进程暂停执行了 因此进程在收到信号 SIGCONT 之后会继续执行（SIGCONT 的作用就是让一个暂停的进程继续执行）
[1]  + 5832 continued  ./job13.out # 因为进程还是在后台当中，因此进程继续执行还是在后台执行，所以依然没有输出
➜  daemon git:(master) ✗ fg %1 # 这条命令是让后台进程组当中的第一个作业到前台执行，因此进程开始打印输出
[1]  + 5832 running    ./job13.out
I am a process of foregroup process
I am a process of foregroup process
^C # 在这里输入 ctrl + c 命令，让前台进程组当中所有进程停止执行
➜  daemon git:(master) ✗ 
```

在上面的输出结果当中，我们首先在后台启动一个进程，因为是在后台所以当前进程组不是前台进程组，因此不会在终端当中打印输出，而当我们使用 fg 命令将后台当中的最近生成的一个作业（当我们输入命令之后，终端打印的[]当中的数字就是表示作业号，默认是从 1 开始的，因为我们只启动一个后台进程（执行一条命令就是开启一个作业），因此作业号等于 1）放到前台来执行，在上面的例子当中，命令 fg 和 fg %1 的效果是一样的。

## 总结

在本篇文章当中主要给大家介绍了后台进程的一些生与死的情况，总体来说有以下内容：

- 当我们退出终端的时候，shell 会给所有前台和后台进程组发送一个 SIGHUP 信号，nohup 命令的原理就是让程序忽略这个 SIGHUP 信号。
- 当后台进程从终端当中读的时候内核会给这个进程发送一个 SIGTTIN 信号。
- 当我们设置了 ssty tostop 之后，如果我们往终端当中进行写操作的话，那么内核会给这个进程发送一个 SIGTTOU 信号，这两个信号的默认行为都是终止这个进程的执行。
- 我们可以使用 jobs fg bg 命令让终端和后台进程进行交互操作，fg 将一个后台进程放到前台执行，如果这个进行暂停执行的话， shell 还会给这个进程发送一个 SIGCONT 信号让这个进程继续执行，bg 可以让一个后台暂停执行的进程恢复执行，本质也是给这个后台进程发送一个 SIGCONT 信号。
- 当你在终端当中输入 ctrl + c 的时候，内核会给所有的前台进程组当中所有的进程发送 SIGINT 信号，当你在终端输入 ctrl + z 时，内核会给前台进程组当中的所有进程发送 SIGTSTP 信号，当你在终端输入 ctrl + \ 内核会给所有的前台进程组发送 SIGQUIT 信号。
- 综合上面的分析，上面的结果可以使用下面的图进行表示分析。

![6](../../images/linux/shell/7.png)



---

以上就是本篇文章的所有内容了，我是**LeHung**，我们下期再见！！！更多精彩内容合集可访问项目：<https://github.com/Chang-LeHung/CSCore>

关注公众号：**一无是处的研究僧**，了解更多计算机（Java、Python、计算机系统基础、算法与数据结构）知识。

![](https://img2022.cnblogs.com/blog/2519003/202207/2519003-20220703200459566-1837431658.jpg)

