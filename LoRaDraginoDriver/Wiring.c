/**
 * Shamelessly stolen from the Dragino LoRa example
 *
 * Adapted, cleaned up and extended by Benjamin W.
 */

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <malloc.h>
#include <stddef.h>
#include "Wiring.h"
#include "Sf.h"
#include "Bytes.h"

#define         MIN(a,b)                    ((a)<(b)?(a):(b))
#define         OPMODE_SLEEP                0x00

// FRF
#define        REG_FRF_MSB                  0x06
#define        REG_FRF_MID                  0x07
#define        REG_FRF_LSB                  0x08

#define        FRF_MSB                      0xD9 // 868.1 Mhz
#define        FRF_MID                      0x06
#define        FRF_LSB                      0x66

// REG
#define         REG_OPMODE                  0x01
#define         REG_FIFO                    0x00
#define         REG_SYNC_WORD			    0x39
#define         REG_MODEM_CONFIG            0x1D
#define         REG_MODEM_CONFIG2           0x1E
#define         REG_MODEM_CONFIG3           0x26
#define         REG_SYMB_TIMEOUT_LSB  	    0x1F
#define         REG_MAX_PAYLOAD_LENGTH 	    0x23
#define         REG_PAYLOAD_LENGTH          0x22
#define         PAYLOAD_LENGTH              0x40
#define         REG_VERSION	  				0x42
#define         REG_HOP_PERIOD              0x24
#define         REG_FIFO_ADDR_PTR           0x0D
#define         REG_FIFO_TX_BASE_AD         0x0E
#define         REG_FIFO_RX_BASE_AD         0x0F
#define         REG_LNA                     0x0C
#define         LNA_MAX_GAIN                0x23
#define         REG_RX_NB_BYTES             0x13
#define         REG_FIFO_RX_CURRENT_ADDR    0x10
#define         REG_IRQ_FLAGS               0x12
#define         REG_PKT_SNR_VALUE			0x19
#define         REG_IRQ_FLAGS_MASK          0x11

#define         REG_DIO_MAPPING_1           0x40 // common
#define         REG_DIO_MAPPING_2           0x41 // common

#define         RegPaConfig                 0x09 // common
#define         RegPaRamp                   0x0A // common
#define         RegPaDac                    0x5A // common

// DIO function mappings                         D0D1D2D3
#define         MAP_DIO0_LORA_RXDONE        0x00  // 00------
#define         MAP_DIO0_LORA_TXDONE        0x40  // 01------
#define         MAP_DIO1_LORA_RXTOUT        0x00  // --00----
#define         MAP_DIO1_LORA_NOP           0x30  // --11----
#define         MAP_DIO2_LORA_NOP           0xC0  // ----11--

// Constants for radio registers
#define         OPMODE_LORA                 0x80
#define         OPMODE_MASK                 0x07
#define         OPMODE_STANDBY              0x01
#define         OPMODE_FSTX                 0x02
#define         OPMODE_TX                   0x03
#define         OPMODE_FSRX                 0x04
#define         OPMODE_RX                   0x05
#define         OPMODE_RX_SINGLE            0x06
#define         OPMODE_CAD                  0x07


// Bits masking the corresponding IRQs from the radio
#define IRQ_LORA_RXTOUT_MASK 0x80
#define IRQ_LORA_RXDONE_MASK 0x40
#define IRQ_LORA_CRCERR_MASK 0x20
#define IRQ_LORA_HEADER_MASK 0x10
#define IRQ_LORA_TXDONE_MASK 0x08
#define IRQ_LORA_CDDONE_MASK 0x04
#define IRQ_LORA_FHSSCH_MASK 0x02
#define IRQ_LORA_CDDETD_MASK 0x01

struct rpi_wiring
{
    int SsPin;
    int Dio0Pin;
    int ResetPin;
    int Channel;
    byte RegVersion;
    Sf SpreadingFactor;
    uint32_t Frequency;
    Sx RfTransceiver;
    LoRaState State;
};

void SetupLoRaWiring(Wiring* wiring);

