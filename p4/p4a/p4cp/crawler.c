#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#define NEW   0
#define ADDED 1
#define DONE  2
#define LINKLENGTH 256

typedef struct node {
  char* page;
  char*url;
  int status; //NEW-> ADDED -> DONE
  struct node *next;
}node_t;

char* (*fetch_ptr)(char* url);
void (*edge_ptr)(char *from, char *to);
int queue_sz = 0;
pthread_cond_t empty_bounded, empty_unbounded, empty_work;
pthread_cond_t fill_bounded, fill_unbounded;
pthread_mutex_t m, n, l, w;

//crawler member function
void * downloader(void*param);
void * parser(void *param);

//linkedlist interface
void push_visited(char* link); //return true if successfully add the node
void push_unbounded(char * page, char* link); //return true if successfully add the node
node_t *pop();
int check_status(char* data);
void change_status(char* curr_from, int status);
int check_exit();
void print_list_visited();
//bounded list
char** url_queue;
char* curr_from;

node_t* unbounded_queue = NULL;
node_t* visited_check = NULL;
int num_work = 0;
int num_page = 0;
int num_url = 0;
int work_done;
int queue_pop_ptr = 0;//points to current position of the bounded queue
int queue_push_ptr = 0;

int crawl(char *start_url,
    int download_workers,
    int parse_workers,
    int queue_size,
    char * (*_fetch_fn)(char *url),
    void (*_edge_fn)(char *from, char *to)) 
{
  if(queue_size > 0){
    queue_sz = queue_size;
  }else{
    return -1;
  }
  pthread_mutex_init(&m, NULL);
  pthread_mutex_init(&l, NULL);
  pthread_mutex_init(&n, NULL);
  pthread_mutex_init(&w, NULL);
  pthread_cond_init(&fill_bounded, NULL);
  pthread_cond_init(&fill_unbounded, NULL);
  pthread_cond_init(&empty_bounded, NULL);
  pthread_cond_init(&empty_work, NULL);

  url_queue = (char**)malloc(sizeof(char*)*queue_size);
  url_queue[0] = strdup(start_url); //is this right?
  num_url++;
  num_work = 1;
  queue_push_ptr = (queue_push_ptr+1)%queue_sz;
  fetch_ptr = _fetch_fn;
  edge_ptr = _edge_fn;

  //thread queue 
  pthread_t download_p [download_workers];
  pthread_t parse_p [parse_workers];

  int j,k;
  for(j = 0; j < download_workers;j++)
    pthread_create(&(download_p[j]),NULL,downloader,NULL);

  for(k = 0; k < parse_workers;k++)
    pthread_create(&(parse_p[k]),NULL,parser, NULL);

  pthread_mutex_lock(&w);
  while(num_work > 0){
    pthread_cond_wait(&empty_work,&w);
    //    printf("waiting for work_done\n");
  }
  pthread_mutex_unlock(&w);
  return 0;
}

//void print_list_visited(){
//  node_t* temp = visited_check;
//  if(temp==NULL){
//    return;
//  }
//  while(temp!= NULL){
//    printf("%s:%d-> ",temp->url,temp->status);
//    temp = temp->next;
//  }
//  printf("\n");
//  return;
//}
void push_visited(char* link){
  pthread_mutex_lock(&l);
  node_t* temp = visited_check ;
  if(visited_check==NULL)
  {
    visited_check= malloc(sizeof(node_t));
    visited_check->url =strdup( link);
    visited_check->page = NULL;
    visited_check->next = NULL;
    pthread_mutex_unlock(&l);
    return;
  }

  while(temp->next != NULL )
  {
    temp = temp->next;
  }
  temp->next = malloc(sizeof(node_t));
  temp->next->url =strdup( link);
  temp->next->page = NULL;
  temp->next->next = NULL;
  pthread_mutex_unlock(&l);
  return;
}
void push_unbounded( char* p, char* link){
  node_t* temp = unbounded_queue;
  //printf("in push and head: %s\n", temp->page);
  if(unbounded_queue==NULL)
  {
    unbounded_queue = malloc(sizeof(node_t));
    unbounded_queue ->page =strdup(p);
    unbounded_queue->url =strdup(link);
    // printf("page is%s\n",unbounded_queue->page);
    unbounded_queue->next = NULL;
    return;
  }

  while(temp->next != NULL )
  {
    temp = temp->next;
  }
  temp->next = malloc(sizeof(node_t));
  temp->next->page =strdup(p);
  temp->next->url = strdup(link);
  temp->next->next = NULL;
  return;
}

