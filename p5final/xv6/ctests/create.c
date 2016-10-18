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

int
main(int argc, char *argv[])
{
  int fd;
  struct stat st;
  
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
  
  if (st.type != T_SMALLFILE) {
    printf(1, "Did not create a small file\n");
    test_failed();
  }
  else {
    test_passed();
  }
  
	exit();
}
