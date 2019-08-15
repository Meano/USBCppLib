#ifndef USBINTERFACE_H
#define USBINTERFACE_H

#include "USBEndpoints.h"

typedef struct EPConfigStruct_s EPConfigStruct;
class USBDevice;

typedef struct InterfaceDescriptorStruct_s InterfaceDescriptorStruct;
typedef struct EndpointDescriptorStruct_s EndpointDescriptorStruct;

struct InterfaceDescriptorStruct_s {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
};

struct EndpointDescriptorStruct_s {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint8_t wMaxPacketSizeLSB;
    uint8_t wMaxPacketSizeMSB;
    uint8_t bInterval;
};

class USBInterface {
public:

    USBInterface(USBDevice *deviceBase, uint8_t interfaceIndex, uint8_t endpointCount, uint8_t interfaceClass, uint8_t interfaceSubClass, uint8_t interfaceProtocol);

    uint8_t *GetInterfaceDescriptor();
    uint32_t GetInterfaceDescriptorSize();
    void ConfigInit();

    virtual bool EndpointCallback(uint8_t epIndex, EPDirect epDirect, uint32_t * buffer, uint32_t bytesTransfer) { return false; };

    ~USBInterface();

    uint8_t EndpointCount;
    EPConfigStruct *EPConfig;

protected:
    bool RegisterEndpoint(uint8_t configIndex, uint8_t epIndex, EPDirect epDirect, EPType epType, uint32_t epMaxPacket, uint32_t epMaxBuffer, uint32_t *epBuffer);
    USBDevice *DeviceBase;
    uint8_t InterfaceClass;
    uint8_t InterfaceSubClass;
    uint8_t InterfaceProtocol;

    uint32_t DescriptorSize;
    uint8_t *Descriptor;

    uint32_t InterfaceExtendDescriptorSize;
    uint8_t * InterfaceExtendDescriptor;

private:
    uint8_t InterfaceIndex;

};


#endif
