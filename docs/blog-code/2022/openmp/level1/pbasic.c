

#include <stdio.h>
#include <pthread.h>

void* func(void* args) {
  printf("hello world from tid = %ld\n", pthread_self());
  return NULL;
}

int main() {
  pthread_t threads[4];
  for(int i = 0; i < 4; i++) {
    pthread_create(&threads[i], NULL, func, NULL);
  }
  for(int i = 0; i < 4; i++) {
    pthread_join(threads[i], NULL);
  }
  return 0;
}