node_t *pop(){
  node_t* temp = unbounded_queue;
  if(temp == NULL){
    // printf("temp is NULL\n");
    return NULL;
  }
  node_t* next_node = unbounded_queue->next;
  unbounded_queue = next_node;
  // printf("[in pop()] the data of the node is: ");
  // printf("%s\n", temp->url);
  return temp; 
}

int check_status(char* link){
  pthread_mutex_lock(&l);
  node_t* temp = visited_check;
  if(temp == NULL){
    pthread_mutex_unlock(&l);
    return NEW;
  }//link not in the queue
  while(temp!=NULL)
  {
    if(temp->url!= NULL&&strcmp(temp->url,link)==0)
    {
      pthread_mutex_unlock(&l);
      return temp->status; 
    }
    temp = temp->next;
  }
  pthread_mutex_unlock(&l);
  return NEW;
}

void change_status(char* curr_from, int status)
{
  pthread_mutex_lock(&l);
  node_t* temp  = visited_check;
  if(temp == NULL){
    pthread_mutex_unlock(&l);
    return;
  }//NEW;//link not in the queue
  while(temp!=NULL)
  {
    if(temp->url!= NULL&&strcmp(temp->url,curr_from)==0)
    {
      temp->status = status;
      pthread_mutex_unlock(&l);
      return;
    }
    temp = temp->next;
  }
  pthread_mutex_unlock(&l);
  return;
}
int check_exit(){
  pthread_mutex_lock(&l);
  node_t* temp  = visited_check;
  if(temp == NULL){
    pthread_mutex_unlock(&l);
    return 0;
  }
  if(num_url!=0)
    pthread_mutex_unlock(&l);
  return 0;
  while(temp!=NULL)
  {
    if(temp->status != DONE){
      pthread_mutex_unlock(&l);
      return 0;
    }
    temp = temp->next;
  }
  pthread_mutex_unlock(&l);
  return 1;
}
//Downloader
void * downloader(void * arg)
{
  char* page;
  char* link;

  while(1)
  {
    //print_list_visited();
    pthread_mutex_lock(&m);//lock m for bounded queue
    while (num_url == 0){
 //     printf("waiting in downloader, num_url:%d\n",num_url);
      pthread_cond_wait(&fill_bounded,&m);
    }
    link = strdup(url_queue[queue_pop_ptr]);
    queue_pop_ptr = (queue_pop_ptr+1)%queue_sz;
    num_url--;
    pthread_cond_signal(&empty_bounded);
    pthread_mutex_unlock(&m);
    page = fetch_ptr(link);
    assert(page!=NULL);
    /*end of dequeue*/
    /*remove from queue,push into unbounded queue, signal parser*/
    //  pthread_mutex_lock(&l);//lock l for visited_check(linked list)
    //if link is already added, don't push
   // if(check_status(link)== ADDED){
   //   fprintf(stderr,"%s is already ADDED to the visited_check!\n",link);
   //   //      pthread_mutex_unlock(&l);
   //   continue;
   // }else if(check_status(link)==DONE){
   //  // if((check_exit()==1)){
   //  //   //        pthread_mutex_unlock(&l);
   //  //   	printf("time to exit\n");
   //  //   pthread_mutex_lock(&w);
   //  //   work_done = 1;
   //  //   pthread_cond_signal(&empty_work);
   //  //   pthread_mutex_unlock(&w);
   //  // }
   //   continue;
   // }else
    
    
   if(check_status(link)==NEW) {
 //     fprintf(stderr,"%s is NEW add to the visited_check queue\n",link);
      push_visited(link);
      //fprintf(stderr,"after push visited_check is:");
  //    fprintf(stderr,"%s is ADDED to the visited_check!\n",link);
      change_status(link,ADDED);
    }
    //   pthread_mutex_unlock(&l);

    pthread_mutex_lock(&n);//Lock n for unbounded queue
    push_unbounded(page,link); 
    num_page++;
    pthread_cond_signal(&fill_unbounded);
    pthread_mutex_unlock(&n);
  }
  return NULL;
}

