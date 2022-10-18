
#include <stdio.h>
#include <stdlib.h>

void __attribute__((destructor)) __exit() {
  printf("this is exit\n");
}

void __attribute__((constructor)) init() {
  printf("this is init\n");
}

void on__exit() {
  printf("this in on exit\n");
}

void at__exit() {
  printf("this in at exit\n");
}

int main() {
  on_exit(on__exit, NULL);
  atexit(at__exit);
  printf("this is main\n");
  return 0;
}