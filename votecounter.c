/*
 * VCforStudents.c
 *
 *  Created on: Feb 2, 2018
 *      Author: ayushi
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include "makeargv.h"

#define MAX_NODES 100
char *candidates;
char *directory;
int numCandidates;
int numNodes;
//Function signatures

int parseInput(char *filename, node_t *n);

int parseInputLine(char *s, node_t *n);

int constructNode(char **names, node_t *t);

void addChild(char **names, node_t *p);

int findNode(char *names, node_t *n);

//Helper function Using given name to locate the index of node
int findNode(char *target, node_t *n){

  for(int i = 0; i < numNodes; i++){
	if(strcmp(trimwhitespace(target),trimwhitespace(n[i].name)) == 0) {
	  return i;
	}
  } 
  return -1;
}

// Helper function for finding node by id as one in makeargv.h file does not work with our nodes.
node_t* findNodeById(node_t *n, int tobefound){

  for(int i = 0; i < numNodes; i++){
	if(n[i].id == tobefound) {
	  return &n[i];
	}
  } 
  return NULL;
}


//Helper function to add dependecies
void addChild(char **names, node_t *p){
  int p_idx = findNode(names[0],p);
  int size = 0;
  int childCount = 0;
  if( p_idx == -1){
	printf("Error: Cannot find Parent Node!\n");
  }

  node_t *parent = &p[p_idx];

  while (names[size] != NULL) {
	size++;
  }

  printf("Size :%d \n", size);
  for(int i = 2; i < size; i++){
	parent-> children[i - 2] = findNode(names[i],p);
	childCount++;
  }
  parent->num_children = childCount;
  printf("Number of Children, C: %d\n", childCount);
}

// Helper function to init node_t
int constructNode(char **names, node_t *t){
  int i = 0;

  if(names == NULL){
	printf("Error: Empty name array\n");
  }
  while(names[i] != NULL){
	node_t * node = (node_t*)malloc(sizeof(node_t));
	node->id = i;
	strcpy(node->name,names[i]);
	printf("Node constructed, id: %d, name: %s\n",node->id,node->name);
	t[i++] = *node;	
  }
  return i;    
}


/**Function : parseInput
 * Arguments: 'filename' - name of the input file
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Total Allocated Nodes
 * About parseInput: parseInput is supposed to
 * 1) Open the Input File [There is a utility function provided in utility handbook]
 * 2) Read it line by line : Ignore the empty lines [There is a utility function provided in utility handbook]
 * 3) Call parseInputLine(..) on each one of these lines
 ..After all lines are parsed(and the DAG created)
 4) Assign node->"prog" ie, the commands that each of the nodes has to execute
 For Leaf Nodes: ./leafcounter <arguments> is the command to be executed.
 Please refer to the utility handbook for more details.
 For Non-Leaf Nodes, that are not the root node(ie, the node which declares the winner):
 ./aggregate_votes <arguments> is the application to be executed. [Refer utility handbook]
 For the Node which declares the winner:
 This gets run only once, after all other nodes are done executing
 It uses: ./find_winner <arguments> [Refer utility handbook]
 */

int parseInput(char *filename, node_t *n){
  FILE *fp = file_open(filename);

  //buffer for readline, malloc 1024 based on the Utility Handbook
  char *lineBuf = (char*)malloc(1024);
  char **parsedLine;
  numNodes = 0;


  if(fp == NULL){
	exit(EXIT_FAILURE);
  }

  //Read the first line for candidates
  lineBuf = read_line(lineBuf,fp);  
  makeargv(lineBuf, " ", &parsedLine);
  strtok(lineBuf," "); 
  numCandidates = atoi(parsedLine[0]);
  candidates = (char*)malloc(sizeof(char)*20);
  char *newCand = (char*)malloc(sizeof(char)*20);

  int i; 
  for(i = 1; i <= numCandidates; i++){
	 candidates = strcat(candidates,parsedLine[i]);
	 strcat(candidates," ");
  }
  trimwhitespace(candidates);
  printf("Number of candidates: %d, Candidate Names: %s\n",numCandidates,candidates);

  while(!feof(fp)){
	printf("line read \n");
	lineBuf = read_line(lineBuf,fp);
	if (lineBuf != NULL) {
	  numNodes += parseInputLine(lineBuf,n);
	}
  }

  fclose(fp);

  return numNodes;
}

