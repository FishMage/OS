#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "crawler.h"

void *Malloc(size_t size) {
  void *r = malloc(size);
  assert(r);
  return r;
}

char *Strdup(const char *s) {
  void *r = strdup(s);
  assert(r);
  return r;
}

char *fetch(char *link) {
  int fd = open(link, O_RDONLY);
  if (fd < 0) {
    perror("failed to open file");
    return NULL;
  }
  int size = lseek(fd, 0, SEEK_END);
  assert(size >= 0);
  char *buf = Malloc(size+1);
  buf[size] = '\0';
  assert(buf);
  lseek(fd, 0, SEEK_SET);
  char *pos = buf;
  while(pos < buf+size) {
    int rv = read(fd, pos, buf+size-pos);
    assert(rv > 0);
    pos += rv;
  }
  close(fd);
  return buf;
}

void edge(char *from, char *to) {
  printf("%s -> %s\n", from, to);
}

int main(int argc, char *argv[]) {
  char * test_url = "<a href=\"http://pages.cs.wisc.edu/~remzi/Classes/537/Spring2016/\">Fun, Free Projects!</a>";
  int rc = crawl(test_url, 1, 1, 1, fetch, edge);
  assert(rc == 0);
  return 0;
}
