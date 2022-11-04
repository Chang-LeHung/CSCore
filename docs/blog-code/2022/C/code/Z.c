
#include <stdio.h>
#include <unistd.h>


int main() {

  if(fork()) {
    while(1);
  }
  return 0;
}