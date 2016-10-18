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
  
  char *filename = "test_file.txt";
  if((fd = open(filename, O_SMALLFILE | O_RDWR)) >= 0){
    printf(1, "Created small file even though O_CREATE was never passed in.\n");
    test_failed();
  }
  
  test_passed();
  exit();
}
