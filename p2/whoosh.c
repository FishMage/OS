#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <dirent.h>
#define MAX_LINE 130
#define MAX_ARGS 5
int is_red;
char *args [MAX_LINE];
char * path [MAX_LINE];
int has_path; // if 0, use default path
int path_ctr,num_args ;
char *f_output, *err_output;
FILE *out_stream,* err_stream;
int out_fd,err_fd;


int pwd();
int get_path();
int other_cmd();
int is_redirect(char*cmd);
int get_args(char * s);
int cd(char* path);
void clear_args();
char* search_cmd(char *cmd);

void prompt_error(){
	char error_message[30] = "An error has occurred\n";
	write(fileno(err_stream), error_message, strlen(error_message));
}

void prompt_whoosh(){
	char whoosh[20] = "whoosh> ";
	write(fileno(out_stream),whoosh, strlen(whoosh));
}
int other_cmd(char * cmd){
	printf("cmd: %s\n",cmd);
	char* outf = malloc(MAX_LINE*sizeof(char));
	char* oute = malloc(MAX_LINE*sizeof(char));
	outf = strdup("");
	oute = strdup("");

	char* myargv[MAX_ARGS];

	int i;
	for(i = 0; i< MAX_ARGS; i++){
		myargv[i] = malloc(MAX_LINE*sizeof(char));
	}

		strcpy(myargv[0],cmd);
		

	int j;
	for(j = 1; j < MAX_ARGS; j++){
		if(args[j]==NULL||(strcmp(args[j],"")==0))
			break;
			myargv[j] = strdup(args[j]);  		
	}
	myargv[j] =NULL;
	
	//execute the command
	int rc = fork();
		if(rc==0){
		
		if(is_red == 1){
		int i;
		for(i = 1; i < num_args; i++){
			if(strcmp(args[i],">")==0){
				if((args[i+1] != NULL) &&(args[i+2]==NULL)){
				f_output = args[i+1];	
				strcat(outf,f_output);
				strcat(oute,f_output);
				strcat(outf,".out");	
				strcat(oute,".err");	
					
				close(STDOUT_FILENO);
				out_fd = open(outf,O_WRONLY, S_IRUSR);
				if(out_fd<0){
					prompt_error();
					return -1;
				}
				close(STDOUT_FILENO);
				err_fd = open(oute,O_WRONLY, S_IRUSR);
					if(err_fd<0){
					prompt_error();
					return -1;
				}
				args[i] = NULL;
				break;	
				}
			    }
	         	}
		}
		execv(myargv[0],myargv);
		exit(0);
		return 0;	
		}

	else if(rc > 0){
		wait(NULL);
		return 1;
	}else{
		return 1;
	}
	
	return 0;
}
int is_redirect(char * cmd){
	int i;
	//char* target = NULL;
	for(i = 0; i<strlen(cmd);i++){
		if(cmd[i] =='>'){
		 f_output = cmd+ i+1;
		 
		//close(fileno(stdout));
			is_red = 1;
		return 0;
		}
	}
	return 1;
}
int get_args(char * s){
	int i,j,arg_ctr, space_ctr;
	arg_ctr = 0;
	j = 0;
	space_ctr = 0;
	while(j<strlen(s)){
		if(s[j] ==' ')
		space_ctr++;
		j++;
	}
	if(space_ctr > 0){
	char* temp = malloc(MAX_LINE*sizeof(char));
	temp = strtok(s," ");
	i = 0;
	while(temp!=NULL){
		char* itr = malloc(MAX_LINE*sizeof(char)); 
			strcpy(itr,temp);
		args[i] = itr;
		temp = strtok(NULL," ");
		arg_ctr ++;
		i++;
		}
	}else if(space_ctr == 0){
		s[strlen(s) -1] = '\0';
		strcpy(args[0], s);
		arg_ctr=1;
		return 1;
	}
	args[i-1] = strtok(args[i-1],"\n");
	args[i] = NULL;
	return arg_ctr;
}
int cd(char* path){
	 if(chdir(path)<0)
	 	return- 1;
	else return 0;	 
}

int pwd(){
	char * cwd = malloc(MAX_LINE*sizeof(char));
	cwd = getcwd(cwd,MAX_LINE*sizeof(char));
	printf("%s\n",cwd);
	fflush(stdout);
	free(cwd);
	return 1;
}

int get_path(){
	int i;
	for(i = 0; i < MAX_LINE; i++){
		if(args[i+1]==NULL)
			break;
		strcpy(path[i],args[i+1]);
	}
	has_path =1;
	return i;
}
char* search_cmd(char *cmd){		
	char* slash = strdup("/");
	char* completed_path = malloc(MAX_LINE*sizeof(char));
	char* searched_cmd = malloc(MAX_LINE*sizeof(char));
	int i;
	struct stat s;
	char* modified_cmd = strcat(slash,cmd);
	if(has_path ==0){
		strcpy(searched_cmd,path[0]);
		completed_path = strcat(searched_cmd,modified_cmd);
		if(stat(completed_path,&s)<0){
			return "notfound";
		}
		return completed_path;
	}
	else{
		for(i = 0; i< path_ctr; i++){
      		 	strcpy(searched_cmd,path[i]);
			completed_path = strcat(searched_cmd,modified_cmd);
			if(stat(completed_path,&s)==0){
				return completed_path;
			}
		}
			return "notfound";
	}
	return "notfound";
}
void clear_args(){
	int i;
	for(i = 0; i<num_args; i++){
		if(strcmp(args[i],"")!=0)
		args[i] = NULL;
	}
}

int main(int argc, char* argv[]){
	int i;
	is_red = 0;
	has_path = 0;
	out_stream = stdout;
	err_stream = stderr;
	for(i = 0; i< MAX_LINE; i++){
		path[i] = malloc(MAX_LINE*sizeof(char));
	}
	strcpy(path[0],"/bin");
	int cmd_size = MAX_LINE*sizeof(char);
	char* cmd = malloc(cmd_size);
	char* searched_cmd= malloc(cmd_size);
		while(1){
		for(i = 0; i< MAX_LINE; i++){
		args[i] = malloc(MAX_LINE*sizeof(char));
			}
		prompt_whoosh();
		if(fgets(cmd, cmd_size, stdin) != NULL){
		if((strlen(cmd)!=0) &&( cmd[strlen(cmd)-1]!='\n')){
			char j = getchar();
			while(j!='\n'){
			 j = getchar();
			}
			prompt_error();
			continue;
		}	
		if(is_redirect(cmd)==0){
			continue;
		}
		num_args = get_args(cmd);
		if(strcmp("exit",args[0])==0){
			fclose(out_stream);
			exit(0);
		}
		else if(strcmp("cd",args[0])==0){
			if(num_args > 1){
				if(cd(args[1])<0){
					prompt_error();
					continue;
				}
			}else{
			if(cd(getenv("HOME"))<0){
				prompt_error();
				continue;
				}
			}
		}	
		else if(strcmp("pwd",args[0])==0){
			pwd();
			continue;
		}
		else if(strcmp("path",args[0])==0){
			path_ctr = get_path();	
			continue;
		}else if(strcmp("\n",args[0])==0){
			continue;	
		}else if(strcmp(" ",args[0])==0)
			continue;
		
		else{
			searched_cmd  = search_cmd(args[0]);
			if(strcmp("notfound",searched_cmd)==0){
				prompt_error();
				continue;
			}
			other_cmd(searched_cmd);
			continue;
			}
		}
	}
	return 0;
}
