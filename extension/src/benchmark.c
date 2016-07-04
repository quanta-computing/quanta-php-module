#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include "computed_functions_filter.c"

char *get_functions(void) {
  char *content;
  int fd;
  struct stat sb;

  fd = open("/Users/Kraoz/Downloads/suce.log", O_RDONLY);
  fstat(fd, &sb);
  content = malloc(sb.st_size);
  read(fd, content, sb.st_size);
  return content;
}

int main(int argc, char const *argv[]) {
  char *functions = get_functions();
  char *function, *saveptr;
  struct timeval start;
  struct timeval end;
  int count = 0;

  gettimeofday(&start, NULL);
  for (function = strtok_r(functions, "\n", &saveptr)
  ; function; function = strtok_r(NULL, "\n", &saveptr)) {
    ++count;
    hp_match_monitored_function(function);
  }
  gettimeofday(&end, NULL);
  printf("%d count\n", count);
  printf("%ldmis\n", (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
  return 0;
}
