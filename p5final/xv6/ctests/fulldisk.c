#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

void
test_failed()
{
	printf(1, "TEST FAILED\n");
	exit();
}

void
test_passed()
{
 printf(1, "TEST PASSED\n");
 exit();
}

#define MAX 500
#define SMALL_SIZE 25

int
main(int argc, char *argv[])
{
  int fd;
  char buf[MAX];
  int n;
  int i;
  
  for(i = 0; i < MAX; i++){
    buf[i] = 'a';
  }
  
  char name[2] = { '0' , '\0'};
  
  int count = 0;
  while(1) {
    printf(1, "creating large file: %s\n", name);
    if((fd = open(name, O_CREATE | O_RDWR)) < 0){
      printf(1, "Failed to create a large file\n");
      test_failed();
    }
    
    int write_failed = 0;
    for(i = 0; i < MAXFILE; i++){
      n = write(fd, buf, MAX);
      if(n < MAX){
        write_failed = 1;
        break;
      }
      count++;
    }
    close(fd);
    
    if(write_failed == 1){
      break;
    }
    
    name[0]++;
  }
  
  printf(1, "all free blocks are used\n");
  
  if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
    printf(1, "Failed to create the small file\n");
    test_failed();
  }
  
  if((n = write(fd, buf, SMALL_SIZE)) != SMALL_SIZE){
    printf(1, "Write to small file failed!\n");
    test_failed();
  }
  printf(1, "bytes written = %d\n", n);
  close(fd);
  
  test_passed();
  exit();
}
