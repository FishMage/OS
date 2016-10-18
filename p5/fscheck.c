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
int  checkptr;
int arraysz;
int curr_size;
uint curr_type;

  uint
i2b(uint inum) //inodes per block
{
  return (inum / IPB) + 2;
}

  void
rsect(uint sec, void *buf)
{
  if(lseek(fsfd, sec * 512L, 0) != sec * 512L){
    printf("lseek");
    exit(1);
  }
  if(read(fsfd, buf, 512) != 512){
    printf("read");
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
mrkfree(uint addr)
{
  //check the bitmap
  //printf("BBLOCK returned %lu\n", BBLOCK(addr,ninodes));
  char* bp = (char*)(img_ptr+BBLOCK(addr,ninodes)*BSIZE);
  int bi;
  bi = addr % BPB;
  char m = 1 << (bi % 8);
  char* data = bp + (addr/8);
  if((*data & m) == 0){
    // printf("it's free\n");
    return 0; // 0 for free block
  }else
  {
    //    printf("addr %u is allocated\n",addr);
    return 1;	//1 for allocated block
  }
}

int
existed(uint* array, uint value){
  int i;
  for(i = 0; i < arraysz; i++){
    if(array[i] == value)
      return value;
  }
  return 0;
}

int
bitmapInode_check(uint* usedAddr){
  char* bp = (char*)(img_ptr+ ((ninodes)/IPB + 3)*BSIZE);
  uint addr =0; 
  uint inuse = 0;
  char m = 1;
  char* data = bp ;
  int i,j;
  for(i = 0; i < BSIZE; i++){
    m = 1;
    for(j = 0; j < 8; j++){
      if((*data & m) != 0){// allocated
	inuse = 0; 
	//check
	addr = i*8 + j;
	//value = existed(usedAddr,addr);
	if(usedAddr[addr] >0)
	  inuse =1; 
	if(addr >= ninodes/IPB +4){
	  if(inuse==0){ //not in use
	    //	    printf("data is: %d\n",*data);
	    //	    printf("bitmap marks block %u in use but it is not in use\n",addr);
	    return -1;
	  }
	  //printf("addr %u is mark in use \n",addr);
	  //shift
	}
      }
      m = m*2;
    }
    data += 1;
  }
  return 0;
}

int 
parent_match(ushort pinum, ushort cinum){
  int i,j;
  char buffer[BSIZE];
  //  printf("pinum:%u  cinum:%u\n",pinum,cinum);
  if(pinum ==1&& cinum ==1)
    return 0;
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

	    printf("   match: %d->%d\n",pinum,cinum);
	    //	    printf("find match parent\n");
	    return 0;
	  }
	  dir++;
	}
      }
      //Indirect
      // rsect(din->addrs[NDIRECT],buffer);
      uint addr = din->addrs[NDIRECT];
      uint* baddr  = (uint*)(img_ptr+(addr*BSIZE));
      while(*baddr != 0){ //if addr in data block is not 0
	rsect(*baddr,buffer);
	struct xv6_dirent * ind_dir = (struct xv6_dirent*)buffer;
	while((char*)ind_dir < buffer+BSIZE){
	  if(ind_dir->inum!=0 && ind_dir->inum == cinum){
	    printf("   match: %d->%d\n",pinum,cinum);
	    return 0;
	  }
	}
	baddr++;
      }
    }
    din++;
  }
  return -1;
}
// dirent_read scan dir entries for a single dir
// check the format
// add the entry to the inodeArr for reference count checking
// return the inode number of parent
// otherwise return 0
int
dirent_read(struct xv6_dirent *dir, int num_i, uint* inodeArr, uint * parents,int ctr){
  uint parent_i = 0;
  struct xv6_dirent * start_pos = dir;
  while((char*)dir < (char*)start_pos+BSIZE){
    if(dir->inum ==0)
      break;
    if(dir->inum!=0){
      if(strcmp(dir->name,".")==0||strcmp(dir->name,"..")==0){
	if(strcmp(dir->name,".")==0 && num_i ==1){
//	  printf("in . inode %d++\n",dir->inum);
	  inodeArr[1]++;
	}
	if(strcmp(dir->name,"..") == 0){
	  if(num_i ==1 &&  dir->inum!=1){
	    fprintf(stderr,"ERROR: root directory does not exist.\n");
	    exit(1);
	    }
	  parent_i = dir->inum;
	  if(parent_i !=0){
	    printf("testing mismatch: %d->%d  ",parent_i,num_i);
	    if(parent_match(parent_i,num_i)!=0){
	      fprintf(stderr,"ERROR: parent directory mismatch\n");
	      exit(1);
	    } 
	  }

	}
	//	  if(parents[dir->inum] > 1){
	//	      printf("directory appears more than one in file system\n");
	//	      return -1;
	//	    }
	//	  }
	ctr++; // check "." & ".." formatting 
    }
    else{	//for entires other than "." & ".."
//           printf("%d ++\n",dir->inum);
      inodeArr[dir->inum]++;
    }
    //  printf("dir->inum:%u, dir->name %s\n",dir->inum,dir->name);
  }	
  dir++;
}

return ctr;
}

