#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include "computed_functions_filter.c"

int count_lines(char *str) {
  int count = 0;
  while (*str) {
    if (*str == '\n')
      ++count;
    ++str;
  }
  return count;
}

struct method {
  char *function_name;
  char *class_name;
};

struct method *get_methods(void) {
  struct stat sb;
  char *content;
  struct method *lines;
  struct method *ret;
  int fd;

  fd = open("/Users/Kraoz/Downloads/suce-1.log", O_RDONLY);
  fstat(fd, &sb);
  content = malloc(sb.st_size);
  read(fd, content, sb.st_size);
  ret = lines = malloc((count_lines(content) + 1) * sizeof(*lines));
  lines->function_name = content;
  while (*content) {
    if (*content == ' ') {
      *content = '\0';
      lines->class_name = content + 1;
    } else if (*content == '\n') {
      *content = '\0';
      ++lines;
      lines->function_name = content + 1;
    }
    ++content;
  }
  lines->function_name = NULL;
  return ret;
}

#define INDEX_2_BYTE(index)  (index >> 3)
#define INDEX_2_BIT(index)   (1 << (index & 0x7));
#define QUANTA_MON_MAX_MONITORED_FUNCTIONS_HASH  256
#define QUANTA_MON_MAX_MONITORED_FUNCTIONS  23
#define QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE ((QUANTA_MON_MAX_MONITORED_FUNCTIONS_HASH + 7)/8)

typedef unsigned long ulong;

inline uint8_t hp_inline_hash(char * str) {
  ulong h = 5381;
  uint i = 0;
  uint8_t res = 0;

  while (*str)
    h = (h + (h << 5)) ^ (ulong)*str++;

  for (i = 0; i < sizeof(ulong); i++)
    res += ((uint8_t *)&h)[i];
  return res;
}

void hp_monitored_functions_filter_init(char **functions, uint8_t *monitored_function_filter) {
  for(int i = 0; functions[i] != NULL; i++) {
    if (!*functions[i]) continue;
    char *str  = strstr(functions[i], "::");
    uint8_t hash = hp_inline_hash(str ? str + 2 : functions[i]);
    int idx = INDEX_2_BYTE(hash);
    monitored_function_filter[idx] |= INDEX_2_BIT(hash);
  }
}

int hp_monitored_functions_filter_collision(uint8_t hash, uint8_t *monitored_function_filter) {
  uint8_t mask = INDEX_2_BIT(hash);
  return monitored_function_filter[INDEX_2_BYTE(hash)] & mask;
}

int main(int argc, char const *argv[]) {
  uint8_t monitored_function_filter[QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE];
  char *monitored_functions[QUANTA_MON_MAX_MONITORED_FUNCTIONS];
  struct method *methods = get_methods();
  struct method *methods_save = methods;
  struct timeval start, end;
  int count = 0;

  bzero(monitored_function_filter, QUANTA_MON_MONITORED_FUNCTION_FILTER_SIZE);
  hp_fill_monitored_functions(monitored_functions);
  hp_monitored_functions_filter_init(monitored_functions, monitored_function_filter);
  gettimeofday(&start, NULL);
  while (methods->function_name) {
    // printf("Matching %s::%s\n", methods->function_name, methods->class_name);
    if (hp_match_monitored_function(methods->function_name, methods->class_name) != -1)
      ++count;
    ++methods;
  }
  gettimeofday(&end, NULL);
  printf("%d count, %fms\n", count, ((end.tv_sec -
    start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000.0);
  methods = methods_save;
  count = 0;
  gettimeofday(&start, NULL);
  while (methods->function_name) {
    if (hp_monitored_functions_filter_collision(
    hp_inline_hash(methods->function_name), monitored_function_filter))
      ++count;
    ++methods;
  }
  gettimeofday(&end, NULL);
  printf("%d count, %fms\n", count, ((end.tv_sec -
    start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000.0);
  return 0;
}
