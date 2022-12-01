# 你在终端启动的进程，最后都是什么下场？（下）

在上期文章[你在终端启动的进程，最后都是什么下场？（上）](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487367&idx=1&sn=430b3afe8399ed32cf832d942b305a5d&chksm=cf0c938ef87b1a9803b2db922463612e5729579802d733ad6fd778d0c0d8439766b43af2801e&token=1948688554&lang=zh_CN#rd)当中我们介绍了前台进程最终结束的几种情况，在本篇文章当中主要给大家介绍后台进程号和孤儿进程可能被杀死的集中情况。

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

在上面的程序当中我们可以看到，在 main 函数当中，nohup 首先创建使用 signal 命令忽略了 SIGHUP 信号，SIG_IGN 就是忽略这个信号，然后使用 execvp 执行我们在命令行当中指定的程序。

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