Wiring* CreateWiring(uint32_t frequency, Sf spreadingFactor)
{
    return CreateWiringFull(frequency, spreadingFactor, 6, 7, 0, 0, 0, SXNONE);
}

Wiring* CreateWiringFull(uint32_t frequency, Sf spreadingFactor,
            int ssPin,
            int dio0,
            int reset,
            int channel,
            int regVersion,
            Sx rfTransceiver)
{
    Wiring* wiring = (Wiring*) malloc(sizeof(Wiring));

    wiring->SsPin = ssPin;
    wiring->Dio0Pin = dio0;
    wiring->ResetPin = reset;
    wiring->Channel = channel;
    wiring->RegVersion = regVersion ? regVersion : REG_VERSION;
    wiring->SpreadingFactor = spreadingFactor;
    wiring->Frequency = frequency;
    wiring->RfTransceiver = rfTransceiver;
    wiring->State = LORA_STATE_UNKNOWN;

    return wiring;
}

void SetupWiring(Wiring* wiring)
{
    if(wiring == NULL)
        return;

    wiringPiSetup();
    pinMode(wiring->SsPin, OUTPUT);
    pinMode(wiring->Dio0Pin, INPUT);
    pinMode(wiring->ResetPin, OUTPUT);

    wiringPiSPISetup(wiring->Channel, 500000);

    SetupLoRaWiring(wiring);

    SetLoRaOpMode(wiring);
    StandBy(wiring); // enter standby mode (required for FIFO loading))
}

byte ReadRegistry(Wiring* wiring, byte addr)
{
    unsigned char spibuf[2];

    digitalWrite(wiring->SsPin, LOW);
    spibuf[0] = addr & 0x7F;
    spibuf[1] = 0x00;
    wiringPiSPIDataRW(wiring->Channel, spibuf, 2);
    digitalWrite(wiring->SsPin, HIGH);

    return spibuf[1];
}

void SetupLoRaWiring(Wiring* wiring)
{
    if(wiring == NULL)
        return;

    digitalWrite(wiring->ResetPin, HIGH);
    delay(100);
    digitalWrite(wiring->ResetPin, LOW);
    delay(100);

    byte version = ReadRegistry(wiring, wiring->RegVersion);

    if (version == 0x22)
    {
        // Is transceiver SX1272
        wiring->RfTransceiver = SX1272;
    }
    else
    {
        // ... or SX1276?
        digitalWrite(wiring->ResetPin, LOW);
        delay(100);
        digitalWrite(wiring->ResetPin, HIGH);
        delay(100);
        version = ReadRegistry(wiring, wiring->RegVersion);

        if (version == 0x12) // Yes
        {
            wiring->RfTransceiver = SX1276;
        }
        else // No, unrecognized transceiver!
        {
            wiring->RfTransceiver = SXNONE;
            return;
        }
    }

    Opmode(wiring, OPMODE_SLEEP);

    // Set frequency
    uint64_t frf = ((uint64_t)wiring->Frequency << 19) / 32000000;
    WriteRegistry(wiring, REG_FRF_MSB, (uint8_t) (frf >> 16));
    WriteRegistry(wiring, REG_FRF_MID, (uint8_t) (frf >> 8));
    WriteRegistry(wiring, REG_FRF_LSB, (uint8_t) (frf >> 0));

    WriteRegistry(wiring, REG_SYNC_WORD, 0x34); // LoRaWAN public sync word

    if (wiring->RfTransceiver == SX1272)
    {
        if (wiring->SpreadingFactor == SF11 || wiring->SpreadingFactor == SF12)
        {
            WriteRegistry(wiring, REG_MODEM_CONFIG, 0x0B);
        }
        else
        {
            WriteRegistry(wiring, REG_MODEM_CONFIG, 0x0A);
        }

        WriteRegistry(wiring, REG_MODEM_CONFIG2, (wiring->SpreadingFactor << 4) | 0x04);
    }
    else if(wiring->RfTransceiver == SX1276)
    {
        if (wiring->SpreadingFactor == SF11 || wiring->SpreadingFactor == SF12)
        {
            WriteRegistry(wiring, REG_MODEM_CONFIG3, 0x0C);
        }
        else
        {
            WriteRegistry(wiring, REG_MODEM_CONFIG3, 0x04);
        }
        WriteRegistry(wiring, REG_MODEM_CONFIG, 0x72);
        WriteRegistry(wiring, REG_MODEM_CONFIG2, (wiring->SpreadingFactor << 4) | 0x04);
    }

    if (wiring->SpreadingFactor == SF10 || wiring->SpreadingFactor == SF11 || wiring->SpreadingFactor == SF12)
    {
        WriteRegistry(wiring, REG_SYMB_TIMEOUT_LSB, 0x05);
    }
    else
    {
        WriteRegistry(wiring, REG_SYMB_TIMEOUT_LSB, 0x08);
    }

    WriteRegistry(wiring, REG_MAX_PAYLOAD_LENGTH, 0x80);
    WriteRegistry(wiring, REG_PAYLOAD_LENGTH, PAYLOAD_LENGTH);
    WriteRegistry(wiring, REG_HOP_PERIOD, 0xFF);
    WriteRegistry(wiring, REG_FIFO_ADDR_PTR, ReadRegistry(wiring, REG_FIFO_RX_BASE_AD));

    WriteRegistry(wiring, REG_LNA, LNA_MAX_GAIN);

    wiring->State = LORA_STATE_IDLE;
}

