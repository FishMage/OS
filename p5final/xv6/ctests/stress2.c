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
#define NITERATIONS 100
#define NFILES 100

int
main(int argc, char *argv[])
{
  int fd, pid, i, j, k;
  
  char filename[2] = { '0', '\0' };
  char buf[SIZE];
  char buf2[SIZE];
  
  for(i = 0; i < NITERATIONS; i++){
    printf(1, "iteration %d\n", i);
    
    //create and write
    for(j = 0; j < NFILES; j++){
      //printf(1, "write file %d\n", j);
      if((fd = open(filename, O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
        printf(1, "Failed to create a small file\n");
        test_failed();
      }
      
      //writing unique data to each file
      uint *sector = (uint *)&buf;
      for(k = 0; k < NBLOCKS; k++, sector++){
        *sector = k*(j+1);
      }
      
      if(write(fd, buf, SIZE) != SIZE){
        printf(1, "Failed to write to small file\n");
        test_failed();
      }
      close(fd);
      filename[0]++;
    }
    
    filename[0] = '0'; //reset filename
    
    //read
    for(j = 0; j < NFILES; j++){
      //printf(1, "read file %d\n", j);
      if((fd = open(filename, O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
        printf(1, "Failed to open the small file\n");
        test_failed();
      }
      if(read(fd, buf2, SIZE) != SIZE){
        printf(1, "Read failed!\n");
        test_failed();
      }
      close(fd);
      
      //regenerate data that should be in file
      uint *sector = (uint *)&buf;
      for(k = 0; k < NBLOCKS; k++, sector++){
        *sector = k*(j+1);
      }
      
      //check data
      for(k = 0; k < SIZE; k++){
        if(buf[k] != buf2[k]){
          printf(1, "Data mismatch.\n");
          test_failed();
        }
      }
      
      filename[0]++;
    }
    
    filename[0] = '0'; //reset filename
    
    //remove
    for(j = 0; j < NFILES; j++){
      //printf(1, "remove file %d\n", j);
      
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
      
      filename[0]++;
    }
    
    filename[0] = '0'; //reset filename
  }
  
  test_passed();
  exit();
}