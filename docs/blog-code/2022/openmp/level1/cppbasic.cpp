
#include <thread>
#include <iostream>

void* func() {

  printf("hello world from %ld\n", std::this_thread::get_id());
  return 0;
}

int main() {

  std::thread threads[4];
  for(auto &t : threads) {
    t = std::thread(func);
  }
  for(auto &t : threads) {
    t.join();
  }

  return EXIT_SUCCESS;
}