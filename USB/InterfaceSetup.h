#ifndef INTERFACESETUP_H
#define INTERFACESETUP_H

#include "USBInterface.h"
#include "USBDevice_Types.h"

#define EP0ConfigIn             0
#define EP0ConfigOut            1

#define MIN(a,b)                ((a) < (b) ? (a) : (b))
#define MAX(a,b)                ((a) > (b) ? (a) : (b))

class InterfaceSetup : public USBInterface
{
public:
    InterfaceSetup(USBDevice *deviceBase);

    virtual bool EndpointCallback(uint8_t epIndex, EPDirect epDirect, uint32_t * buffer, uint32_t bytesTransfer);

    ~InterfaceSetup();

private:
    CONTROL_TRANSFER transfer;
    EPConfigStruct * iEPConfig;
    EPConfigStruct * oEPConfig;
    uint32_t SetupBufferIn[64];
    uint32_t SetupBufferOut[64];

    void DecodeSetupPacket(uint8_t *data, SETUP_PACKET *packet);
    bool RequestSetup(void);

    bool SetupCallbackIn(uint32_t * buffer, uint32_t byteWritten);
    bool SetupCallbackOut(uint32_t * buffer, uint32_t byteWritten);

};

#endif
