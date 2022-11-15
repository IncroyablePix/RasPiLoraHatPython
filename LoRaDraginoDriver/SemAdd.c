#include "SemAdd.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

int TransferSemaphore(int key)
{
    int sid = semget(key, 1, 0666 | IPC_CREAT);

    if (sid == -1)
    {
        perror("semget");
        exit(-1);
    }

    return sid;
}

int CallSemaphore(int sid, int op)
{
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = op;
    sb.sem_flg = 0;

    if (semop(sid, &sb, 1) == -1)
    {
        perror("semop");
        exit(-2);
    }

    return 0;
}

void p(int semaphore)
{
    CallSemaphore(semaphore, -1);
}

void v(int semaphore)
{
    CallSemaphore(semaphore, 1);
}
