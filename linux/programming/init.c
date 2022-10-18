
#include <stdio.h>

void __attribute__((constructor)) init() {
  printf("before main funciton\n");
}

int main() {
  printf("this is main funciton\n");
}