#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<pthread.h>
#include<sys/types.h>

#define BUFFER_LENGTH 256 

pthread_mutex_t mutex;
int fd;

int rowA,columnA;
int rowB,columnB;

int threadCount;

int **A;
int **B;
int **C;

int first = 0;

void *createThread(void* arg)
{
	int *data = (int*)arg;
	int jobNum = *data;

	int start = -1;
	int end = -1;

	int totalCell = rowA*columnB;

	if(threadCount > totalCell)//some thread don't need to calculate, since the matrix is too small
	{
		if(jobNum < totalCell)
		{
			start = jobNum;
			end = jobNum;
		}
	}
	else//every thread will get at least one cell to calculate
	{
		if(jobNum < totalCell%threadCount)//need one additional cell 
		{
			start = jobNum*((totalCell/threadCount)+1);
			end = start+(totalCell/threadCount);
		}
		else
		{
			start = (totalCell%threadCount)*((totalCell/threadCount)+1);
			start += (jobNum-(totalCell%threadCount)) * (totalCell/threadCount);
			end = start+(totalCell/threadCount)-1;
		}
	}

	if(start != -1 && end != -1)//have job
	{
		for(int i = start; i <= end; i++)// i = the number of the cell
		{
			int r = i/columnB;
			int c = i%columnB;

			int sum = 0;

			for(int j = 0; j < columnA; j++)
			{
				sum += A[r][j]*B[j][c];
			}

			C[r][c] = sum;
		}
	}


	//job done 
	pthread_mutex_lock(&mutex);

	char str[20] = {};
	
	if(first == 0)
	{
		strcpy(str,"clear");
		first = 1;
	}
	else
	{
		strcpy(str,"done");
	}

	write(fd, str, strlen(str)); // Send the string to the LKM

	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
	
}

int main(int argc, char *argv[])
{
	pthread_t t[100];
	char stringToSend[BUFFER_LENGTH];

	pthread_mutex_init(&mutex,0);

	fd = open("/proc/thread_info", O_RDWR);   

	FILE *in1;
	FILE *in2;

	in1 = fopen(argv[2],"r");
	fscanf(in1,"%d %d",&rowA,&columnA);
	A = (int**)malloc(sizeof(int*)*rowA);
	for(int i = 0; i < rowA; i++)
	{
		A[i] = (int*)malloc(sizeof(int)*columnA);
	}
	for(int i = 0; i < rowA; i++)
	{
		for(int j = 0; j < columnA; j++)
		{
			fscanf(in1, "%d", &A[i][j]);
		}
	}
	fclose(in1);
	
	in2 = fopen(argv[3],"r");
	fscanf(in2,"%d %d",&rowB,&columnB);
	B = (int**)malloc(sizeof(int*)*rowB);
	for(int i = 0; i < rowB; i++)
	{
		B[i] = (int*)malloc(sizeof(int)*columnB);
	}
	for(int i = 0; i < rowB; i++)
	{
		for(int j = 0; j < columnB; j++)
		{
			fscanf(in2, "%d", &B[i][j]);
		}
	}
	fclose(in2);

	threadCount = atoi(argv[1]);

	C = (int**)malloc(sizeof(int*)*rowA);
	for(int i = 0; i < rowA; i++)
	{
		C[i] = (int*)malloc(sizeof(int)*columnB);
	}
	
	
	for(int i = 0; i < threadCount; i++)
	{
		int *num = (int*)malloc(sizeof(int));
		*num = i;
		pthread_create(&t[i],NULL,createThread,(void*)num);
	}

	for(int i = 0; i < threadCount; i++)
	{
		pthread_join(t[i],NULL);
	}         

	pthread_mutex_destroy(&mutex); 

	printf("PID:%d\n",getpid());

	char result[16384];
	read(fd,result,16384);

	printf("%s",result);


	FILE *fout = fopen("output.txt", "w");

	fprintf(fout,"%d %d\n", rowA, columnB);
	for(int i = 0; i < rowA; i++)
	{
		for(int j = 0; j < columnB; j++)
		{
			fprintf(fout,"%d ", C[i][j]);
		}
		fprintf(fout,"\n");
	}
	
	fclose(fout);
	
	return 0;

}
