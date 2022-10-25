# 如何优雅的杀掉一个进程

## 前言

在我们通常使用linux操作系统的时候，经常会有这样的需求——杀死一个进程，比如说你一不小心启动了一个后台进程或者守护进程，而这个进程是你不需要的，因此你久想杀掉他，在本篇文章当中主要给大家介绍一些杀死进程的方法，以及这隐藏在这后面的原理。

## 你可以杀死哪些进程

在我们杀死一个进程的时候最好不要使用管理员权限，因为你可能会一不小心杀死系统当中一些很重要的进程。同时需要了解，在linux当中有很多与权限相关的操作，如果你只是一个普通的用户，那么你就只能杀死你自己的进程，不能够杀死别的用户的进程。但是root用户或者你有sudo权限，那么你就可以为所欲为了😂。

杀死进程的基本原理：我们使用命令去杀死进程，本质上是通过一个进程（比如说kill命令的那个进程）给另外一个进程发送信号，当进程接收到信号的时候就会进行判断是哪个信号，然后根据不同的信号做出相应的行为。

在linux操作系统当中，常见的信号如下所示，信号前面表示代表不同信号的数值，比如说我们执行命令 `kill -9 1234` 就是将 9 这个值对应的信号 SIGKILL 发送给进程号等于 1234 的进程：

```shell
 1) SIGHUP	 2) SIGINT	 3) SIGQUIT	 4) SIGILL	 5) SIGTRAP
 6) SIGABRT	 7) SIGBUS	 8) SIGFPE	 9) SIGKILL	10) SIGUSR1
11) SIGSEGV	12) SIGUSR2	13) SIGPIPE	14) SIGALRM	15) SIGTERM
16) SIGSTKFLT	17) SIGCHLD	18) SIGCONT	19) SIGSTOP	20) SIGTSTP
21) SIGTTIN	22) SIGTTOU	23) SIGURG	24) SIGXCPU	25) SIGXFSZ
26) SIGVTALRM	27) SIGPROF	28) SIGWINCH	29) SIGIO	30) SIGPWR
31) SIGSYS	34) SIGRTMIN	35) SIGRTMIN+1	36) SIGRTMIN+2	37) SIGRTMIN+3
38) SIGRTMIN+4	39) SIGRTMIN+5	40) SIGRTMIN+6	41) SIGRTMIN+7	42) SIGRTMIN+8
43) SIGRTMIN+9	44) SIGRTMIN+10	45) SIGRTMIN+11	46) SIGRTMIN+12	47) SIGRTMIN+13
48) SIGRTMIN+14	49) SIGRTMIN+15	50) SIGRTMAX-14	51) SIGRTMAX-13	52) SIGRTMAX-12
53) SIGRTMAX-11	54) SIGRTMAX-10	55) SIGRTMAX-9	56) SIGRTMAX-8	57) SIGRTMAX-7
58) SIGRTMAX-6	59) SIGRTMAX-5	60) SIGRTMAX-4	61) SIGRTMAX-3	62) SIGRTMAX-2
63) SIGRTMAX-1	64) SIGRTMAX
```

对于信号来说，进程可以有不同的应对行为，具体来说主要有以下三种：

- 忽略这个信号。
- 使用默认行为去处理这个信号，比如SIGINT和SIGTERM这两个信号的默认行为就是退出程序。
- 自己定义函数捕获这个信号，我们可以自己写一个函数，并且使用系统调用将这个函数进行注册，当收到对应的信号的时候就去执行我们自己实现的函数，但是需要注意的是，并不是所有的信号我们都可以进行捕获的，比如说SIGKILL和SIGSTOP这两个信号。

## 程序的定位

我们通常可以使用 ps 和 top 两个命令进行程序的定位，在前面的两篇文章[Linux命令系列之top——里面藏着很多鲜为人知的宝藏知识](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247486949&idx=1&sn=428516fe22182f41de4dfed075b03e7a&chksm=cf0c91ecf87b18fad120db8c34c0a22e748ed403991803d114e7e505dbcc6c5d53cff2bbddb0&token=102890258&lang=zh_CN#rd)和[这才是使用ps命令的正确姿势](https://mp.weixin.qq.com/s?__biz=Mzg3ODgyNDgwNg==&mid=2247487127&idx=1&sn=5486fcdac3ea1c251b4e0bc63bf110be&chksm=cf0c929ef87b1b88e4f76559a1b2c271438e89d8e5dbe9aef4cecf8c6f27eb8d695224aa48a1&token=985838262&lang=zh_CN#rd)当中我么已经仔细讨论过这个问题了！因此当我们想要杀死某个程序的时候我们可以通过上述两个命令进行程序的定位。

## 使用kill命令杀死进程

kill命令的使用方法如下所示：

```shell
kill [option] <pid> [...] # [option] 是参数选项比如 -9 pid 表示进程的进程id
```

发送一个 SIGINT 信号给进程 1234

```shell
kill -SIGINT 1234
或者
kill -2 1234
```

如果进程 1234 执行SIGINT的默认行为的话，那么进程1234就会退出，因为默认行为就是退出程序。

强制杀死进程 1234

```shell
kill -SIGKILL 1234
或者
kill -9 1234
```

因为信号 SIGKILL 是不能够被忽略或者捕获的，这个就是强制杀死程序，这条命令可以保证一定杀死进程，但是我们一般情况下最好不要使用这条命令，因为很多程序有他自己的逻辑，比如清理一些数据和系统资源，但是如果你不关心这些就无所谓了。

杀死所有你有权限杀死的进程

```shell
kill -9 -1
```

上面的命令当中 -1 的意思表示将 -9 这个信号发送给所有你有权限发送的进程，这个命令慎用。





## 为什么我们不能够捕获所有的信号

在前面的文章当中我们提到了，SIGKILL和SIGSTOP这两个信号是不能够被捕获的！试想一下，在你的系统的当中有一个病毒程序，他会不断的创建新的进程并且不断的申请系统资源，那么你还没有办法杀死他，你只能够眼睁睁的看着你的系统卡死。这种问题 linux 设计者早就想到了，基于这个问题就肯定需要有一种方式能够万无一失的杀掉进程，因此才出现了不能够被捕获的信号。