//Parser
void * parser(void *param)
{
  char *saveptr1 =NULL;
  char *saveptr2 = NULL;
  char *tok = NULL;
  char *tok1 = NULL;
  char* ptr = NULL;
  node_t* unbounded_node = NULL;
  while(1)
  {
    pthread_mutex_lock(&n);
    //printf("in parser while loop\n");
    //    unbounded_node = pop();
    //    if(unbounded_node!=NULL){
    //      ptr = unbounded_node->page;
    //      curr_from = unbounded_node->url;
    //    }else{
    //      ptr = NULL;
    //      curr_from = NULL;
    //    }
    /*set this to the string pointer we are passed with the open page (pop the linked list)*/ 
    while(num_page == 0){
 //     fprintf(stderr,"Wait\n");
      pthread_cond_wait(&fill_unbounded,&n);
  //    fprintf(stderr,"wake up\n");
    }
    unbounded_node = pop();
    if(unbounded_node!=NULL){
      num_page--;
      ptr = strdup(unbounded_node->page);
      curr_from = strdup(unbounded_node->url);
    }
    //printf("in parser after pop()\n");
    pthread_mutex_unlock(&n);


    if(ptr!=NULL&&curr_from!=NULL){
      for (tok = strtok_r(ptr, " \t\n", &saveptr1); tok!=NULL; tok = strtok_r(NULL, " \t\n", &saveptr1))
      {
	if (strncmp(tok, "link:", sizeof(char)*5) == 0)
	{
	  tok1 = strtok_r(tok, ":", &saveptr2);
	  if(check_status(saveptr2)==NEW){
	    pthread_mutex_lock(&m);
	    while(num_url == queue_sz){
	      pthread_cond_wait(&empty_bounded,&m);
	    }
	    //printf("new url is%s\n",saveptr2);
	    //if not , add to the hashmap and call edge() to print the relationship
	    //fprintf(stdout,"now pushing url:%s\n",saveptr2);
	    url_queue[queue_push_ptr] = strdup(saveptr2);
	   // printf("length of savptr2 :%lu\n",strlen(saveptr2));
	    // edge_ptr(curr_from,saveptr2);
	    queue_push_ptr = (queue_push_ptr+1)%queue_sz;
	    num_url++;
	    num_work++;
	   // fprintf(stderr,"num_url:%d\n",num_url);
	    //fprintf(stdout,"%s will be popped from url_queue[]\n",url_queue[queue_pop_ptr]);	
	    //fprintf(stdout,"signalling\n");
	    pthread_cond_signal(&fill_bounded);
	    pthread_mutex_unlock(&m);
	    edge_ptr(curr_from,saveptr2);
	  }
	}
      }
      //pthread_mutex_lock(&l);
    //  fprintf(stderr,"%s is ADDED, now change to DONE\n",curr_from);
      change_status(curr_from,DONE);
      // pthread_mutex_unlock(&l);

    }
    pthread_mutex_lock(&w);
    num_work--;
    if(num_work==0){
      pthread_cond_signal(&empty_work);
    }
    pthread_mutex_unlock(&w);
    //  if(curr_from!=NULL){
    //   }
  }// while(1)
  return NULL;
}	