void WriteTx(Wiring* wiring, Bytes* data)
{
    if(wiring == NULL)
        return;

    WriteRegistry(wiring, REG_DIO_MAPPING_1,
                  MAP_DIO0_LORA_TXDONE |
                        MAP_DIO1_LORA_NOP |
                        MAP_DIO2_LORA_NOP
                        ); // IRQ mapping DIO0=TxDone DIO1=NOP DIO2=NOP
    WriteRegistry(wiring, REG_IRQ_FLAGS, 0xFF); // Remove all IRQ flags
    WriteRegistry(wiring, REG_IRQ_FLAGS_MASK, ~IRQ_LORA_TXDONE_MASK); // Enable ALL IRQs but TxDone

    // Address pointers
    WriteRegistry(wiring, REG_FIFO_TX_BASE_AD, 0x00);
    WriteRegistry(wiring, REG_FIFO_ADDR_PTR, 0x00);
    WriteRegistry(wiring, REG_PAYLOAD_LENGTH, data->length); // Send payload length

    WriteBuffer(wiring, REG_FIFO, data); // Write all bytes to the radio FIFO

    Opmode(wiring, OPMODE_TX); // Start transmission (OPMODE_TX)
}

char* ReadRx(Wiring* wiring)
{
    long int signalToNoiseRatio;            // Future purpose
    int receivedSignalStrengthIndication;   // Future purpose

    Bytes *bytes = (Bytes *)malloc(sizeof(Bytes));
    bytes->length = 0;
    bytes->bytes = (byte *)malloc(sizeof(byte) * 256);

    if(digitalRead(wiring->Dio0Pin) == 1)
    {
        if(Receive(wiring, bytes))
        {
            // Currently unused
            byte value = ReadRegistry(wiring, REG_PKT_SNR_VALUE);
            if(value & 0x80) // If the SNR sign bit is 1
                signalToNoiseRatio = -(((~value + 1) & 0xFF ) >> 2); // Invert and divide by 4
            else
                signalToNoiseRatio = (value & 0xFF ) >> 2; // Divide by 4

            receivedSignalStrengthIndication = wiring->RfTransceiver == SX1272 ? 139 : 157;
        }
    }

    char* data = NULL;
    if(bytes->length > 0)
        data = (char*) bytes->bytes;
    else
        free(bytes->bytes);

    free(bytes);

    return data;
}

