#include "USBInterface.h"
#include "USBDevice.h"

USBInterface::USBInterface(USBDevice *deviceBase, uint8_t interfaceIndex, uint8_t endpointCount, uint8_t interfaceClass, uint8_t interfaceSubClass, uint8_t interfaceProtocol)
{
    EndpointCount = endpointCount;
    DeviceBase = deviceBase;
    InterfaceIndex = interfaceIndex;
    EPConfig = new EPConfigStruct[endpointCount];
    InterfaceClass = interfaceClass;
    InterfaceSubClass = interfaceSubClass;
    InterfaceProtocol = interfaceProtocol;
//    ConfigInit();
}

void USBInterface::ConfigInit(){
    DescriptorSize = 9 + EndpointCount * 7 + InterfaceExtendDescriptorSize;

    Descriptor = new uint8_t[DescriptorSize];

    InterfaceDescriptorStruct * interfaceDescriptor = (InterfaceDescriptorStruct *)Descriptor;
    interfaceDescriptor->bLength = 9;
    interfaceDescriptor->bDescriptorType = 4;
    interfaceDescriptor->bInterfaceNumber = InterfaceIndex;
    interfaceDescriptor->bAlternateSetting = 0;
    interfaceDescriptor->bNumEndpoints = EndpointCount;
    interfaceDescriptor->bInterfaceClass = InterfaceClass;
    interfaceDescriptor->bInterfaceSubClass = InterfaceSubClass;
    interfaceDescriptor->bInterfaceProtocol = InterfaceProtocol;
    interfaceDescriptor->iInterface = 5;
    if(InterfaceExtendDescriptorSize != 0) {
        memcpy(Descriptor + 9, InterfaceExtendDescriptor, InterfaceExtendDescriptorSize);
    }
}

bool USBInterface::RegisterEndpoint(uint8_t configIndex, uint8_t epIndex, EPDirect epDirect, EPType epType, uint32_t epMaxPacket, uint32_t epMaxBuffer, uint32_t *epBuffer)
{
    if(configIndex >= EndpointCount)
        return false;
    EPConfigStruct *epConfig = &EPConfig[configIndex];
    epConfig->Index = epIndex;
    epConfig->Direct = epDirect;
    epConfig->Type = epType;
    epConfig->MaxPacket = epMaxPacket;
    epConfig->MaxBuffer = epMaxBuffer;
    epConfig->Buffer = epBuffer;
    epConfig->Interface = this;
    EndpointDescriptorStruct * endpointDescriptor = (EndpointDescriptorStruct *)(Descriptor + 9 + InterfaceExtendDescriptorSize + 7 * configIndex);
    endpointDescriptor->bLength = 7;
    endpointDescriptor->bDescriptorType = 5;
    endpointDescriptor->bEndpointAddress = epIndex | (epDirect << 7);
    endpointDescriptor->bmAttributes = epType;
    endpointDescriptor->wMaxPacketSizeLSB = LSB(epConfig->MaxPacket);
    endpointDescriptor->wMaxPacketSizeMSB = MSB(epConfig->MaxPacket);
    endpointDescriptor->bInterval = epType == EPTypeInterrupt ? 1 : 0;
    return true;
}

uint32_t USBInterface::GetInterfaceDescriptorSize()
{
    return DescriptorSize;
}

uint8_t *USBInterface::GetInterfaceDescriptor()
{
    return Descriptor;
}

USBInterface::~USBInterface()
{
    delete [] EPConfig;
    delete [] Descriptor;
}