int
dirent_check(struct dinode* din, int num_i, uint* inodeArr, uint* parents){
  int ctr = 0;// check "." & ".." formatting 
  char buffer[BSIZE];
  int j;
  //  int parent_i = 0;
  uint addr;
  for(j = 0; j < NDIRECT; j++)// Direct part
  {
    if(din->addrs[j]!=0){
      rsect(din->addrs[j],buffer);
      //char *iname = (char*)malloc(DIRSIZ*sizeof(char));
      //iname = &((struct xv6_dirent*)buffer)->name[0];
      //name = strdup((struct dirent*)buffer)->name);
      struct xv6_dirent *dir = (struct xv6_dirent*)buffer;
      ctr = dirent_read(dir,num_i,inodeArr, parents,ctr);
      //printf("ctr is %d after %d iteration\n",ctr, j);
    }
    //check parent mismatch
  }
  //  parents[dir->inum]++;
  //  while((char*)dir < buffer+BSIZE){
  //    //      printf("inum: %u, type: %u", dir->inum, di)
  //    //	printf("dir addr is %u   %s\n", dir->inum, dir->name);
  //    if(dir->inum!=0){
  //      //printf("addr is :%u\n", din-> addrs[j]);
  //      //(img_ptr+(din->addrs[j]*BSIZE));
  //      if(strcmp(dir->name,".")==0||strcmp(dir->name,"..")==0){
  //        if(strcmp(dir->name,"..") == 0){
  //          if(num_i==1&&dir->inum!=1){
  //            //   printf("in root dir, and inum is %d\n",dir->inum); 
  //            printf("parent directory mismatch\n");
  //          }
  //          if(num_i!=1 && parent_match(dir->inum,num_i) !=0){
  //            printf("parent directory mismatch\n");
  //            return -1;
  //          }
  //          if(parents[dir->inum] > 1){
  //            printf("directory appears more than one in file system\n");
  //            return -1;
  //          }
  //          //  printf(".. detected, inum is %u\n",dir->inum);
  //        }
  //        ctr++;
  //      }
  //      else{ 
  //        inodeArr[dir->inum]++;
  //        printf("inum : %u marked %u\n",dir->inum,inodeArr[dir->inum]);
  //      }
  //      printf("name is:%s ",dir->name);
  //      printf("inum is:%u\n",dir->inum);
  //    }
  //    dir++;
  // }
  // }
  //  }
  //Indirect part
addr = din->addrs[NDIRECT];
//printf("Reading indirect directories addr is %u\n",addr);
if(addr!=0){
  uint* ind_addr  = (uint*)(img_ptr+(addr*BSIZE));
  if(*ind_addr!=0){
    rsect(*ind_addr,buffer);
    struct xv6_dirent *ind_dir = (struct xv6_dirent*)buffer;
    ctr = dirent_read(ind_dir,num_i,inodeArr, parents,ctr);
  }      
}
//      printf("in indirect part of dirent_format(), ind_addr is%u\n",*ind_addr);
//      rsect(*ind_addr,buffer);
//      struct xv6_dirent *ind_dir = (struct xv6_dirent*)buffer;
//      while( (char*)ind_dir < buffer+BSIZE){
//	//
//	if(ind_dir->inum != 0){
//	  if(strcmp(ind_dir->name,".")==0||strcmp(ind_dir->name,"..")==0){
//	    if(strcmp(ind_dir->name,"..") == 0){
//	      if(num_i==1&&ind_dir->inum!=1){
//		//   printf("in root dir, and inum is %d\n",dir->inum); 
//		printf("parent directory mismatch\n");
//	      }
//	      if(num_i!=1 && parent_match(ind_dir->inum,num_i) !=0){
//		printf("parent directory mismatch\n");
//		return -1;
//	      }
//	      if(parents[ind_dir->inum] > 1){
//		printf("directory appears more than one in file system\n");
//		return -1;
//	      }
//	      //  printf(".. detected, inum is %u\n",dir->inum);
//	    }
//	  }
//	  else{ 
//	    inodeArr[ind_dir->inum]++;
//	    printf("inum : %u marked %u\n",ind_dir->inum,inodeArr[ind_dir->inum]);
//	  }
//	  printf("name is:%s ",ind_dir->name);
//	  printf("inum is:%u\n",ind_dir->inum);
//	}
//	ind_dir++;
//      }
//    }
//  }
//

//  if(ctr!=2){
//    printf("ctr is %d\n",ctr);
//    printf("directory not properly formatted\n");
//    return -1;// formatting error
//  }
//parent error
//  printf("ctr is %d\n",ctr);

if(ctr!=2){
  //  printf("ctr is %d\n",ctr);
  fprintf(stderr,"ERROR: directory not properly formatted.\n");
  exit(1);// formatting error
}
return 0;
}
// add all addr in a single inode into the addrs array which contains all addrs of address
int 
addr_check(uint addr){
  //  printf("addrs to be check : range is %d -- %d\n",size- nblocks,nblocks);
  if((addr < (size- nblocks)||(addr > nblocks) ))
  {
    fprintf(stderr,"ERROR: bad address in inode.\n");
    exit(1);
  }
  if(mrkfree(addr)==0){
    if(curr_type ==1){
      //printf("addr :%u ",addr);
      fprintf(stderr,"ERROR: inode referred to in directory but marked free.\n");
    }else{
      fprintf(stderr,"ERROR: address used by inode but marked free in bitmap.\n");
      exit(1);
    }
  }
  return 0;
}


