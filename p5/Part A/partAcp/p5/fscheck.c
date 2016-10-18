#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define stat xv6_stat  // avoid clash with host struct stat
#define dirent xv6_dirent  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
//#define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)

#include "stat.h"
#undef stat
#undef dirent


uint size;
uint ninodes;
uint nblocks; 
int fsfd;

  uint
i2b(uint inum) //inodes per block
{
  return (inum / IPB) + 2;
}

  void
rsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * 512L, 0) != sec * 512L){
    perror("lseek");
    exit(1);
  }
  if(read(fsfd, buf, 512) != 512){
    perror("read");
    exit(1);
  }
}

  void
rinode(uint inum, struct dinode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct dinode *dip;

  bn = i2b(inum);
  rsect(2, buf);
  dip = ((struct dinode*)buf) + (inum % IPB);
  *ip = *dip;
}

int 
check_addr(struct dinode* din)
{
  uint addr;
  //direct addr check
  addr = din->addrs[NDIRECT-1];
  // printf("addr is %d\n",addr);
  //  printf("ninodes/IPB+4 is %d, size- nblocks is %lu\n",(ninodes/IPB+4),(size- nblocks));
  if((addr < (size- nblocks)||(addr > nblocks) ))
    {
      //  perror("bad address in inode\n");
      // exit(0);
      printf("exit when checking directm\n");
      return -1;
    }

  //indrect addr check
  uint buf[BSIZE];
  addr = din->addrs[NDIRECT];
  
  rsect(addr,buf);
  
  uint num_block = 0;
  while(buf[num_block]!= 0||(num_block < BSIZE/4)){
    if(buf[num_block] < ninodes/IPB+4 || buf[num_block] > nblocks){
      if(buf[num_block]==0){
	return 0;
      }
      printf("buf[%d]is %u",num_block,buf[num_block]);
      printf("exit when checking indirect\n");
      return -1;
    }
    num_block++;
  }
  return 0;
}
  int
main(int argc, char *argv[])
{
  char buf[BSIZE]; //512 for the size of a block?

  if(argc < 2)
  {
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  } 

  fsfd = open(argv[1], O_RDONLY);
  if (fsfd < 0)
  {
    perror("image not found\n");
    exit(1);
  }
  //read the super block
  rsect(1, buf);
  ninodes = ((struct superblock*)buf)->ninodes;
  nblocks = ((struct superblock*)buf)->nblocks;
  size = ((struct superblock*)buf)->size;

    printf("The size is: %d, there are %d inodes and %d blocks\n", size, ninodes, nblocks);

  //  void *img_ptr = mmap(NULL,sbuf.st_size,PROT_READ,MAP_PRIVATE,fsfd,0);
  rsect(2, buf);
  int rc;
  struct stat sbuf;
  rc = fstat(fsfd,&sbuf);
    assert(rc ==0);

  void *img_ptr = mmap(NULL,sbuf.st_size,PROT_READ,MAP_PRIVATE,fsfd,0);
  assert(img_ptr!= MAP_FAILED);
  struct dinode* din  = (struct dinode*)(img_ptr+(2*BSIZE));
  int i;
  for(i = 0; i < ninodes; i++ ){
    if(i > 1)
      {
	if (din->addrs[NDIRECT] != 0 ){
	   
    
	//	if((din->type != T_FILE )|| (din->type != T_DIR )|| (din->type != T_DEV)||(din->type !=4))
	if(din->type >4 || din->type < 0)
       {
	 printf("will exit, the type is %d\n",din->type);
        perror("bad inode\n");
        exit(0);
       }
     }
    
    //error #2
    
	if ((din->addrs[NDIRECT]!= 0 )&&( check_addr(din)!=0))
      {
	//check to see if in use
       perror("bad address in inode\n");
	exit(0);
      }


    if ((i == 1) && (din->type != 1))
      {
	perror("root directory does not exist\n");
	exit(0);
      }

    if (din->type == 1)
      {
    	char buffer[BSIZE];
    	rsect(din->addrs[NDIRECT],buffer);
	char *iname = (char*)malloc(DIRSIZ*sizeof(char));
	iname = &((struct xv6_dirent*)buffer)->name[0];
    	//	name = strdup((struct dirent*)buffer)->name);
	printf("name is:%s***\n",iname);

      }

     }
    printf("for inode %d, type is: %u, size is %d\n",i,din->type,din->size);
    din++;
  }
  /*Use mmap()
   * int rc;
   * rc = stat(fd,&sbuf);
   * assert(rc ==0);
   * void *img_ptr = mmap(NULL,sbuf.st_size,PROT_READ,MAP_PRIVATE),fd,0;
   * assert(img_ptr!= MAP_FAILED);
   *struct superblock *sb;
   sb =(struct superblock*)(img_ptr + BSIZE);
   printf("%d %d %d\n",sb->size,sb->nblocks, sb->ninodes);
   * */
  //no errors - exit with 1




  return 0;

}
