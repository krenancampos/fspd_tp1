/******************************************************************************
*   A "hello world" Pthreads program which creates a large number of 
*   threads per process.  A sleep() call is used to ensure that all
*   threads are in existence at the same time.
******************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#define NTHREADS	8 /* 32 */

#define SOMETIME  ((int)(drand48()*10))

int turn = 0;

void *task(void *thread_param)
{
   int myid = *((int*)thread_param);
   int zzz = SOMETIME;
   printf("%d: Hello World! Vai dormir %d s\n",myid,zzz);
   sleep(zzz);
   printf("%d: saindo\n", myid);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    pthread_t threads[NTHREADS];
    int arg[NTHREADS];
    int rc; 
    int t;

    srand48(0);

    for(t=0;t<NTHREADS;t++){
        arg[t] = (int)t+1;
        rc = pthread_create(&threads[t], NULL, task, (void *)&arg[t]);
        if (rc){
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            printf("Code %d= %s\n",rc,strerror(rc));
            exit(-1);
        }
    }
    printf("main(): created %d threads.\n", t);
    for(t=0;t<NTHREADS;t++){
        pthread_join(threads[t],NULL);
    }
    printf("main(): all threads exited.\n");
    exit(0);
}
