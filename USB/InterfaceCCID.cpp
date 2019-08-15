#include "InterfaceCCID.h"
#include "USBDevice.h"

InterfaceCCID::InterfaceCCID(USBDevice *deviceBase) : USBInterface(deviceBase, 0, 3, 0x0B, 0x00, 0x00)
{
    InterfaceExtendDescriptorSize = 0x36;
    uint8_t temp[0x36] =  {
        0x36, 0x21, 0x10, 0x01, 0x00, 0x03, 0x01, 0x00,
        0x00, 0x00, 0x10, 0x0E, 0x00, 0x00, 0x10, 0x0E,
        0x00, 0x00, 0x00, 0xCD, 0x25, 0x00, 0x00, 0xCD, 
        0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x38, 0x00, 0x01, 0x00, 0x0F, 0x01, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    };
    InterfaceExtendDescriptor = new uint8_t[0x36 + 2];
    memcpy(InterfaceExtendDescriptor, temp, InterfaceExtendDescriptorSize);
    ConfigInit();
    RegisterEndpoint(EP1ConfigIn,  1, EPDirectIn,  EPTypeControl,   64, 64, BlkInBuffer );
    RegisterEndpoint(EP1ConfigOut, 1, EPDirectOut, EPTypeControl,   64, 64, BlkOutBuffer);
    RegisterEndpoint(EP2ConfigIn,  2, EPDirectIn,  EPTypeInterrupt, 64, 64, BlkOutBuffer);
}

bool InterfaceCCID::EndpointCallback(uint8_t epIndex, EPDirect epDirect, uint32_t * buffer, uint32_t bytesTransfer)
{
    if(epIndex == 1) {
        if(epDirect == EPDirectIn) {
            //return SetupCallbackIn(buffer, bytesTransfer);
        }
        else {
            //return SetupCallbackOut(buffer, bytesTransfer);
        }
    }
    else if(epIndex == 2) {
        if(epDirect == EPDirectIn) {
            //return SetupCallbackIn(buffer, bytesTransfer);
        }
    }
    return false;
}

InterfaceCCID::~InterfaceCCID()
{

}
