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

#ifndef USBDEVICE_TYPES_H
#define USBDEVICE_TYPES_H

#include "target.h"

/* Standard requests */
#define GET_STATUS        (0)
#define CLEAR_FEATURE     (1)
#define SET_FEATURE       (3)
#define SET_ADDRESS       (5)
#define GET_DESCRIPTOR    (6)
#define SET_DESCRIPTOR    (7)
#define GET_CONFIGURATION (8)
#define SET_CONFIGURATION (9)
#define GET_INTERFACE     (10)
#define SET_INTERFACE     (11)

/* bmRequestType.dataTransferDirection */
#define HOST_TO_DEVICE (0)
#define DEVICE_TO_HOST (1)

/* bmRequestType.Type*/
#define STANDARD_TYPE  (0)
#define CLASS_TYPE     (1)
#define VENDOR_TYPE    (2)
#define RESERVED_TYPE  (3)

/* bmRequestType.Recipient */
#define DEVICE_RECIPIENT    (0)
#define INTERFACE_RECIPIENT (1)
#define ENDPOINT_RECIPIENT  (2)
#define OTHER_RECIPIENT     (3)

/* Descriptors */
#define DESCRIPTOR_TYPE(wValue)  (wValue >> 8)
#define DESCRIPTOR_INDEX(wValue) (wValue & 0xff)

/* Standard descriptor types */
#define DEVICE_DESCRIPTOR        (1)
#define CONFIGURATION_DESCRIPTOR (2)
#define STRING_DESCRIPTOR        (3)
#define INTERFACE_DESCRIPTOR     (4)
#define ENDPOINT_DESCRIPTOR      (5)
#define QUALIFIER_DESCRIPTOR     (6)

/* Standard descriptor lengths */
#define DEVICE_DESCRIPTOR_LENGTH        (0x12)
#define CONFIGURATION_DESCRIPTOR_LENGTH (0x09)
#define INTERFACE_DESCRIPTOR_LENGTH     (0x09)
#define ENDPOINT_DESCRIPTOR_LENGTH      (0x07)


/*string offset*/
#define STRING_OFFSET_LANGID            (0)
#define STRING_OFFSET_IMANUFACTURER     (1)
#define STRING_OFFSET_IPRODUCT          (2)
#define STRING_OFFSET_ISERIAL           (3)
#define STRING_OFFSET_ICONFIGURATION    (4)
#define STRING_OFFSET_IINTERFACE        (5)

/* USB Specification Release Number */
#define USB_VERSION_2_0 (0x0200)

/* Least/Most significant byte of short integer */
#define LSB(n)  ((n)&0xff)
#define MSB(n)  (((n)&0xff00)>>8)

/* Convert physical endpoint number to descriptor endpoint number */
#define PHY_TO_DESC(endpoint) (((endpoint)>>1) | (((endpoint) & 1) ? 0x80:0))

/* bmAttributes in configuration descriptor */
/* C_RESERVED must always be set */
#define C_RESERVED      (1U<<7)
#define C_SELF_POWERED  (1U<<6)
#define C_REMOTE_WAKEUP (1U<<5)

/* bMaxPower in configuration descriptor */
#define C_POWER(mA)     ((mA)/2)

/* bmAttributes in endpoint descriptor */
#define E_CONTROL       (0x00)
#define E_ISOCHRONOUS   (0x01)
#define E_BULK          (0x02)
#define E_INTERRUPT     (0x03)

/* For isochronous endpoints only: */
#define E_NO_SYNCHRONIZATION    (0x00)
#define E_ASYNCHRONOUS          (0x04)
#define E_ADAPTIVE              (0x08)
#define E_SYNCHRONOUS           (0x0C)
#define E_DATA                  (0x00)
#define E_FEEDBACK              (0x10)
#define E_IMPLICIT_FEEDBACK     (0x20)

/* Device status */
#define DEVICE_STATUS_SELF_POWERED  (1U<<0)
#define DEVICE_STATUS_REMOTE_WAKEUP (1U<<1)

/* Endpoint status */
#define ENDPOINT_STATUS_HALT        (1U<<0)

/* Standard feature selectors */
#define DEVICE_REMOTE_WAKEUP        (1)
#define ENDPOINT_HALT               (0)

typedef struct {
    struct {
        uint8_t dataTransferDirection;
        uint8_t Type;
        uint8_t Recipient;
    } bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} SETUP_PACKET;

typedef struct {
    SETUP_PACKET setup;
    uint8_t *ptr;
    uint32_t remaining;
    uint8_t direction;
    bool zlp;
    bool notify;
} CONTROL_TRANSFER;

typedef enum {ATTACHED, POWERED, DEFAULT, ADDRESS, CONFIGURED} DEVICE_STATE;

typedef struct {
    volatile DEVICE_STATE state;
    uint8_t configuration;
    bool suspended;
} USB_DEVICE;

#endif
