#include "logger.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
#include "stdbool.h"
#include <stdlib.h>

static char pathFile[100];
int loggerLevel[MAXLOGLEVEL];
pthread_t my_thread;
static bool endLog = 0;
static sem_t sema_msg_start, sema_msg_write;
char logBuf[512];
FILE *log_file;
pthread_mutex_t mutexLog;

void setLevel(int *lvl)
{
    for (int i=0; i<MAXLOGLEVEL; i++)
        loggerLevel[i] = lvl[i];
}

void setPath(char *buf)
{
    strcpy(pathFile, buf);
}

const char* logLevelAsString(int level)
{
    switch (level)
    {
        case ll_debug:
            return "debug  ";
        case ll_warning:
            return "warning";
        case ll_error:
            return "error  ";
        case ll_message:
            return "message";
        default:
            return "unknown";
    }
}

void writeLog(int lvl, char * mess, const char* func, char *line)
{
    if (loggerLevel[(lvl-1)] == 0) return;

    sem_wait(&sema_msg_write);
    pthread_mutex_lock( &mutexLog );

    sprintf(logBuf, " %s (%s : %s ): %s \n",
             logLevelAsString(lvl), func, line, mess);

    pthread_mutex_unlock( &mutexLog );
    sem_post(&sema_msg_start);
}

void* threads_Logger()
{
    printf("start\n");
    FILE *log_file = fopen(pathFile, "w");

    if(log_file == NULL)
    {
        printf("log_file isn't created\n");
        exit(1);
    }
    printf("log_file created\n");
    fprintf(log_file, "------------------------------------------------------\n");
    fprintf(log_file, "Program compiled on %s at %s\n", __DATE__ , __TIME__);
    for (int i=0; i<MAXLOGLEVEL; i++)
        if (loggerLevel[i] != 0)
            fprintf(log_file, " logLevel Logger = %s\n", logLevelAsString(i+1));
    fprintf(log_file, "------------------------------------------------------\n");
    fflush(log_file);

    char fileName[100];
    sprintf (fileName,"%s:%s", __FILENAME__, __FUNCTION__);
    time_t cur_time;
    struct tm *dt;
    time(&cur_time);
    dt = localtime(&cur_time);
    char logBuff[512];
    sprintf(logBuff, " %02d:%02d:%02d %s(%s): \n	>>> Logger Thread created >>>\n",
             dt->tm_hour, dt->tm_min, dt->tm_sec, pathFile, fileName);
    fprintf(log_file, "%s", logBuff);
    fprintf(log_file, "------------------------------------------------------\n");
    fflush(log_file);

    while(endLog == 0)
    {
        sem_post(&sema_msg_write);
        sem_wait(&sema_msg_start);
        pthread_mutex_lock( &mutexLog );
        fprintf(log_file, "%s", logBuf);
        memset(logBuf, 0, sizeof(logBuf));
        pthread_mutex_unlock( &mutexLog );
        fflush(log_file);
    }
    fprintf(log_file, "\n		>>>log Thread closed>>>\n");
    fflush(log_file);
    fclose(log_file);
    sem_destroy(&sema_msg_start);

    return NULL;
}


int logger()
{
    sem_init(&sema_msg_start, 0, 0);
    sem_init(&sema_msg_write, 0, 0);
    pthread_mutex_init(&mutexLog, NULL);
    if (pthread_create(&my_thread, NULL, threads_Logger, NULL) !=0)
    {
        printf("error\n");
        exit(1);
    }
    return 0;
}

void stopLogThread()
{
    endLog = 1;
    sem_post(&sema_msg_start);
}

