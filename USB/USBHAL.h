#ifndef USBHAL_H
#define USBHAL_H

#include "target.h"
#include "USBEndpoints.h"
//#include "mhscpu_usb_hal.h"

typedef struct USBStruct_s USBStruct;
class USBHAL {
public:
    static USBHAL *Instance;
    static void StaticISR(void);

    USBHAL(USBStruct *usb);

bool InitEndpoint(uint8_t epIndex, EPDirect epDirect, EPConfigStruct *epConfig);

    void Reset(void);
    void Connect(void);
    void DisConnect(void);
    bool ActiveEndpoint(uint8_t epIndex, EPDirect epDirect);
    void SetAddress(uint8_t address);
    bool ConfigureDevice(void);
    void EndpointStall(uint8_t epIndex, EPDirect epDirect);

    bool EndpointStartRead(uint8_t epIndex, uint32_t * epBuffer, uint32_t bytesToRead);
    void EndpointProcessRead(uint8_t epIndex);
    void EndpointEndRead(uint8_t epIndex);

    bool EndpointStartWrite(uint8_t epIndex, uint32_t * epBuffer, uint32_t bytesToWrite);
    void EndpointProcessWrite(uint8_t epIndex);
    void EndpointEndWrite(uint8_t epIndex);

    ~USBHAL();
    void ISR();

protected:
    EPStruct EP[USB_ENDPOINT_COUNT][2];

private:
    USBStruct *USB;
    uint32_t UsedFifoSize;
    uint32_t TxFifoIndex;


}__align(4);
#endif
