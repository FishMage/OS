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

#define NINODES 200
#define STRIDE 9

int
main(int argc, char *argv[])
{
  int fd, i, pid;
  struct stat st;
  int latest_inode;
  
  char filename[2] = {'0', '\0'};
  
  //creating a file to get the latest inode number
  if((fd = open("test_file.txt", O_CREATE | O_SMALLFILE)) < 0){
    printf(1, "Failed to create a small file\n");
    test_failed();
    exit();
  }
  if(fstat(fd, &st) < 0){
    close(fd);
    printf(1, "Failed to get stat on the small file\n");
    test_failed();
    exit();
  }
  close(fd);
  
  latest_inode = st.ino;
  printf(1, "latest_inode = %d\n", latest_inode);
  
  for(i = latest_inode+1; i < NINODES; i++){
    printf(1, "creating file at inode %d\n", i);
    if((fd = open(filename, O_CREATE | O_SMALLFILE)) < 0){
      printf(1, "Failed to create a small file\n");
      test_failed();
    }
    if(fstat(fd, &st) < 0){
      printf(1, "Failed to get stat on the small file\n");
      test_failed();
    }
    
    if(st.ino != i){
      printf(1, "ERROR! inodes were not allocated in order\n");
      test_failed();
    }
    
    close(fd);
    filename[0]++;
  }
  
  printf(1, "used up all inodes\n");
  
  filename[0] = '0'; //reset filename
  
  for(i = latest_inode+1+STRIDE; i < NINODES; i += STRIDE){
    printf(1, "deleting and recreating file at inode %d\n", i);
    filename[0] += STRIDE;
    
    //delete file
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
    
    //try to open file, but it should fail
    if((fd = open(filename, O_RDWR)) >= 0){
      printf(1, "Opened small file that should have been deleted\n");
      test_failed();
    }
    
    //create a small file again and make sure it allocates the only inode available (the inode that was just freed)
    if((fd = open(filename, O_CREATE | O_SMALLFILE | O_RDWR)) < 0){
      printf(1, "Failed to create a small file\n");
      test_failed();
    }
    
    if(fstat(fd, &st) < 0){
      printf(1, "Failed to get stat on the small file\n");
      test_failed();
    }
    
    if(st.ino != i){
      printf(1, "Failed to allocate small file at the right inode; allocated at inode %d, but inode %d should have been used\n", st.ino, i);
      test_failed();
    }
    
    close(fd);
  }

  test_passed();
	exit();
}
