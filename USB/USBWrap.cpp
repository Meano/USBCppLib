#include "USBDevice.h"
#include "InterfaceCCID.h"

USBDevice *USBBase;

extern "C" void USBSetup()
{
    USBBase = new USBDevice(USB0, 1, 0x12, 0x62, 0x01);
    USBBase->RegisterInterface(new InterfaceCCID(USBBase));
    USBBase->Connect();
}
