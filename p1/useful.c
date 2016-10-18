char * temp = strtok(s2, " ");
while(temp != NULL){
	fprintf(stdout, "&s\n",temp);
	temp = strtok(NULL, " ");
}
