#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

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

#define MAX 50

int
main(int argc, char *argv[])
{
  int fd, i;
  char buf[MAX];
  char buf2[MAX];
  char buf3[MAX];

  for(i = 0; i < MAX; i++){
    buf[i] = (char)(i+(int)'0');
  }
  
  for(i = 0; i < MAX; i++){
    buf2[i] = (char)(i-(int)'0');
  }
  
  
  //create
  if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE)) < 0){
    printf(1, "Failed to create a small file\n");
    test_failed();
  }
  close(fd);
  
  
  //write
  if((fd = open("test_file.txt", O_RDWR)) < 0){
    printf(1, "Failed to open a small file\n");
    test_failed();
  }
 
  if(write(fd, buf, MAX) != MAX){
    printf(1, "Write failed!\n");
    test_failed();
  }
  close(fd);
  
  
  //write with O_RDONLY flag
  if((fd = open("test_file.txt", O_RDONLY)) < 0){
    printf(1, "Failed to open a small file as read only\n");
    test_failed();
  }
  
  if(write(fd, buf2, MAX) >= 0){ //tries to overwrite data with new data
    printf(1, "Write succeeded despite O_RDONLY flag!\n");
    test_failed();
  }
  close(fd);

  
  //read
  if((fd = open("test_file.txt", O_RDONLY)) < 0){
    printf(1, "Failed to open a small file as read only\n");
    test_failed();
  }
  
  if(read(fd, buf3, MAX) != MAX){
    printf(1, "Read failed!\n");
    test_failed();
  }
  close(fd);
  
  for(i = 0; i < MAX; i++){
    if(buf[i] != buf3[i]){
      printf(1, "Data mismatch, possibly because second write succeeded despite O_RDONLY flag\n");
      test_failed();
    }
  }
  
  test_passed();
	exit();
}
