#include <stdio.h>

void __attribute__((destructor)) __exit() {
  printf("this is exit\n");
}

void __attribute__((constructor)) init() {
  printf("this is init\n");
}


int main() {
  printf("this is main\n");
  return 0;
}