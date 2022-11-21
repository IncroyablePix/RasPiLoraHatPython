#ifndef NOALORAWRAPPER_WIRING_H
#define NOALORAWRAPPER_WIRING_H

#include <stdint-gcc.h>
#include "Sx.h"
#include "Bytes.h"
#include "Sf.h"

typedef enum
{
    LORA_STATE_UNKNOWN = -1,
    LORA_STATE_IDLE,
    LORA_STATE_RX,
    LORA_STATE_TX
} LoRaState;

typedef struct rpi_wiring Wiring;

Wiring* CreateWiring(uint32_t frequency, Sf spreadingFactor);
Wiring* CreateWiringFull(uint32_t frequency, Sf spreadingFactor,
                    int ssPin,
                    int dio0,
                    int reset,
                    int channel,
                    int regVersion,
                    Sx rfTransceiver);

void SetupWiring(Wiring* wiring);
void WriteTx(Wiring* wiring, Bytes* data);
void ToggleReceiver(int ssPin, int toggle);
char* ReadRx(Wiring* wiring);
void ToggleReceiver(int ssPin, int toggle);
void WriteRegistry(Wiring* wiring, byte addr, byte value);
byte ReadRegistry(Wiring* wiring, byte addr);
void Opmode(Wiring* wiring, uint8_t mode);
void SetLoRaOpMode(Wiring* wiring);
void WriteBuffer(Wiring* wiring, byte addr, Bytes* data);
int Receive(Wiring* wiring, Bytes *payload);
void StandBy(Wiring* wiring);
void SetReading(Wiring* wiring);
void SetWriting(Wiring* wiring);
void ConfigPower(Wiring* wiring, int8_t pw);
void SetPaRampUpTime(Wiring* wiring);
LoRaState GetWiringState(const Wiring* wiring);
int HasTransceiver(const Wiring* wiring);

#endif //NOALORAWRAPPER_WIRING_H
