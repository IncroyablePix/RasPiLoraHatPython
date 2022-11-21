#ifndef NOALORAWRAPPER_LORACOMMUNICATOR_H
#define NOALORAWRAPPER_LORACOMMUNICATOR_H

#include "Sf.h"
#include "Wiring.h"

typedef struct rpi_lora_communicator LoRaCommunicator;

typedef enum
{
    LORA_ROLE_UNKNOWN = -1,
    LORA_ROLE_SENDER,
    LORA_ROLE_RECEIVER
} LoRaRole;

LoRaCommunicator* InitLoRaCommunicator(uint32_t frequency, Sf spreadingFactor, LoRaRole role);
LoRaCommunicator* InitLoRaCommunicatorEx(Wiring* wiring, LoRaRole role);
void SetStaticOnReceive(LoRaCommunicator* loraCom, void(*OnReceive)(char* message, int length));
void SetOnReceive(LoRaCommunicator* loraCom, void(*OnReceive)(char* message, int length, void* extra), void* extra);
LoRaState GetLoRaState(LoRaCommunicator* loraCom);
void LoRaSend(LoRaCommunicator* loraCom, char* message);
void StopLoRaListen(LoRaCommunicator* loraCom);
pthread_t LoRaListenThread(LoRaCommunicator* loraCom);
void LoRaListen(LoRaCommunicator* loraCom);
int HasLoRaTransceiver(const LoRaCommunicator* loraCom);

#endif //NOALORAWRAPPER_LORACOMMUNICATOR_H
