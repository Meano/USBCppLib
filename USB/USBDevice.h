/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "target.h"
#include "USBHAL.h"
#include "USBDevice_Types.h"
#include "InterfaceSetup.h"
#include "mhscpu_usb_hal.h"

class InterfaceSetup;

class USBDevice : public USBHAL
{
public:
    USBDevice(USBStruct *usb, uint32_t interfaceCount, uint16_t vendorID, uint16_t productID, uint16_t productRelease);

    bool configured(void);
    void connect(bool blocking = true);
    void disconnect(void);
    bool addEndpoint(uint8_t endpoint, uint32_t maxPacket);
    bool readStart(uint8_t endpoint, uint32_t maxSize);
    bool readEP(uint8_t endpoint, uint8_t * buffer, uint32_t * size, uint32_t maxSize);
    bool readEP_NB(uint8_t endpoint, uint8_t * buffer, uint32_t * size, uint32_t maxSize);
    bool write(uint8_t endpoint, uint8_t * buffer, uint32_t size, uint32_t maxSize);
    bool writeNB(uint8_t endpoint, uint8_t * buffer, uint32_t size, uint32_t maxSize);


    virtual void USBCallback_busReset(void) {};
    virtual bool USBCallback_request() { return false; };
    virtual void USBCallback_requestCompleted(uint8_t * buf, uint32_t length) {};
    virtual bool USBCallback_setConfiguration(uint8_t configuration) { return false; };
    virtual bool USBCallback_setInterface(uint16_t interface, uint8_t alternate) { return false; };


    virtual const uint8_t * deviceDesc();
    virtual const uint8_t * configurationDesc();
    virtual const uint8_t * stringLangidDesc();
    virtual const uint8_t * stringImanufacturerDesc();
    virtual const uint8_t * stringIproductDesc();
    virtual const uint8_t * stringIserialDesc();
    virtual const uint8_t * stringIConfigurationDesc();
    virtual const uint8_t * stringIinterfaceDesc();
    virtual uint16_t reportDescLength() { return 0; };

    ~USBDevice();

protected:
    virtual void busReset(void);
    virtual void connectStateChanged(unsigned int connected);
    virtual void suspendStateChanged(unsigned int suspended);
    uint8_t * findDescriptor(uint8_t descriptorType);

    uint16_t VENDOR_ID;
    uint16_t PRODUCT_ID;
    uint16_t PRODUCT_RELEASE;
    uint32_t deviceDescriptor[64];

    uint32_t InterfaceCount;
    uint32_t RegisteredInterface;
    USBInterface **Interface;

    uint32_t ConfigurationDescriptorSize;
    uint32_t * ConfigurationDescriptor;

    InterfaceSetup *ControlInterface;

public:
    bool RegisterInterface(USBInterface *interface);

    bool RequestGetDescriptor(CONTROL_TRANSFER *transfer);
    bool RequestSetAddress(CONTROL_TRANSFER *transfer);
    bool RequestSetConfiguration(CONTROL_TRANSFER *transfer);
    bool RequestGetConfiguration(CONTROL_TRANSFER *transfer);
    bool RequestGetInterface(CONTROL_TRANSFER *transfer);
    bool RequestSetInterface(CONTROL_TRANSFER *transfer);
    bool RequestSetFeature(CONTROL_TRANSFER *transfer);
    bool RequestClearFeature(CONTROL_TRANSFER *transfer);
    bool RequestGetStatus(CONTROL_TRANSFER *transfer);
    bool requestGetInterface(CONTROL_TRANSFER *transfer);
    bool requestSetInterface(CONTROL_TRANSFER *transfer);

    //CONTROL_TRANSFER transfer;
    USB_DEVICE device;

    uint16_t currentInterface;
    uint8_t currentAlternate;
private:
    bool RegisterInterfaceEndpoint(USBInterface *interface);
};


#endif