int Receive(Wiring* wiring, Bytes *payload)
{
    if(wiring == NULL)
    {
        payload->length = 0;
        payload->bytes[0] = 0;
        return 0;
    }

    WriteRegistry(wiring, REG_IRQ_FLAGS, 0x40);

    int irqflags = ReadRegistry(wiring, REG_IRQ_FLAGS);

    // Payload CRC: 0x20
    if((irqflags & 0x20) == 0x20)
    {
        // CRC error!
        WriteRegistry(wiring, REG_IRQ_FLAGS, 0x20);
        return -1;
    }

    byte currentAddr = ReadRegistry(wiring, REG_FIFO_RX_CURRENT_ADDR);
    byte receivedCount = ReadRegistry(wiring, REG_RX_NB_BYTES);
    payload->length = MIN(receivedCount, UINT8_MAX -1);

    WriteRegistry(wiring, REG_FIFO_ADDR_PTR, currentAddr);

    for(int i = 0; i < receivedCount; i++)
        payload->bytes[i] = (char) ReadRegistry(wiring, REG_FIFO);

    payload->bytes[payload->length] = '\0';

    return receivedCount;
}

void WriteRegistry(Wiring* wiring, byte addr, byte value)
{
    if(wiring == NULL)
        return;

    unsigned char spibuf[2];

    spibuf[0] = addr | 0x80;
    spibuf[1] = value;

    digitalWrite(wiring->SsPin, LOW);
    wiringPiSPIDataRW(wiring->Channel, spibuf, 2);
    digitalWrite(wiring->SsPin, HIGH);
}

void Opmode(Wiring* wiring, uint8_t mode)
{
    if(wiring == NULL)
        return;

    WriteRegistry(wiring, REG_OPMODE, (ReadRegistry(wiring, REG_OPMODE) & ~OPMODE_MASK) | mode);
}

void WriteBuffer(Wiring* wiring, byte addr, Bytes* data)
{
    if(wiring == NULL)
        return;

    byte spiBuffer[256];
    spiBuffer[0] = addr | 0x80;

    for (int i = 0; i < data->length; i++)
        spiBuffer[i + 1] = data->bytes[i];

    ToggleReceiver(wiring->SsPin, 1);
    wiringPiSPIDataRW(wiring->Channel, spiBuffer, data->length + 1);
    ToggleReceiver(wiring->SsPin, 0);
}

void SetLoRaOpMode(Wiring* wiring)
{
    if(wiring == NULL)
        return;

    uint8_t u = OPMODE_LORA;
    if (wiring->RfTransceiver != SX1272)
        u |= 0x8;   // TBD: SX1276 high freq

    WriteRegistry(wiring, REG_OPMODE, u);
}

void ToggleReceiver(int ssPin, int toggle)
{
    digitalWrite(ssPin, toggle ? HIGH : LOW);
}

void StandBy(Wiring* wiring)
{
    Opmode(wiring, OPMODE_STANDBY);
}

void SetReading(Wiring* wiring)
{
    if(wiring == NULL)
        return;

    wiring->State = LORA_STATE_RX;
    Opmode(wiring, OPMODE_RX);
}

void SetWriting(Wiring* wiring)
{
    if(wiring == NULL)
        return;

    SetPaRampUpTime(wiring);
    ConfigPower(wiring, 23);
}

void ConfigPower(Wiring* wiring, int8_t pw)
{
    if(wiring == NULL)
        return;

    if (wiring->RfTransceiver != SX1272) // SX1276
    {
        if(pw >= 17)
            pw = 15;
        else if(pw < 2)
            pw = 2;

        // Check board type for BOOST pin
        WriteRegistry(wiring, RegPaConfig, (uint8_t) (0x80 | (pw & 0xf)));
        WriteRegistry(wiring, RegPaDac, ReadRegistry(wiring, RegPaDac) | 0x4);
    }
    else
    {
        // set PA config (2-17 dBm using PA_BOOST)
        if(pw > 17)
            pw = 17;
        else if(pw < 2)
            pw = 2;

        WriteRegistry(wiring, RegPaConfig, (uint8_t) (0x80 | (pw - 2)));
    }

    wiring->State = LORA_STATE_TX;
}

void SetPaRampUpTime(Wiring* wiring)
{
    WriteRegistry(wiring, RegPaRamp, (ReadRegistry(wiring, RegPaRamp) & 0xF0) | 0x08); // Set PA ramp-up time 50 uSec
}

LoRaState GetWiringState(const Wiring* wiring)
{
    if(wiring == NULL)
        return LORA_STATE_UNKNOWN;

    return wiring->State;
}
