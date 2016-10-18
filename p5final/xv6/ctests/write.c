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

#define MAX 55

int
main(int argc, char *argv[])
{
  int fd;
  char buf[MAX];
  char buf2[MAX];
  int n;
  int i;
  
  for(i = 0; i < MAX; i++){
    buf[i] = (char)(i+(int)'0');
  }
  
  if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
    printf(1, "Failed to create the small file\n");
    test_failed();
    exit();
  }
  
  if((n = write(fd, buf, MAX)) < 0){
    printf(1, "Write failed!\n");
    test_failed();
  }
  printf(1, "bytes written = %d\n", n);
  close(fd);
  
  if(n != (NDIRECT+1)*4){
    printf(1, "Failed to write the right amount to the small file.\n");
    test_failed();
  }
  
  
  if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
    printf(1, "Failed to create the small file\n");
    exit();
  }
  
  if((n = read(fd, buf2, MAX)) < 0){
    printf(1, "Read failed!\n");
    exit();
  }
  printf(1, "bytes read = %d\n", n);
  close(fd);
  
  if(n != (NDIRECT+1)*4){
    printf(1, "Failed to read the right amount to the small file.\n");
    test_failed();
  }
  
  for(i = 0; i < (NDIRECT+1)*4; i++){
    if(buf[i] != buf2[i]){
      printf(1, "Data mismatch.\n");
      test_failed();
    }
  }
  
  test_passed();
  exit();
}
