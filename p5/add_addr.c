int 
add_addr(struct dinode* din, uint * usedAddr){
  //Direct 
  int i;
  uint addr = 0;
  for(i = 0; i < NDIRECT; i++){
    if(din->addrs[i] != 0){// && din->type!=1){
      if(usedAddr[addr] > 1)
      {
	printf("address: %u used more than once\n", addr);
	perror("address used more than once\n");
	return -1;
      }
      usedAddr[din->addrs[i]]++;
    }
  }
    //Indirect
    addr = din->addrs[NDIRECT];
    if(addr!= 0){ 
      usedAddr[addr]++; //
      uint* baddr  = (uint*)(img_ptr+(addr*BSIZE));
      //printf("baddr is %u\n",*baddr);
      while(*baddr!=0){
	// printf("baddr is %u\n",*baddr);
	if(usedAddr[baddr] > 1)
	{
	  printf("address: %u used more than once\n", addr);
	  perror("address used more than once\n");
	  return -1;
	}
	usedAddr[*baddr]++; //
	baddr++;
      }
    }
  }
