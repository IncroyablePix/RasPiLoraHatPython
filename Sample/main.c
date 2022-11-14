#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <Wiring.h>
#include <Sf.h>
#include <Bytes.h>
#include <LoRaCommunicator.h>

void OnReceive(char* message, int length, void* extra)
{
    LoRaCommunicator* loraCom = (LoRaCommunicator*)extra;
    printf("Received: %s\n", message);
    Stop(loraCom);
}

int main (int argc, char *argv[])
{
    if (argc < 2)
    {
        printf ("Usage: argv[0] (send|receive) [message]\n");
        exit(1);
    }

    byte hello[] = "Hello, World!";
    Sf sf = SF7;
    uint32_t freq = 868000000;

    LoRaRole role = !strcmp(argv[1], "send") ? LORA_ROLE_SENDER : LORA_ROLE_RECEIVER;
    LoRaCommunicator* loraCom = InitLoRaCommunicator(freq, sf, role);
    SetOnReceive(loraCom, OnReceive, loraCom);

    if(GetLoRaState(loraCom) == LORA_STATE_UNKNOWN)
        return 1;

    if(role == LORA_ROLE_SENDER)
    {
        LoRaSend(loraCom, (argc > 2 ? argv[2] : hello)); // Send once!
    }
    else if (role == LORA_ROLE_RECEIVER)
    {
        LoRaListen(loraCom); // Loops -> To stop the loop, you need to "Stop(loraCom);" or kill the process.
    }

    return EXIT_SUCCESS;
}
