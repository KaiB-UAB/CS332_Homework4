#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
typedef struct pthread_args
{
	pthread_t ptid;
  	int place;
  	char *command;
	char *args;
} JOBS;
/* create the queue data structure and initialize it */
queue *queue_init(int n) {
	queue *q = (queue *)malloc(sizeof(queue));
	q->size = n;
	q->buffer = malloc(sizeof(int)*n);
	q->start = 0;
	q->end = 0;
	q->count = 0;

	return q;
}

/* insert an item into the queue, update the pointers and count, and
   return the no. of items in the queue (-1 if queue is null or full) */
int queue_insert(queue *q, int item) {
	if ((q == NULL) || (q->count == q->size))
	   return -1;

	q->buffer[q->end % q->size] = item;	
	q->end = (q->end + 1) % q->size;
	q->count++;
	printf("job %d added to the queue\n",q->count);
	return q->count;
}

/* delete an item from the queue, update the pointers and count, and 
   return the item deleted (-1 if queue is null or empty) */
int queue_delete(queue *q) {
	if ((q == NULL) || (q->count == 0))
	   return -1;

	int x = q->buffer[q->start];
	q->start = (q->start + 1) % q->size;
	q->count--;

	return x;
}

/* display the contents of the queue data structure */
void queue_display(queue *q) {
	int i;
	if (q != NULL && q->count != 0) {
		printf("queue has %d elements, start = %d, end = %d\n", 
			q->count, q->start, q->end);
		printf("queue contents: ");
		for (i = 0; i < q->count; i++)
	    		printf("%d ", q->buffer[(q->start + i) % q->size]);
		printf("\n");
	} else{
		printf("queue empty, nothing to display\n");
	}
}

/* delete the queue data structure */
void queue_destroy(queue *q) {
	free(q->buffer);
	free(q);
}

void *run_job(void *arg){
	char buf[BUFSIZ];
	char bufTwo[BUFSIZ];
	int fdout,fderr;
	struct pthread_args *task;
	task=(JOBS *)arg;
	char* argList[]={task->command,task->args,NULL};
	pid_t pid;
	pid=fork();
	if(pid==0){
		snprintf(buf,sizeof(buf),"job_%d.out",task->place);
		if((fdout=open(buf,O_CREAT|O_APPEND|O_WRONLY,0755,task->place))==-1){
			printf("Error opening file %s for output\n",buf);
		}
		dup2(fdout,1);
		dup2(fdout,2);
		execvp(task->command,argList);
		snprintf(bufTwo,sizeof(buf),"job_%d.err",task->place);
		if ((fderr = open(buf, O_CREAT | O_APPEND | O_WRONLY, 0755,task->place)) == -1) {
		printf("Error opening file %s for error\n",bufTwo);
		exit(-1);
		}
		dup2(fderr,1);
		perror("exec");
	}

	return(NULL);
}
int main(int argc ,char **argv){
	int size=1000;
	JOBS *job= (JOBS *)malloc(sizeof(JOBS)*size);
	int i=0;
	if(argc<2){
		printf("Usage: %s <Max job count>\n",argv[0]);
		exit(-1);
	}else if(argc>2){
		printf("Too many arguments! Usage: %s <Max job count>\n",argv[0]);
	}
    queue* q;
	int max_jobs;
	sscanf(argv[1],"%d",&max_jobs);
	max_jobs=max_jobs+1;
	q=queue_init(100);
	int loop=0;
	while(loop==0){
		printf("Enter command: ");
		char *commandline[1000];
		commandline[0]=(char *)malloc(1000*sizeof(char*));
		commandline[1]=(char *)malloc(1000*sizeof(char*));
		commandline[2]=(char *)malloc(1000*sizeof(char*));
		char *store=(char*) malloc(10000*sizeof(char*));
		fgets(store,10000,stdin);
		char* delim=" ";
		commandline[0]=strdup(strtok(store,delim));
		commandline[1]=strdup(strtok(NULL,delim));
		commandline[2]=strdup(strtok(NULL,delim));
		if(strcmp(commandline[0],"showjobs")==0){
			queue_display(q);
		}else{
			if(strcmp(commandline[0],"submit")==0){
				job[i].place=(q->count)+1;
				job[i].command=commandline[1];
				job[i].args=commandline[2];
				queue_insert(q,q->count);
				pthread_create(&job[i].ptid, NULL,run_job,(void *)&job[i]);
				i++;
			}
		}
		
	}
    return 0;
}