int 
add_addr(struct dinode * din,uint*  usedAddr){
  //Direct 
  //  printf("in add_add for node\n");
  int i;
  uint addr = 0;
  for(i = 0; i < NDIRECT; i++){
    //printf("adding %d th element\n",i);
    addr = din->addrs[i]; 
    //  printf("addr is %u\n",addr);
    if(addr!=0 &&addr_check(addr) == 0){
      if(usedAddr[addr] > 1)
      {
	//	printf("address: %u used more than once\n", addr);
	fprintf(stderr,"ERROR: address used more than once.\n");
	return -1;
      }
      usedAddr[addr]++;
    }
  }
  //Indirect
  addr = din->addrs[NDIRECT];
  if(addr!= 0){ 
    //  printf("Reading indirect addrs, start at block%u\n",addr);
    usedAddr[addr]++; //
    char buffer[BSIZE];
    rsect(addr,buffer);
    //   uint* baddr  = (uint*)(img_ptr+(addr*BSIZE));
    uint* baddr = (uint*)buffer;
    //printf("baddr is %u\n",*baddr);
    while((char*)baddr < buffer+BSIZE && *baddr!=0 && addr_check(*baddr)==0 ){
      // printf("baddr is %u\n",*baddr);
      if(usedAddr[*baddr] > 1)
      {
	//	printf("address: %u used more than once\n", addr);
	fprintf(stderr,"ERROR: address used more than once.\n");
	exit(1);
      }
      usedAddr[*baddr]++; //
      baddr++;
    }

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
    fprintf(stderr,"image not found.\n");
    exit(1);
  }
  //read the super block
  rsect(1, buf);
  ninodes = ((struct superblock*)buf)->ninodes;
  nblocks = ((struct superblock*)buf)->nblocks;
  size = ((struct superblock*)buf)->size;
  curr_size = 0;
  // printf("The size is: %d, there are %d inodes and %d blocks\n", size, ninodes, nblocks);

  //  void *img_ptr = mmap(NULL,sbuf.st_size,PROT_READ,MAP_PRIVATE,fsfd,0);
  rsect(2, buf);
  int rc;
  struct stat sbuf;
  rc = fstat(fsfd,&sbuf);
  assert(rc == 0);
  arraysz = nblocks; 
  uint* usedAddr = malloc(nblocks*sizeof(uint));
  uint* parents= malloc(nblocks*sizeof(uint));
  uint* inodeArr = malloc(ninodes*sizeof(uint)); //for dir reference count
  memset(usedAddr, 0, (nblocks*sizeof(uint)));
  memset(inodeArr, 0, (ninodes*sizeof(uint)));
  memset(parents, 0, (nblocks*sizeof(uint)));
  img_ptr = mmap(NULL,sbuf.st_size,PROT_READ,MAP_PRIVATE,fsfd,0);
  assert(img_ptr!= MAP_FAILED);
  struct dinode* din  = (struct dinode*)(img_ptr+(2*BSIZE));
  int i;
  for(i = 0; i < ninodes; i++ ){
    curr_type = din->type;
    if(i >= 1)
    {
      if(din->type == 0){
	break;
      }
      if(din->type == T_FILE|| din->type== T_DIR|| din->type == T_DEV){



//	printf("for inode %d, type is: %u, size is %d, addr is %u\n",i,din->type,din->size,din->addrs[0]);
	if(add_addr(din,usedAddr)!=0){
	  exit(1);
	}

	if (din->addrs[NDIRECT] != 0 ){ 

	  //if((din->type != T_FILE )|| (din->type != T_DIR )|| (din->type != T_DEV)||(din->type !=4))
	}

	if ((i == 1) && (din->type != T_DIR))
	{
	  fprintf(stderr,"ERROR: root directory does not exist.\n");
	  exit(1);
	}
	//check consistency of directories
	if (din->type == 1)
	{
	  if(dirent_check(din,i,inodeArr,parents)==-1){
	    exit(1);
	  }
	}
      }else{
	fprintf(stderr,"ERROR: bad inode.\n");
	exit(1);
      }
    }
    din++;
  }
  if(bitmapInode_check(usedAddr)!=0)
    exit(1);
  //Check the consistency of Directories
  din  = (struct dinode*)(img_ptr+(2*BSIZE));
  if(inodeArr[1] ==0){
    fprintf(stderr,"ERROR: root directory does not exist.\n");
    exit(1);
  }
  for(i = 0; i < ninodes; i++){
    if(i>=1){
      if(din->type == T_FILE||din->type == T_DIR||din->type == T_DEV){
	//printf("inode[i]:%u\n",inodeArr[i]);
	if(inodeArr[i] == 0){
	  //	printf("inode %d type:%d\n",i,din->type);
	  fprintf(stderr,"ERROR: inode %d marked use but not found in a directory.\n",i);
	  //continue;
	  exit(1);
	}
	if(din->nlink!=inodeArr[i]){
//	  printf("inode %d: %u != %u\n",i,din->nlink,inodeArr[i]);
	if(din->type ==1){
	  fprintf(stderr,"ERROR: directory appears more than once in file system.\n");
	  exit(1);
	  }
	  fprintf(stderr,"ERROR: bad reference count for file.\n");
	  exit(1);
	}
      }else{
	if(inodeArr[i] > 0){
//	  printf("inode %d  ",i);
	  fprintf(stderr,"ERROR: inode referred to in directory but marked free.\n");
	  exit(1);
	}
      }
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

 // printf("DONE\n");
  return 0;

}
