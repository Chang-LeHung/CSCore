
#include <stdio.h>

char* str= "hello world";

int main() {
  printf("%s\n", str);
  *str = '1';
  return 0;
}