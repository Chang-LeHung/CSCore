

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define COLOR_NORMAL "\033[0m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RED "\033[1;31m"
#define COLOR_GREY "\033[1;30m"

#define PRIV(mode, match, char) \
        {if(mode & match) printf(char); else printf("-");}

#define echotime(sec) \
    {char buffer[100];\
    struct tm *info = localtime(&sec);\
    strftime(buffer, 80, " %Y-%m-%d %H:%M:%S", info); printf("%s", buffer);}


void print_file_detail_info(char * filename) {

  struct stat buf;
  
  int ret = lstat(filename, &buf);
  if (ret == -1)
  {
    perror(strerror(errno));
    exit(-1);
  }
  // match device type
  switch (buf.st_mode & S_IFMT)
  {
  case S_IFLNK:
    printf("l");
    break;
  case S_IFBLK:
    printf("b");
    break;
  case S_IFREG:
    printf("-");
    break;
  case S_IFDIR:
    printf("d");
    break;
  case S_IFSOCK:
    printf("s");
    break;
  case S_IFCHR:
    printf("c");
    break;
  case S_IFIFO:
    printf("p");
  }
  // find priviledge
  PRIV(buf.st_mode, S_IREAD, "r")
  PRIV(buf.st_mode, S_IWUSR, "w")
  PRIV(buf.st_mode, S_IEXEC, "x")

  PRIV(buf.st_mode, S_IRGRP, "r")
  PRIV(buf.st_mode, S_IWGRP, "w")
  PRIV(buf.st_mode, S_IXGRP, "x")

  PRIV(buf.st_mode, S_IROTH, "r")
  PRIV(buf.st_mode, S_IWOTH, "w")
  PRIV(buf.st_mode, S_IXOTH, "x")

  struct passwd* p = getpwuid(buf.st_uid);
  struct group* grp = getgrgid(buf.st_gid);

  printf(" %-3ld %-5s %-5s %5ld", buf.st_nlink, p->pw_name, grp->gr_name, buf.st_size);
  echotime(buf.st_ctim.tv_sec)
  printf(" %s", filename);
  if (S_ISLNK(buf.st_mode))
  {
    char buf[1024];
    size_t s = readlink(filename, buf, 1024);
    buf[s] = '\0';
    printf(COLOR_GREEN " -> %s" COLOR_NORMAL, buf);
  }
  
  printf("\n");

}

int main(int argc, char* argv[]) {

  if(argc != 2) {
    printf(COLOR_RED "no dir sepcified default show current dir files\n" COLOR_NORMAL);
    argv[1] = malloc(10);
    sprintf(argv[1], "./");
  }

  DIR* dir = opendir(argv[1]);
  if (dir == NULL)
  {
    perror(strerror(errno));
    exit(-1);
  }
  
  struct dirent* dirs;
  while ((dirs = readdir(dir)) != NULL)
  {
    char* filename = malloc(1024);
    // printf("%s %c %ld\n", dirs->d_name, dirs->d_type, dirs->d_ino);
    strcpy(filename, argv[1]);
    strcat(filename, dirs->d_name);

    // printf("filename = %s\n", filename);
    print_file_detail_info(filename);
  }
  return 0;
}

