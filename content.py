
import os
import sys

def find_all_dirs(path):
  r = []
  for dir in os.listdir(path):
    dir = os.path.join(path, dir)
    if dir[2:].startswith("."):
      continue
    if os.path.isdir(dir):
      r.append(dir)
      r += find_all_dirs(dir)
  return r

if __name__ == "__main__":
  for d in find_all_dirs("./"):
    print(d + "/*" + sys.argv[1])

