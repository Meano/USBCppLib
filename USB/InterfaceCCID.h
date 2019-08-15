#ifndef INTERFACECCID_H
#define INTERFACECCID_H

#include "USBInterface.h"
#include "USBDevice_Types.h"

#define EP1ConfigIn             0
#define EP1ConfigOut            1
#define EP2ConfigIn             2

#define MIN(a,b)                ((a) < (b) ? (a) : (b))
#define MAX(a,b)                ((a) > (b) ? (a) : (b))

class InterfaceCCID : public USBInterface
{
public:
    InterfaceCCID(USBDevice *deviceBase);

    virtual bool EndpointCallback(uint8_t epIndex, EPDirect epDirect, uint32_t * buffer, uint32_t bytesTransfer);

    ~InterfaceCCID();

private:
    uint32_t BlkInBuffer[64 >> 2];
    uint32_t BlkOutBuffer[64 >> 2];
    uint32_t InterruptBuffer[64 >> 2];
};

#endif
