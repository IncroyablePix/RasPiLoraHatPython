#ifndef LORADRIVER_SEMADD_H
#define LORADRIVER_SEMADD_H

union SemUnion
{
    int Val;
    struct semid_ds *Buf;
    unsigned short *Array;
    struct seminfo *m_Buf;
};

int TransferSemaphore(int key);
int CallSemaphore(int sid, int op);
void p(int semaphore);
void v(int semaphore);

#endif //LORADRIVER_SEMADD_H
