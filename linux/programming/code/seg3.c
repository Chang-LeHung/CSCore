
#include <stdio.h>

int main() {

  int arr[10];
  arr[1 << 20] = 100; // 会导致 segmentation fault
  printf("arr[12] = %d\n", arr[1 << 20]); // 会导致 segmentation fault
  return 0;
}