/**Function : parseInputLine
 * Arguments: 's' - Line to be parsed
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Region Nodes allocated
 * About parseInputLine: parseInputLine is supposed to
 * 1) Split the Input file [Hint: Use makeargv(..)]
 * 2) Recognize the line containing information of
 * candidates(You can assume this will always be the first line containing data).
 * You may want to store the candidate's information
 * 3) Recognize the line containing "All Nodes"
 * (You can assume this will always be the second line containing data)
 * 4) All the other lines containing data, will show how to connect the nodes together
 * You can choose to do this by having a pointer to other nodes, or in a list etc-
 * */

int parseInputLine(char *s, node_t *n){
  char **res;

  makeargv(s, " ",&res);

  // ":" will be in second index if we care scanning 2+ line
  if(strcmp(res[1],":") != 0){
	//Second Line, construct all nodes and store in n
	return constructNode(res, n);
  }else{
	//2+ line, Add dependencies for parents and childs
	addChild(res,n);
	return 0;
  }

}


//bool contains(int children[], int completedVals[]) {

//}

/**Function : execNodes
 * Arguments: 'n' - Pointer to Nodes to be allocated by parsing
 * About execNodes: parseInputLine is supposed to
 * If the node passed has children, fork and execute them first
 * Please note that processes which are independent of each other
 * can and should be running in a parallel fashion
 * */

// Save id's into int array I thought of this as if you go through two for loops
// one for loop then go rerun with completedIds to run with aggregatevotes then finally call who won.
// we might need an array to hold all the output file names for each child. For each agregate vote call.
// we also need this to be run in parallel however calling fork causes two duplicate functions to run.
// break will break from the for loop however im not sure how you stop the child or parent or if it keeps running until
// the end of the file.
// The way im doing it may be wrong. Also we never parse for canidates.
void execNodes(node_t *n) {
  //LeafNode
  if(n->num_children == 0){
	char* output = "Output_";
	char* fileName = n->name;
	char * outputfile = (char *) malloc(1 + strlen(output)+ strlen(fileName));
	strcpy(outputfile, output);
	strcat(outputfile, fileName);
	int fd = open(outputfile, O_CREAT | O_WRONLY|O_APPEND|O_TRUNC,S_IRUSR|S_IWGRP);
	printf("Output file created: %s\n",outputfile);
	char * inputfile = (char *) malloc(1 + strlen(output)+ strlen(fileName));
	strcpy(inputfile,directory);
    strcat(inputfile,"/");	
    strcat(inputfile,n->name);	
	//printf("numCandidates: %d, NameCandidates:%s",numCandidates,candidates);
	printf("InputFile: %s,OutputFile:%s, numCandidates: %d, candidates: %s\n",inputfile,outputfile,numCandidates,candidates);
	execl("./leafcounter","./leafcounter",inputfile, outputfile, numCandidates, candidates, (char*)NULL);
	return;
  }
  else{
	//Non-LeafNode	
	//if has child fork and recursive call execNodes()
	pid_t pid;
	int currNumChildren = n->num_children;
	char* childOutputs[currNumChildren];
	int i;

	//For each Child, fork
	for(i = 0; i < currNumChildren;i++){
	  node_t *currChild = findNodeById(n,n->children[i]);
	  //pid = fork();
	  //if(pid == 0){
		execNodes(currChild);
	//   }else{
	  //	  wait(&pid);
       	  char* output = "Output_";
	  	  char * outputfile = (char *) malloc(1 + strlen(output)*10*sizeof(char));
	  	  strcpy(outputfile, output);
	  	  strcat(outputfile, n->name);
	  	  childOutputs[i] = outputfile;
	  	  continue;
	    //}
	  }//End of iteration
	//Parent node exec "aggregate_votes" and "find_winner";"
	//if(pid != 0){
	 // wait(&pid);
	 char* output = (char*)malloc(1024);
	 strcat(output,n->name);
	  if (strcmp(n->name, "Who_Won") == 0) {
		char*argv[5+currNumChildren+numCandidates];
	//	execl("./find_winner",currNumChildren,)
	  } else {
	//	execv("aggregate_votes", args);
	  }
	//}

  }//End of non-leaf

}

int main(int argc, char **argv){

  //Allocate space for MAX_NODES to node pointer
  struct node* mainnodes=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

  if (argc != 2){
	printf("Usage: %s Program\n", argv[0]);
	return -1;
  }
  
  //call parseInput
  int num = parseInput(argv[1], mainnodes);
  //	printf("Number of Nodes: %d,",num);
  directory =(char*)malloc(1024*sizeof(char));
  
  strcpy(directory,strtok(argv[1],"/"));
  printf("directory:%s\n",directory);
  //Call execNodes on the root node
  //printgraph(mainnodes, num);
  execNodes(mainnodes);
  return 0;
}
