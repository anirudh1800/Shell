/*------------------shellparser.h-----------------*/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<signal.h>
#include<errno.h>
#include<dirent.h>
#include<sys/utsname.h>
#include<sys/stat.h>
#include<time.h>

#define VAR_MAX_NUM 10
#define PIPE_MAX_NUM 10
#define FILE_MAX_SIZE 40

#define MAXLINE 90


/*Structure to store the user typed commands with arguments*/

struct commandType{
	char *command;
	char *varList[VAR_MAX_NUM];
	int varNum;
};

/*
 * Structure that stores the commbination of commands supporting pipes 
 * and input/output redirection.
 */
 
typedef struct parseInfo parseInfo;
 
struct parseInfo{
  int   boolInfile;		       
  int   boolOutfile;		      
  int   boolBackground;		       
  struct commandType CommArray[PIPE_MAX_NUM];
  int   pipeNum;
  char  inFile[FILE_MAX_SIZE];	       
  char  outFile[FILE_MAX_SIZE];	       
};

parseInfo *parse(char *); /*Function to parse the user typed commands*/

void init_info(parseInfo*);/*Function the initialize the structure variables in parseInfo varible*/

int isPipe(char c);/*Function to check pipes*/

void parse_command(char *string,struct commandType *);/*Function to parse a single command*/

void free_info(parseInfo *);/*Function to free the parseInfo pointer variable*/

void print_info(parseInfo *);/*Function to print the contents of parseInfo pointer variable*/
