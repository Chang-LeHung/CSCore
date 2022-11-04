

#include <stdio.h>
#include <omp.h>


int main() {

  #pragma omp parallel num_threads(4)

  
  {
    printf("hello world from tid = %d\n", omp_get_thread_num());
  }


  return 0;
}



