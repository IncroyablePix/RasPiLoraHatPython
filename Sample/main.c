#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Wiring.h>
#include <Sf.h>
#include <Bytes.h>
#include <LoRaCommunicator.h>
#include <pthread.h>

int count = 0;

void OnReceive(char* message, int length, void* extra)
{
    LoRaCommunicator* loraCom = (LoRaCommunicator*)extra;
    printf("Received: %s\n", message);

    if(++count == 3)
        StopLoRaListen(loraCom);
}

int main (int argc, char** argv)
{
    Sf sf = SF7;
    uint32_t freq = 868000000;

    LoRaRole role = !strcmp(argv[1], "send") ? LORA_ROLE_SENDER : LORA_ROLE_RECEIVER;
    LoRaCommunicator* loraCom = InitLoRaCommunicator(freq, sf, role);
    SetOnReceive(loraCom, OnReceive, loraCom);

    if(role == LORA_ROLE_SENDER)
    {
        LoRaSend(loraCom, "Hello World!");
    }
    else if(role == LORA_ROLE_RECEIVER)
    {
        pthread_t thread = LoRaListenThread(loraCom);
        pthread_join(thread, NULL);
    }

    return EXIT_SUCCESS;
}
