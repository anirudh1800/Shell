/*---------------shell parser.c-----------------*/ 
 
#include"shellparser.h"

void init_info(parseInfo *p){
	p->pipeNum=0;
	p->boolInfile=0;		 
	p->boolOutfile=0;		       
	p->boolBackground=0;
}

int isPipe(char c){
	if(c=='&'||c=='|')
		return 0 ;/* not fully implemented */
	else
		return 0;
}

void resize(int pos,char *a[],int varcount){
	int i,len=varcount+1,point=0;
	char *temp[varcount+1];
	for(i=0;i<len;i++){
		temp[i]=(char*)malloc(sizeof(char)*50);
		memset(temp[i],'\0',50);
	}
	for(i=0;i<len;i++){
		if(i==pos)
			memcpy(temp[i],"",1);
		else{
			memcpy(temp[i],a[point],50);
			point++;
		}
	}
	for(i=0;i<len-1;i++)
		free(a[i]);
	for(i=0;i<len;i++){
			a[i]=(char*)malloc(sizeof(char)*50);
			a[i]=temp[i];
	}
}


void parse_command(char *string,struct commandType *comm){
	int j=0,i=0;
	int len=strlen(string)-1;
	comm->varNum=0;
	char *start=string;
	comm->command=(char*)malloc(sizeof(char)*50);
	memset(comm->command,'\0',sizeof(char)*50);
	int c;
	while((*string)!=' '){
			if((*string)=='\0')
				break;
		i++;
		string++;
	}
	strncpy(comm->command,start,i);
	if((*string)=='\0')
		return;
	string++;
	comm->varList[j]=(char*)malloc(sizeof(char)*50);
	strcpy(comm->varList[j],"");
	while((string-start)<=len){
		comm->varList[j]=(char*)malloc(sizeof(char)*50);
		memset(comm->varList[j],'\0',sizeof(char)*50);
		char *temp=string;
		while((*string)!=' '){
			string++;
			if((*string)=='\0'){
				break;
			}
		}
		if((*string)==' '||(*string)=='\0'){
			strncpy(comm->varList[j],temp,(string-temp));
				comm->varNum++;
				assert(comm->varNum<VAR_MAX_NUM);
		}
		if((*string)=='\0')
			break;
		string++;
		j++;	
	}
}

parseInfo* parse(char *cmdLine){
	parseInfo *result;
	char command[MAXLINE];
	int i;
	for(i=0;i<MAXLINE;i++)
		command[i]='\0';
	i=0;
	if(cmdLine[0]=='\n' || cmdLine[0]=='\0')
		return NULL;
	result=(parseInfo*)malloc(sizeof(parseInfo));
	init_info(result);
	for(;(*cmdLine)!='\0';cmdLine++,i++){
			if(isPipe(*cmdLine)){
				parse_command(command,&result->CommArray[result->pipeNum]);
				result->pipeNum++;
					i=0;continue;
			}
			else if(*(cmdLine+1)=='>'||*(cmdLine+1)=='<'){
					char c=*(cmdLine+1);
					command[i]=*cmdLine;
					parse_command(command,&result->CommArray[result->pipeNum]);
					cmdLine++;cmdLine++;
					char *str=cmdLine;
					while((*cmdLine)==' '){
						cmdLine++;
						str++;
					}
					while((*cmdLine)!=' '){
						if((*cmdLine)=='\0')
							break;
						cmdLine++;
					}
					if(c=='>'){
						strncpy(result->outFile,str,(cmdLine-str));
						result->boolOutfile=1;
					}
					else{
						strncpy(result->inFile,str,(cmdLine-str));
						result->boolInfile=1;
					}
			}
			else if(*(cmdLine+1)=='&')
				result->boolBackground=1;
			else if(*(cmdLine+1)=='\0'){
					command[i]=*cmdLine;
					parse_command(command,&result->CommArray[result->pipeNum]);
					break;
			}
			else 
				command[i]=*cmdLine;
			if((*cmdLine)=='\0')
				break;
	}
	return result;
}

void free_info(parseInfo* garbage){
	 if(garbage!=NULL){
		 int i,j=garbage->pipeNum;
		 while(j>=0){
			struct commandType *dispose=&garbage->CommArray[j];
			free(dispose->command);
			for( i=0;i<dispose->varNum;i++)
					free(dispose->varList[i]);
			j--;
			}
			free(garbage);
		}
}	 
		
void print_info(parseInfo *info){
	if(info!=NULL){
		int i,j=info->pipeNum;
		if(info->boolInfile==1)
			fprintf(stdout,"Infile:%s\n",info->inFile);
		if(info->boolOutfile==1)		      
			fprintf(stdout,"Outfile:%s\n",info->outFile);		       
		fprintf(stdout,"Background:%d\n",info->boolBackground);
		 while(j>=0){
			struct commandType *com;
			com=&info->CommArray[j];
			fprintf(stdout,"command:%s\n",com->command);
			fprintf(stdout,"variables no:%d\n",com->varNum);
			for( i=1;i<=com->varNum;i++)
						fprintf(stdout,"var[%d]:%s\t",i,com->varList[i]);
			j--;
			}
			fprintf(stdout,"\n");
		}
}		
