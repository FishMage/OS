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
  struct stat st;
  char buf[MAX];
  char buf2[MAX];

  for(i = 0; i < MAX; i++){
    buf[i] = (char)(i+(int)'0');
  }
  
  
  //create
  if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE)) < 0){
    printf(1, "Failed to create a small file\n");
    test_failed();
  }
  close(fd);
  
  
  //type check and write
  if((fd = open("test_file.txt", O_RDWR)) < 0){
    printf(1, "Failed to open a small file\n");
    test_failed();
  }
  
  if(fstat(fd, &st) < 0){
    printf(1, "Failed to get stat on the small file\n");
    test_failed();
  }
  
  if (st.type != T_SMALLFILE) {
    printf(1, "Did not create a small file\n");
    test_failed();
  }
 
  if(write(fd, buf, MAX) != MAX){
    printf(1, "Write failed!\n");
    test_failed();
  }
  close(fd);
 printf(1, "Write passed\n");
  
  //read
  if((fd = open("test_file.txt", O_RDWR)) < 0){
    printf(1, "Failed to open a small file\n");
    test_failed();
  }
  
  if(read(fd, buf2, MAX) != MAX){
    printf(1, "Read failed!\n");
    test_failed();
  }
  close(fd);
  
  for(i = 0; i < MAX; i++){
    if(buf[i] != buf2[i]){
      printf(1, "Data mismatch.\n");
      test_failed();
    }
  }
  
  test_passed();
	exit();
}
