	#include <stdlib.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <string.h>
	#include <assert.h>
	#include <stdint.h>
	#include <pthread.h>
	#define LINKLENGTH 256
	
	struct node {
	char* page;
	struct node *next;
	};
	
	
	pthread_cond_t empty;
	pthread_cond_t fill;
	pthread_mutex_t m;
	//crawler member function
	void * downloader(void*param);
	void * parser(void *param);
	
	//linkedlist interface
	void push(page * p); //return true if successfully add the node
	char *pop();	 
	//bounded list
	char** url_queue;
	
	node head = NULL;
	
	int num_url = 0;
	int queue_ptr = 0;
	int queue_sz;
	/*
	void do_fill(char * raw_url){
		char* url = (char*)malloc(LINKLENGTH*sizeof(char));
		char* original_url;
		strcpy(original_url, raw_url);
		
		strtok_r(url, "\"", &original_url);
	    strtok_r(url, "\"", &original_url);
		
		strcpy(url_queue[queue_ptr], url);
		queue_ptr = (queue_ptr+1)%queue_sz;
		isfull++;
		}
	*/
	
	
	void push(char* p){
		node* temp = head;
		while(temp->next != NULL ){
			temp = temp->next;
			}
		temp ->next = malloc(sizeof(node));
		temp->next->page = p;
		temp->next->next = NULL;

		}
		
	char *pop(){		
		if(head == NULL){
			return NULL
			}
		
		node* next_node = head->next;
		char* p = strdup(head->page);//free?
		
		free(head);
		head = next_node;
		
		return p; 
		}
	
	//Downloader
	void * downloader(void *param)
		{
			while (num_url > 0)
			{
					char* page = (char*)malloc(256*sizeof(char));
					page = fetch(url_queue[queue_ptr]);
					queue_ptr = (queue_ptr+1)%queue_sz;
					num_url--;
					//TODO:
					//push the page into parser queue
					push(page);
			}
			return NULL;
		}
	
	//Parser
	void * parser(void *param)
	{
	char *saveptr1;
	char *saveptr2 = NULL;
	char *tok = NULL;
	char *tok1 = NULL;
	char * ptr = pop();
		
		/*set this to the string pointer we are passed with the open page (pop the linked list)*/

	for (tok = strtok_r(ptr, " \t\n", &saveptr1); tok; tok = strtok_r(NULL, " \t\n", &saveptr1))
	{
		if (strncmp(tok, "link:", sizeof(char)*5) == 0)
		{
			printf("tok is: %s\n",tok);
			
			tok1 = strtok_r(tok, ":", &saveptr2);
			printf("saveptr is: %s\n", saveptr2);
			
		//call strtok here
		//an address is next....  add link to downloader's queue AND return edge to program
		}	  		
	}
			return 0;
		}		
			
		/*
			int i;
			for(i = 0; i < queue_sz; i++){
				pthread_mutex_lock(&m);
				while(isfull == queue_sz )
				pthread_cond_wait(&empty, &m);
				//get raw url
				
				//parsing
				//do_fill
				do_fill(test_url);
				pthread_cond_signal(&fill);
				pthread_mutex_unlock(&m);
				}
				*/
		
		
	int crawl(char *start_url,
		  int download_workers,
		  int parse_workers,
		  int queue_size,
		  char * (*_fetch_fn)(char *url),
		  void (*_edge_fn)(char *from, char *to)) 
	{
	int i, j, k, t;	  
	//	Your crawl() function should return when the entire graph has been visited, but we don't require you to cleanly exit from the thread pools
	/*
	int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
	                          void *(*start_routine) (void *), void *arg);
	*/

		pthread_mutex_init(&m, NULL);
		pthread_cond_init(&fill, NULL);
		pthread_cond_init(&empty, NULL);
		
		//initialize the downloader queue(bounded queue)
		url_queue = malloc(sizeof(char*)*queue_size);
		url_queue[0] = start_url; //is this right?
		num_url++;
		queue_sz = queue_size;

	pthread_t downloader [download_workers];
	pthread_t parser [parse_workers];
	
	for(j = 0; j < download_workers;j++)
	{
		int rc_1 = pthread_create(&p,NULL,&downloader[j],NULL);
		assert(rc_1 == 0);
	}
		
	for(k = 0; k < parse_workers;k++)
	{	
		int rc_2 = pthread_create(&p,NULL,&parser[k], NULL);
		assert(rc_2 == 0);
	}
	

	for(i = 0; i< download_workers; i++){
		pthread_join(downloader[i],NULL);
	}
	for(t = 0; t< parse_workers; t++){
		pthread_join(parser[t],NULL);
	}
	//free
	free(url_queue);
	  return 0;
	}
	
