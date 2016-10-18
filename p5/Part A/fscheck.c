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

void* img_ptr;
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
inodeBitmap_check(uint addr)
{
  //check the bitmap
  //printf("BBLOCK returned %lu\n", BBLOCK(addr,ninodes));
  char* bp = (char*)(img_ptr+BBLOCK(addr,ninodes)*BSIZE);
  int bi ;
  bi = addr % BPB;
  char m = 1 << (bi % 8);
  char* data = bp + (addr/8);
  if((*data & m) == 0){
    // printf("it's free\n");
    return 0; // 0 for free block
  }else{
//    printf("addr %u is allocated\n",addr);
    return 1;	//1 for allocated block
  }
}

int
bitmapInode_check(){
  return 0;
}

  int 
check_addr(struct dinode* din)
{
  uint addr;
  //direct addr check
  int i;
  for(i = 0; i < NDIRECT-1; i++){
    addr = din->addrs[i];
    // printf("addr is %d\n",addr);
    // printf("ninodes/IPB+4 is %lu, size- nblocks is %u\n",(ninodes/IPB+4),(size- nblocks));
    if((addr < (size- nblocks)||(addr > nblocks) ))
    {
      //  perror("bad address in inode\n");
      // exit(0);
      printf("exit when checking direct\n");
      return -1;
    }

    else
    {  
      if(inodeBitmap_check(addr)==0){
	perror("address used by inode but marked free in bitmap\n");
      }
      /* //check the bitmap */
      /* //printf("BBLOCK returned %lu\n", BBLOCK(addr,ninodes)); */
      /* //struct dinode* din  = (struct dinode*)(img_ptr+(2*BSIZE)); */
      /* char* p = (char*)(img_ptr+BBLOCK(addr,ninodes)*BSIZE); */
      /* uint offset = 8 - addr%8; */
      /* uint num_byte = addr/8 +1; */
      /* uint ctr = 0; */
      /* char* cmp = (0x01<<offset); */
      /* while(ctr!=num_byte) */
      /*   { */
      /*     ctr++; */
      /*     p++; */
      /*   } */
      /* if((cmp & p)==cmp ){ */
      /*   printf("inode is allocated in the bitmap"); */
      /* } */
      /*   else */
      /*     { */
      /*       perror("address used by inode but marked free in bitmap\n"); */
      /*       exit(0); */
      /*     }	   */
    }
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
parent_match(ushort pinum, ushort cinum){
  int i,j;
//  printf("pinum:%u  cinum:%u\n",pinum,cinum);
  char buffer[BSIZE];
  struct dinode* din  = (struct dinode*)(img_ptr+(2*BSIZE));
  for(i = 0; i < ninodes; i++){
    if(i == pinum){//searching parent dir
    //  printf("in parent dir, inode num: %d\n",i);
      for(j = 0; j < NDIRECT; j++)
      {
	rsect(din->addrs[j],buffer);
	struct xv6_dirent *dir = (struct xv6_dirent*)buffer;
	while((char*)dir < buffer+BSIZE){
	 // printf("dir->inum:%u, cinum:%u\n",dir->inum, cinum);
	  if(dir->inum!=0&&dir->inum == cinum){
//	    printf("find match parent\n");
	    return 0;
	  }
	  dir++;
	}
      }
    }
    din++;
  }
  return -1;
}


int
dirent_format(struct dinode* din, int num_i){
  int ctr = 0;
  char buffer[BSIZE];
  int j;
  for(j = 0; j < NDIRECT; j++)
  {
    rsect(din->addrs[j],buffer);
    //char *iname = (char*)malloc(DIRSIZ*sizeof(char));
    //iname = &((struct xv6_dirent*)buffer)->name[0];
    //name = strdup((struct dirent*)buffer)->name);
    struct xv6_dirent *dir = (struct xv6_dirent*)buffer;
    while((char*)dir < buffer+BSIZE){
      if(dir->inum!=0){
	//printf("addr is :%u\n", din-> addrs[j]);
	//(img_ptr+(din->addrs[j]*BSIZE));
	if(strcmp(dir->name,".")==0||strcmp(dir->name,"..")==0){
	  if(strcmp(dir->name,"..")==0){
	    if(num_i==1&&dir->inum==1){
	   //   printf("in root dir, and inum is %d\n",dir->inum); 
	    }
	    if(num_i!=1 && parent_match(dir->inum,num_i) !=0){
	    //  perror("parent directory mismatch\n");
	      return -1;
	    }
	  //  printf(".. detected, inum is %u\n",dir->inum);
	  }
	  //	  if(strcmp(dir->name,".")==0){
	  //	    printf(". detected, inum is %u\n",dir->inum);
	  //	  }
	  ctr++;
	}
	printf("name is:%s ",dir->name);
	printf("inum is:%u\n",dir->inum);
      }
      dir++;
    }
  }
  if(ctr!=2){
    printf("ctr is %d\n",ctr);
    perror("directory not properly formatted\n");
    return -1;// formatting error
  }
  //parent error
//  printf("ctr is %d\n",ctr);
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
  
  img_ptr = mmap(NULL,sbuf.st_size,PROT_READ,MAP_PRIVATE,fsfd,0);
  assert(img_ptr!= MAP_FAILED);
  struct dinode* din  = (struct dinode*)(img_ptr+(2*BSIZE));
  uint usedAddr[ninodes*NDIRECT];
  int i,addrArray_ptr, check_ptr;
  for(i = 0; i < ninodes; i++ ){
    printf("for inode %d, type is: %u, size is %d, addr is %u\n",i,din->type,din->size,din->addrs[0]);
    if(i >= 1)
    {
      addrArray_ptr = 0;
      //check repeated addr
      if((din->addrs[0]) != 0){
	int j;
	//add the addrs[] in to the used addr array
      	if(din->type !=1){
	  check_ptr = 0;
	for(j = 0; j< NDIRECT; j++){
//	  printf("din->addrs[%d]:%u\n",j,din->addrs[j]);
	  //check
	  while(check_ptr < addrArray_ptr){
//	    printf("check_ptr:%d, arrayptr:%d\n",check_ptr,addrArray_ptr);
	    if(usedAddr[addrArray_ptr] == din->addrs[j])
	    {
	      //printf("address: %u used more than once\n", din->addrs[j]);
	      perror("address used more than once\n");
	      exit(0);
	      }
	    }
	    check_ptr++;
	  //add
	  usedAddr[addrArray_ptr] = din->addrs[j];
	  addrArray_ptr++;
	  }
	  }
	}


      if (din->addrs[NDIRECT] != 0 ){ 

	//if((din->type != T_FILE )|| (din->type != T_DIR )|| (din->type != T_DEV)||(din->type !=4))
	if(din->type >4 || din->type < 0)
	{
	  printf("will exit, the type is %d\n",din->type);
	  perror("bad inode\n");
	  exit(0);
	}
      }

      //error #2
      //inodes address check (within the range)
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
      //check consistency of directories
      if (din->type == 1)
      {
	if(dirent_format(din,i)==-1){
	  exit(0);
	}
      }



      if(din->type==0)
	exit(0);
    }
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
