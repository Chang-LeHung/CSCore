
import os
import sys

if __name__ == '__main__':
    pid = os.fork()
    if pid == 0:
        sys.exit(-1)
    else:
        pid, status = os.wait()
        print(os.WEXITSTATUS(status))
