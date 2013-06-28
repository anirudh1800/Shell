
/*------------shell.c------------*/
#include"shellparser.h"

#define CWDL 32
#define JOBS_MAX 10
#define MAXHIS 50

extern int errno;

typedef struct job job;
typedef struct bjobs bjobs;
typedef struct fifo fifo;

/*Structure to store a background process*/

struct job{
	int status;
	pid_t pid;
	parseInfo cmd;
};

/*Structure to store all the background processes that are active*/
 
struct bjobs{
	int no;
	job jobs[JOBS_MAX];
};

/*Structure to store each command*/

struct History{
	int no;
	char *command;
};

/*Structure to store entries of commands in a fifo buffer*/

struct fifo{
	int entries;
	struct History history[MAXHIS];
};

/*Global Variables*/

char *usr,*buf,*readbuf;
char *cmdLine;
parseInfo *cmd; 
pid_t childpid;
fifo *past;
bjobs *backjobs; 
int statusw;

/*Signal handler for Ctrl+c */

void signalhandler(){
	if(childpid>0){
		printf("\n");
		fflush(stdout);
		kill(childpid,SIGINT);
	}
} 

/*Function to check if the command is built-in*/

int isBuiltInCommand(parseInfo* cmd){
	int pipno=cmd->pipeNum;
	char *c=cmd->CommArray[0].command;
	if ( strncmp(c, "exit", sizeof(c)) == 0)
		return 1;
	if(strncmp(c,"cd",sizeof(c))==0)
		return  1;
	if(strncmp(c,"jobs",sizeof(c))==0)
		return 1;
	if(strncmp(c,"ls",sizeof(c))==0)
		return 1;
	if(strncmp(c,"history",sizeof(c))==0)
		return 1;
	if(strncmp(c,"help",sizeof(c))==0)
		return 1;
	return 0;
}

/*Function to execute built-in commands inside the shell*/

void executeBuiltInCommand(parseInfo *cmd){
	int i=0,pipno=cmd->pipeNum;
	char *c=cmd->CommArray[0].command;
	if (strncmp(c, "exit", sizeof(c)) == 0){
		execlp("/usr/bin/clear","clear",NULL);
		free_info(cmd);
		free(cmdLine);
		free(usr);
		free(buf);
		free(past);
		free(backjobs);
		exit(0);
	}
	else if(strncmp(c,"cd",sizeof(c))==0){
		i=chdir(cmd->CommArray[0].varList[0]);
		if(i==-1)
			fprintf(stderr,"%s:%s\n",cmd->CommArray[0].varList[0],strerror(errno));
	}
	else if(strncmp(c,"ls",sizeof(c))==0){
		char type[25];
		DIR *dir;
		struct dirent *ent;
		char *buf;
		struct stat sb;
		buf=(char*)malloc(CWDL+1);
		getcwd(buf,CWDL-1);
		dir=opendir(buf);
		fprintf(stdout,"filename--filetype--inode no--file size--last file modification\n\n");
		if(dir!=NULL){
			while ((ent = readdir (dir)) != NULL){
				if((strncmp(ent->d_name,"..",2)==0)	||(strncmp(ent->d_name,".",1)==0))
					continue;
				  if(stat(ent->d_name,&sb)!=-1){
					 fprintf(stdout,"%s--",ent->d_name);
					switch (sb.st_mode & S_IFMT) {
						case S_IFBLK:fprintf(stdout,"block device"); 
						              break;
						case S_IFCHR:fprintf(stdout,"character device");
						              break;
						case S_IFDIR:fprintf(stdout,"directory");
						              break;
						case S_IFIFO:fprintf(stdout,"FIFO/pipe");
						              break;
						case S_IFLNK:fprintf(stdout,"symlink");
						              break;
						case S_IFREG:fprintf(stdout,"regular file");
						              break;
						case S_IFSOCK:fprintf(stdout,"socket");
						               break;
						default:fprintf(stdout,"unknown?");
						         break;
				   }
					fprintf(stdout,"--%ld", (long) sb.st_ino);
					fprintf(stdout,"--%lld bytes",(long long) sb.st_size);
					fprintf(stdout,"--%s\n",ctime(&sb.st_mtime));
				}
			}
			closedir (dir);
		}
		else
			fprintf(stderr,"%s\n",strerror(errno));
		free(buf);
	}
	else if(strncmp(c,"history",sizeof(c))==0){
			int i,l=past->entries;
			fprintf(stdout,"History\n");
			for(i=0;i<l;i++)
				fprintf(stdout,"%d\t%s\n",past->history[i].no,past->history[i].command);
	}
	else if(strncmp(c,"help",sizeof(c))==0){
		fprintf(stdout,"help -- Shell Usage:\n"); 
		fprintf(stdout,"[command] [arguments,options] [>,<] [filename]\n");
		fprintf(stdout,"Built-in commands:\ncd\t\tchange current working directory\n");
		fprintf(stdout,"ls\t\tlist all the files and directories present in the current working directory\n");
		fprintf(stdout,"history\t\tpreview the previously typed commands\n");
		fprintf(stdout,"jobs\t\tlists current background jobs running\n");
		fprintf(stdout,"help\t\tdisplay the usage of shell\n");
		fprintf(stdout,"exit\t\texit from the shell\n"); 
	}
	else if(strncmp(c,"jobs",sizeof(c))==0){
		int i=0,num=backjobs->no;
		while(num!=0){
			job j=backjobs->jobs[i];
			parseInfo pi=j.cmd;
			fprintf(stdout,"PROCESS ID\tPROCESS NAME\tSTATUS\n");
			fprintf(stdout,"%d\t\t",j.pid);
			fprintf(stdout,"%s\t\t%d",pi.CommArray[0].command,j.status);
			fprintf(stdout,"\n");
			i++;num--;
		}
	}
}
/*Function to read command from the user prompt in the shell*/

