#ifndef __CRAWLER_H
#define __CRAWLER_H
/*
struct node {
        char* page;
        struct node *next;
};
*/

//crawler member function
void * downloader(void*param);
void * parser(void *param);

//linkedlist interface
void push(char * p); //return true if successfully add the node
char *pop();	
	
int crawl(char *start_url,
	  int download_workers,
	  int parse_workers,
	  int queue_size,
	  char * (*fetch_fn)(char *url),
	  void (*edge_fn)(char *from, char *to));

#endif
