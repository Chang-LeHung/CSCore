
import os
import time


if __name__ == "__main__":
    pid = os.fork()

    if pid == 0:
        while True:
            print(f"pid = {os.getpid()} parent pid = {os.getppid()}")
            time.sleep(1)


