
import os

if __name__ == "__main__":
    print(f"parent pid = {os.getpid()}")
    pid = os.fork()
    if pid != 0:
        # parent process will never exit
        while True:
            pass
    # child process will exit
    print(f"child process pid = {os.getpid()}")