void readCommand(char *cmdLine){
	char c;
	int i=0;
	while((c=getchar())!='\n'){
				cmdLine[i]=c;i++;	
	}
	cmdLine[i]='\0';
}

/*
 * Execute the user typed command that is not built-in using 
 * executeCommand function
 */

int executeCommand(parseInfo *cmd){
	int i=0,pipeno=cmd->pipeNum;
	fflush(stdout);
	FILE *fp=NULL;
	if(cmd->boolOutfile==1)
		if((fp=freopen(cmd->outFile, "w" ,stdout))==NULL){
			fprintf(stderr,"%s\n",strerror(errno));
			exit(1);
		}
    else if(cmd->boolInfile==1){
		fp=fopen(cmd->inFile,"r");
		if(fp==NULL)
			fprintf(stderr,"%s\n",strerror(errno));
	}
	if(pipeno==0){
		if(cmd->CommArray[0].varNum==0){
			i=execlp(cmd->CommArray[0].command,cmd->CommArray[0].command,NULL);}
		else
			i=execvp(cmd->CommArray[0].command,cmd->CommArray[0].varList);
	}
	if(fp!=NULL)
		fclose(fp);
	if(i==-1)
		fprintf(stderr,"error:'%s' command not found\n",cmd->CommArray[0].command);
	return i;
}

/*Add the user typed command to history buffer*/

void addtoHistory(char *commandline,fifo *past){
		past->history[past->entries].command=(char*)malloc(sizeof(char)*50);
		strncpy(past->history[past->entries].command,commandline,sizeof(commandline));
		past->history[past->entries].no=past->entries;
		past->entries++;
		if(past->entries==50)
			past->entries=(past->entries)%50;
}

/*
 * Function to check if user typed command is to be executed as a 
 * background process
 */

int isBackgroundJob(parseInfo *cmd){
	return cmd->boolBackground;
}

/*Function to record the background jobs into the backjobs structure*/

void record(parseInfo *command,pid_t id,int status){
	 int num=backjobs->no++;
	 backjobs->jobs[num].cmd=*command;
	 backjobs->jobs[num].pid=id;
	 backjobs->jobs[num].status=status;
}

/*Main function*/

int main (int argc, char **argv){
    int status;
    char *prompt="%";
    pid_t wpid;
    struct utsname uts;
    char *underline="\033[4m";
    usr=getlogin();
    if(usr==NULL){
		uname(&uts);
		usr=uts.sysname;
	}
	system("clear");
	status=-1;
	backjobs=(bjobs*)malloc(sizeof(bjobs));
    cmdLine=(char*)malloc(MAXLINE);
    readbuf=(char*)malloc(4096);
    buf=(char*)malloc(CWDL+1);
    past=(fifo*)malloc(sizeof(fifo));
    backjobs->no=0;
    past->entries=0;
    signal(SIGINT,signalhandler);
    fflush(stdout);
	memset(cmdLine,'\0',MAXLINE);
	fprintf(stdout,"%sShell-Mini Project\n",underline);
	fprintf(stdout,"\033[24m");
	while(1){
			getcwd(buf,CWDL-1);
			cmdLine=(char*)malloc(MAXLINE);
			fprintf(stdout,"[shell-mini:%s@%s]$",usr,buf);
			readCommand(cmdLine);
			addtoHistory(cmdLine,past);
			if (cmdLine == NULL) 
				continue;
			else if(cmdLine[0]=='\0')
				continue;
			cmd=parse(cmdLine);
			if(isBuiltInCommand(cmd))
					executeBuiltInCommand(cmd);
			else{   
					childpid=fork();
					if(childpid==0){
						status=executeCommand(cmd);
						return status;
					}
					else{
						if(isBackgroundJob(cmd)){
							wpid=fork();
							if(wpid==0){
								record(cmd,childpid,-1);
								wait(&statusw);
								if(WIFEXITED(statusw))
									record(cmd,childpid,0);	
							}	
						}
						else 
							waitpid(childpid,&status,0);
				}
			}
			memset(cmdLine,'\0',MAXLINE);
			errno=0;
	}
	return 0;
}
