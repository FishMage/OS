#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "crawler.h"
#include "cs537.h"
#include "cs537.h"



int main(int argc, char *argv[]) {
char *saveptr1;
char *saveptr2 = NULL;
char *tok = NULL;
char *tok1 = NULL;
		char * ptr = strdup( "Welcome! link:pageb link:pagec");
		
		/*set this to the string pointer we are passed with the open page (pop the linked list)*/

	for (tok = strtok_r(ptr, " \t\n", &saveptr1); tok; tok = strtok_r(NULL, " \t\n", &saveptr1))
	{
		if (strncmp(tok, "link:", sizeof(char)*5) == 0)
		{
			printf("tok is: %s\n",tok);
			
			tok1 = strtok_r(tok, ":", &saveptr2);
			printf("saveptr is: %s\n", saveptr2);
			 char* arrow = strdup("->");
	   char* temp_curr_from = strdup("123");
	   char* tobe_checked1 =  strcat(arrow,saveptr2);
	   strcat(temp_curr_from,tobe_checked1);
	   printf("tobe_checked2:***%s***\n",temp_curr_from);
		//call strtok here
		//an address is next....  add link to downloader's queue AND return edge to program
		}
	  
      
	}
			return 0;
}
