
#include <stdio.h>
#include <omp.h>
#include <unistd.h>

int main() {

  #pragma omp parallel num_threads(4)
  {
    printf("parallel region 1 thread id = %d\n", omp_get_thread_num());
    sleep(1);
  }
  printf("after parallel region 1 thread id = %d\n", omp_get_thread_num());

  #pragma omp parallel num_threads(4)
  {
    printf("parallel region 2 thread id = %d\n", omp_get_thread_num());
    sleep(1);
  }
  printf("after parallel region 2 thread id = %d\n", omp_get_thread_num());

  #pragma omp parallel num_threads(4)
  {
    printf("parallel region 3 thread id = %d\n", omp_get_thread_num());
    sleep(1);
  }

  printf("after parallel region 3 thread id = %d\n", omp_get_thread_num());
  return 0;
}