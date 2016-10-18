#include "fastsort.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 128
#define MAXWORD 150
int static wordIndex;
FILE *fp;
int nlines;
int fileindex = 0;

char * dataLineptr[MAXLINE];

int readlines(char *lineptr[],FILE *fp);

void outputResult(char *lineptr[], int nlines);

int readlines(char *lineptr[],FILE *fp){
    char* emptyln = "\n";
    int lineCount = 0;
    int i = 0;
	char p[128];
//	p = (char*)malloc(sizeof(char)*MAXWORD);
//	if(p == NULL)
//	fprintf(stderr,"Error: Malloc fails\n");
	while(fgets(p,MAXWORD, fp)!=NULL){
	if(p[0]=='\0'){
		fprintf(stderr,"Bad command line parameters\n");
		exit(1);
	}
	if((p[127]!= '\n')&&(p[127]!='\0')){
		fprintf(stderr,"Line too long\n");
		exit(1);
	}
	if (strcmp(p,emptyln)==0){
	lineptr[i] = "";
	return lineCount;
	}
	lineptr[i] = (char*)malloc(sizeof(char)*MAXWORD);
	strcpy(lineptr[i],p);
        i++;
        lineCount++;
    }
    return lineCount;
}

void outputResult(char *lineptr[], int nlines){
    int i;
    for ( i = 0; i < nlines; i++) {
        printf("%s",lineptr[i]);
    }
}
int compare(const void *k1, const void * k2){
	char *word1 = (char*)malloc(sizeof(char)*MAXLINE);
 	if(word1 == NULL)
	fprintf(stderr,"Error: Malloc fails\n");

	char *word2 = (char*)malloc(sizeof(char)*MAXLINE);
	if(word2 == NULL)
	fprintf(stderr,"Error: Malloc fails\n");

	char *str1cpy= (char*)malloc(sizeof(char)*MAXLINE);
	if(str1cpy == NULL)
	fprintf(stderr,"Error: Malloc fails\n");

	char *str2cpy = (char*)malloc(sizeof(char)*MAXLINE);
	if(str2cpy == NULL)
	fprintf(stderr,"Error: Malloc fails\n");

	strcpy(str1cpy, *(char**)k1);
	strcpy(str2cpy, *(char**)k2);
	
//	printf("str1cpy:%s",str1cpy);
//	printf("str2cpy:%s",str2cpy);
	if(wordIndex == 1){
		return strcmp(strtok(str1cpy," "),strtok(str2cpy," "));
	}
	int i;
	char* temp =  strtok(str1cpy," ");
	for(i = 1; i < wordIndex; i++ ){
		temp = strtok(NULL," ");
		if(temp == NULL)
		break;
		strcpy(word1, temp);
	}
	temp = strtok(str2cpy," ");
	for(i = 1; i < wordIndex; i++ ){
		temp = strtok(NULL," ");	
		if(temp == NULL)
		break;
		strcpy(word2, temp);
	}
//	printf("word1:%s,word2:%s\n",word1,word2);
//	free(word1);free(word2);
//	free(str1cpy);free(str2cpy);
	return strcmp(word1, word2);
	
}

int main(int argc, char*argv[]){
    if (argc > 3) {
        fprintf(stderr,"Error: Bad command line parameters\n");
        exit(1);
    }
    if (argc <= 3) {
        if (argc == 2) {
            wordIndex = 1;
            fileindex = 1;
        }else{
    if(argc == 1){
      	fprintf(stderr,"Error: Bad command line parameters\n");
		exit(1);
	}
            //1st Argument: calculate the index of the word in the sentence given
            int charWord = atoi(argv[1]);
	if(charWord==0){
        fprintf(stderr,"Error: Bad command line parameters\n");
        exit(1);
	}
            wordIndex = -charWord;
            fileindex = 2;
        }
//	printf("wordIndex:%d\n",wordIndex);
            //2nd argument: open the file and store the data
            fp = fopen(argv[fileindex],"r");
            if (fp == NULL ){
                fprintf(stderr,"Error: Cannot open file %s\n",argv[fileindex]);
                exit(1);
            }
            struct stat fileStat;
            int fd = fileno(fp);
            if (fstat(fd, &fileStat) < 0)
            return 1;
            //int fsize = (int)fileStat.st_size;
            
            nlines = readlines(dataLineptr, fp);
//	int i;
//	for(i = 0; i< nlines; i++){
//       	printf("dataLineptr[%d]%s",i,dataLineptr[i]);
//	}
            qsort(dataLineptr, nlines, sizeof(char*), compare);
	//print the result
            outputResult(dataLineptr, nlines);
		int i;
		for(i = 0; i< nlines; i++){
		free(dataLineptr[i]);
	}

//	free(word1);free(word2);
            fclose(fp);
        }
    
    return 0;
}

