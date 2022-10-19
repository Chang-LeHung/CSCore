
#include <stdio.h>

void stakc_overflow(int times) {
  printf("times = %d\n", times);
  char data[1 << 20]; // 每次申请 1 Mega 数据
  stakc_overflow(++times);
}

int main() {

  stakc_overflow(1);
  return 0;
}
