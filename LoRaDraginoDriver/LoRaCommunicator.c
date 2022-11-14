#include <malloc.h>
#include <string.h>
#include <wiringPi.h>
#include "LoRaCommunicator.h"
#include "Wiring.h"

struct rpi_lora_communicator
{
    Wiring* LoRaWiring;
    LoRaRole Role;
    int Listening;
    union
    {
        void(*ReceiveHandler)(char* message, int length, void* handler);
        void(*StaticReceiveHandler)(char* message, int length);
    } OnReceive;
    void* MemberOf; // Generic pointer to pass to callback
};

LoRaCommunicator* InitLoRaCommunicator(uint32_t frequency, Sf spreadingFactor, LoRaRole role)
{
    LoRaCommunicator* loraCom = malloc(sizeof(LoRaCommunicator));
    loraCom->LoRaWiring = CreateWiring(frequency, spreadingFactor);
    loraCom->Role = role;

    SetupWiring(loraCom->LoRaWiring);

    if(role == LORA_ROLE_RECEIVER)
        SetReading(loraCom->LoRaWiring);
    else if(role == LORA_ROLE_SENDER)
        SetWriting(loraCom->LoRaWiring);


    return loraCom;
}

LoRaCommunicator* InitLoRaCommunicatorEx(Wiring* wiring, LoRaRole role)
{
    LoRaCommunicator* loraCom = malloc(sizeof(LoRaCommunicator));
    loraCom->LoRaWiring = wiring;
    loraCom->Role = role;

    SetupWiring(loraCom->LoRaWiring);

    return loraCom;
}

void SetStaticOnReceive(LoRaCommunicator* loraCom, void(*OnReceive)(char* message, int length))
{
    if(loraCom != NULL)
    {
        loraCom->OnReceive.StaticReceiveHandler = OnReceive;
    }
}

void SetOnReceive(LoRaCommunicator* loraCom, void(*OnReceive)(char* message, int length, void* extra), void* extra)
{
    if(loraCom != NULL)
    {
        loraCom->OnReceive.ReceiveHandler = OnReceive;
        loraCom->MemberOf = extra;
    }
}

LoRaState GetLoRaState(LoRaCommunicator* loraCom)
{
    if(loraCom != NULL)
        return GetWiringState(loraCom->LoRaWiring);

    return LORA_STATE_UNKNOWN;
}

void LoRaSend(LoRaCommunicator* loraCom, char* message)
{
    if(loraCom != NULL)
    {
        Bytes* bytes = malloc(sizeof(Bytes));
        bytes->bytes = (byte*)message;
        bytes->length = strlen(message);

        WriteTx(loraCom->LoRaWiring, bytes);

        free(bytes);
    }
}

void Stop(LoRaCommunicator* loraCom)
{
    if(loraCom != NULL)
    {
        loraCom->Listening = 0;
    }
}

void LoRaListen(LoRaCommunicator* loraCom)
{
    if(loraCom != NULL)
    {
        loraCom->Listening = 1;

        while(loraCom->Listening)
        {
            char* message = ReadRx(loraCom->LoRaWiring);
            if (message != NULL)
            {
                if (loraCom->MemberOf != NULL && loraCom->OnReceive.ReceiveHandler != NULL)
                    loraCom->OnReceive.ReceiveHandler(message, strlen(message), loraCom->MemberOf);
                else if (loraCom->OnReceive.StaticReceiveHandler != NULL)
                    loraCom->OnReceive.StaticReceiveHandler(message, strlen(message));

                free(message);
                message = NULL;
            }

            delay(1);
        }
    }
}
