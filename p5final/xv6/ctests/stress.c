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

#define NBLOCKS (NDIRECT+1)
#define SIZE NBLOCKS*4
#define NITERATIONS 1000

int
main(int argc, char *argv[])
{
  int fd, pid, i, j;
  
  char *filename = "test_file.txt";
  char buf[SIZE];
  char buf2[SIZE];
  
  uint *sector = (uint *)&buf;
  for(i = 0; i < NBLOCKS; i++, sector++){
    *sector = i;
  }
  
  for(i = 0; i < NITERATIONS; i++){
    printf(1, "iteration %d\n", i);
    
    //create and write
    if((fd = open(filename, O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
      printf(1, "Failed to create a small file\n");
      test_failed();
    }
    if(write(fd, buf, SIZE) != SIZE){
      printf(1, "Failed to write to small file\n");
      test_failed();
    }
    close(fd);
    
    //read
    if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
      printf(1, "Failed to open the small file\n");
      test_failed();
    }
    if(read(fd, buf2, SIZE) != SIZE){
      printf(1, "Read failed!\n");
      test_failed();
    }
    close(fd);

    //check data
    for(j = 0; j < SIZE; j++){
      if(buf[j] != buf2[j]){
        printf(1, "Data mismatch.\n");
        test_failed();
      }
    }
    
    //remove small file
    pid = fork();
    if(pid < 0){
      printf(1, "Fork failed\n");
      test_failed();
    }
    else if(pid == 0) {
      char *args[3] = {"rm", filename, 0};
      exec(args[0], args);
      printf(1, "exec failed!\n");
      test_failed();
    }
    else {
      wait();
    }
  }
  
  test_passed();
  exit();
}