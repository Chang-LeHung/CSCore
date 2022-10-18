
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void __attribute__((destructor)) __exit1() {
  printf("this is exit1\n");
}

void __attribute__((destructor)) __exit2() {
  printf("this is exit2\n");
}


void __attribute__((constructor)) init1() {
  printf("this is init1\n");
}


void __attribute__((constructor)) init2() {
  printf("this is init2\n");
}

void on__exit1() {
  printf("this in on exit1\n");
}

void at__exit1() {
  printf("this in at exit1\n");
}

void on__exit2() {
  printf("this in on exit2\n");
}

void at__exit2() {
  printf("this in at exit2\n");
}


int main() {
  // _exit(1);
  on_exit(on__exit1, NULL);
  on_exit(on__exit2, NULL);
  atexit(at__exit1);
  atexit(at__exit2);
  printf("this is main\n");
  _exit(1);
  return 0;
}