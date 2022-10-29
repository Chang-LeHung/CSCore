# openmp 入门

## 简介

Openmp 一个非常易用的共享内存的并行编程框架，它提供了一些非常简单易用的API，让编程人员从复杂的并发编程当中释放出来，专注于具体功能的实现。openmp 主要是通过编译指导语句以及他的动态运行时库实现，在本篇文章当中我们主要介绍 openmp 一些入门的简单指令的使用。

## 认识 openmp 的简单性

### C 语言实现

```c


#include <stdio.h>
#include <pthread.h>

void* func(void* args) {
  printf("hello world from tid = %ld\n", pthread_self());
  return NULL;
}

int main() {
  pthread_t threads[4];
  for(int i = 0; i < 4; i++) {
    pthread_create(&threads[i], NULL, func, NULL);
  }
  for(int i = 0; i < 4; i++) {
    pthread_join(threads[i], NULL);
  }
  return 0;
}
```

### C++ 实现

```cpp

#include <thread>
#include <iostream>

void* func() {

  printf("hello world from %ld\n", std::this_thread::get_id());
  return 0;
}

int main() {

  std::thread threads[4];
  for(auto &t : threads) {
    t = std::thread(func);
  }
  for(auto &t : threads) {
    t.join();
  }

  return EXIT_SUCCESS;
}
```

### Openmp 实现

```c

#include <stdio.h>
#include <omp.h>


int main() {

  #pragma omp parallel num_threads(4)
  {
    printf("hello world from tid = %d\n", omp_get_thread_num());
  }
  return 0;
}
```

