#include "USBDevice.h"
#include "InterfaceSetup.h"
USBDevice::USBDevice(USBStruct *usb, uint32_t interfaceCount, uint16_t vendorID, uint16_t productID, uint16_t productRelease) : USBHAL(usb)
{
    VENDOR_ID = vendorID;
    PRODUCT_ID = productID;
    PRODUCT_RELEASE = productRelease;
    InterfaceCount = interfaceCount;

    ControlInterface = new InterfaceSetup(this);
    RegisterInterfaceEndpoint(ControlInterface);

    Interface = new USBInterface *[interfaceCount];
    RegisteredInterface = 0;

    ConfigurationDescriptorSize = 9;

    /* Set initial device state */
    device.state = POWERED;
    device.configuration = 0;
    device.suspended = false;
}

bool USBDevice::RegisterInterface(USBInterface *interface)
{
    if(!RegisterInterfaceEndpoint(interface))
        return false;
    Interface[RegisteredInterface++] = interface;
    ConfigurationDescriptorSize += interface->GetInterfaceDescriptorSize();
    return true;
}

bool USBDevice::RegisterInterfaceEndpoint(USBInterface *interface) {
    for(int i = 0; i < interface->EndpointCount; i++) {
        EPConfigStruct *epConfig = &interface->EPConfig[i];
        if(EP[epConfig->Index][epConfig->Direct].IsConfigured) {
            printf_dbg("Endpoint %d %d has Contigured!", epConfig->Index, epConfig->Direct);
            return false;
        }
        EP[epConfig->Index][epConfig->Direct].Config = epConfig;
        EP[epConfig->Index][epConfig->Direct].IsConfigured = true;
    }
    return true;
}

bool USBDevice::RequestGetDescriptor(CONTROL_TRANSFER *transfer)
{
    bool success = false;
    printf_dbg("get descr: type: %d\r\n", DESCRIPTOR_TYPE(transfer->setup.wValue));
    switch (DESCRIPTOR_TYPE(transfer->setup.wValue))
    {
        case DEVICE_DESCRIPTOR:
            if (deviceDesc() != NULL)
            {
                if ((deviceDesc()[0] == DEVICE_DESCRIPTOR_LENGTH) \
                    && (deviceDesc()[1] == DEVICE_DESCRIPTOR))
                {
                    printf_dbg("device descr\r\n");
                    transfer->remaining = DEVICE_DESCRIPTOR_LENGTH;
                    transfer->ptr = (uint8_t*)deviceDesc();
                    transfer->direction = DEVICE_TO_HOST;
                    success = true;
                }
            }
            break;
        case CONFIGURATION_DESCRIPTOR:
            if (configurationDesc() != NULL)
            {
                if ((configurationDesc()[0] == CONFIGURATION_DESCRIPTOR_LENGTH) \
                    && (configurationDesc()[1] == CONFIGURATION_DESCRIPTOR))
                {
                    printf_dbg("conf descr request\r\n");
                    /* Get wTotalLength */
                    transfer->remaining = configurationDesc()[2] \
                        | (configurationDesc()[3] << 8);

                    transfer->ptr = (uint8_t*)configurationDesc();
                    transfer->direction = DEVICE_TO_HOST;
                    success = true;
                }
            }
            break;
        case STRING_DESCRIPTOR:
            printf_dbg("str descriptor\r\n");
            switch (DESCRIPTOR_INDEX(transfer->setup.wValue))
            {
                case STRING_OFFSET_LANGID:
                    printf_dbg("1\r\n");
                    transfer->remaining = stringLangidDesc()[0];
                    transfer->ptr = (uint8_t*)stringLangidDesc();
                    transfer->direction = DEVICE_TO_HOST;
                    success = true;
                    break;
                case STRING_OFFSET_IMANUFACTURER:
                    printf_dbg("2\r\n");
                    transfer->remaining =  stringImanufacturerDesc()[0];
                    transfer->ptr = (uint8_t*)stringImanufacturerDesc();
                    transfer->direction = DEVICE_TO_HOST;
                    success = true;
                    break;
                case STRING_OFFSET_IPRODUCT:
                    printf_dbg("3\r\n");
                    transfer->remaining = stringIproductDesc()[0];
                    transfer->ptr = (uint8_t*)stringIproductDesc();
                    transfer->direction = DEVICE_TO_HOST;
                    success = true;
                    break;
                case STRING_OFFSET_ISERIAL:
                    printf_dbg("4\r\n");
                    transfer->remaining = stringIserialDesc()[0];
                    transfer->ptr = (uint8_t*)stringIserialDesc();
                    transfer->direction = DEVICE_TO_HOST;
                    success = true;
                    break;
                case STRING_OFFSET_ICONFIGURATION:
                    printf_dbg("5\r\n");
                    transfer->remaining = stringIConfigurationDesc()[0];
                    transfer->ptr = (uint8_t*)stringIConfigurationDesc();
                    transfer->direction = DEVICE_TO_HOST;
                    success = true;
                    break;
                case STRING_OFFSET_IINTERFACE:
                    printf_dbg("6\r\n");
                    transfer->remaining = stringIinterfaceDesc()[0];
                    transfer->ptr = (uint8_t*)stringIinterfaceDesc();
                    transfer->direction = DEVICE_TO_HOST;
                    success = true;
                    break;
            }
            break;
        case QUALIFIER_DESCRIPTOR:
            printf_dbg("qualifier descr\r\n");
            transfer->remaining = stringLangidDesc()[0];
            transfer->ptr = (uint8_t*)stringLangidDesc();
            transfer->direction = DEVICE_TO_HOST;
            success = true;
            break;
        case INTERFACE_DESCRIPTOR:
            printf_dbg("interface descr\r\n");
        case ENDPOINT_DESCRIPTOR:
            printf_dbg("endpoint descr\r\n");
            /* TODO: Support is optional, not implemented here */
            break;
        default:
            printf_dbg("ERROR\r\n");
            break;
    }

    return success;
}

bool USBDevice::RequestSetAddress(CONTROL_TRANSFER *transfer)
{
    /* Set the device address */
    SetAddress(transfer->setup.wValue);

    if (transfer->setup.wValue == 0)
    {
        device.state = DEFAULT;
    }
    else
    {
        device.state = ADDRESS;
    }

    return true;
}

bool USBDevice::RequestSetConfiguration(CONTROL_TRANSFER *transfer)
{

    device.configuration = transfer->setup.wValue;
    /* Set the device configuration */
    if (device.configuration == 0)
    {
        /* Not configured */
        // Meano TODO
        //unconfigureDevice();
        device.state = ADDRESS;
    }
    else
    {
        if (USBCallback_setConfiguration(device.configuration))
        {
            /* Valid configuration */
            ConfigureDevice();
            device.state = CONFIGURED;
        }
        else
        {
            return false;
        }
    }
    transfer->direction = HOST_TO_DEVICE;
    return true;
}

bool USBDevice::RequestGetConfiguration(CONTROL_TRANSFER *transfer)
{
    /* Send the device configuration */
    transfer->ptr = &device.configuration;
    transfer->remaining = sizeof(device.configuration);
    transfer->direction = DEVICE_TO_HOST;
    return true;
}

bool USBDevice::RequestGetInterface(CONTROL_TRANSFER *transfer)
{
    /* Return the selected alternate setting for an interface */

    if (device.state != CONFIGURED)
    {
        return false;
    }

    /* Send the alternate setting */
    transfer->setup.wIndex = currentInterface;
    transfer->ptr = &currentAlternate;
    transfer->remaining = sizeof(currentAlternate);
    transfer->direction = DEVICE_TO_HOST;
    return true;
}

bool USBDevice::RequestSetInterface(CONTROL_TRANSFER *transfer)
{
    bool success = false;
    if(USBCallback_setInterface(transfer->setup.wIndex, transfer->setup.wValue))
    {
        success = true;
        currentInterface = transfer->setup.wIndex;
        currentAlternate = transfer->setup.wValue;
    }
    return success;
}

bool USBDevice::RequestSetFeature(CONTROL_TRANSFER *transfer)
{
    bool success = false;

    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer->setup.wIndex != 0)
        {
            return false;
        }
    }

    switch (transfer->setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Remote wakeup feature not supported */
            break;
        case ENDPOINT_RECIPIENT:
            if (transfer->setup.wValue == ENDPOINT_HALT)
            {
                /* TODO: We should check that the endpoint number is valid */
                EndpointStall(transfer->setup.wIndex & 0xf, (transfer->setup.wIndex & 0x80) == 0x80 ? EPDirectIn : EPDirectOut);
                success = true;
            }
            break;
        default:
            break;
    }

    return success;
}

bool USBDevice::RequestClearFeature(CONTROL_TRANSFER *transfer)
{
    bool success = false;

    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer->setup.wIndex != 0)
        {
            return false;
        }
    }

    switch (transfer->setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Remote wakeup feature not supported */
            break;
        case ENDPOINT_RECIPIENT:
            /* TODO: We should check that the endpoint number is valid */
            if (transfer->setup.wValue == ENDPOINT_HALT)
            {
                // TODO unstallEndpoint( WINDEX_TO_PHYSICAL(transfer->setup.wIndex));
                success = true;
            }
            break;
        default:
            break;
    }

    return success;
}

bool USBDevice::RequestGetStatus(CONTROL_TRANSFER *transfer)
{
    static uint16_t status;
    bool success = false;

    if (device.state != CONFIGURED)
    {
        /* Endpoint or interface must be zero */
        if (transfer->setup.wIndex != 0)
        {
            return false;
        }
    }

    switch (transfer->setup.bmRequestType.Recipient)
    {
        case DEVICE_RECIPIENT:
            /* TODO: Currently only supports self powered devices */
            status = DEVICE_STATUS_SELF_POWERED;
            success = true;
            break;
        case INTERFACE_RECIPIENT:
            status = 0;
            success = true;
            break;
        case ENDPOINT_RECIPIENT:
            /* TODO: We should check that the endpoint number is valid */
            // Meano TODO
            /*if (getEndpointStallState(
                WINDEX_TO_PHYSICAL(transfer->setup.wIndex)))
            {
                status = ENDPOINT_STATUS_HALT;
            }
            else*/
            {
                status = 0;
            }
            success = true;
            break;
        default:
            break;
    }

    if (success)
    {
        /* Send the status */
        transfer->ptr = (uint8_t *)&status; /* Assumes little endian */
        transfer->remaining = sizeof(status);
        transfer->direction = DEVICE_TO_HOST;
    }

    return success;
}

void USBDevice::busReset(void)
{
    device.state = DEFAULT;
    device.configuration = 0;
    device.suspended = false;

    /* Call class / vendor specific busReset function */
    USBCallback_busReset();
}

bool USBDevice::configured(void)
{
    /* Returns true if device is in the CONFIGURED state */
    return (device.state == CONFIGURED);
}

void USBDevice::connect(bool blocking)
{
    /* Connect device */
    USBHAL::Connect();

    if (blocking) {
        /* Block if not configured */
        while (!configured()){

        }
    }
}

void USBDevice::disconnect(void)
{
    /* Disconnect device */
    USBHAL::DisConnect();
    
    /* Set initial device state */
    device.state = POWERED;
    device.configuration = 0;
    device.suspended = false;
}

uint8_t * USBDevice::findDescriptor(uint8_t descriptorType)
{
    /* Find a descriptor within the list of descriptors */
    /* following a configuration descriptor. */
    uint16_t wTotalLength;
    uint8_t *ptr;

    if (configurationDesc() == NULL)
    {
        return NULL;
    }

    /* Check this is a configuration descriptor */
    if ((configurationDesc()[0] != CONFIGURATION_DESCRIPTOR_LENGTH) \
            || (configurationDesc()[1] != CONFIGURATION_DESCRIPTOR))
    {
        return NULL;
    }

    wTotalLength = configurationDesc()[2] | (configurationDesc()[3] << 8);

    /* Check there are some more descriptors to follow */
    if (wTotalLength <= (CONFIGURATION_DESCRIPTOR_LENGTH+2))
    /* +2 is for bLength and bDescriptorType of next descriptor */
    {
        return NULL;
    }

    /* Start at first descriptor after the configuration descriptor */
    ptr = &(((uint8_t*)configurationDesc())[CONFIGURATION_DESCRIPTOR_LENGTH]);

    do {
        if (ptr[1] /* bDescriptorType */ == descriptorType)
        {
            /* Found */
            return ptr;
        }

        /* Skip to next descriptor */
        ptr += ptr[0]; /* bLength */
    } while (ptr < (configurationDesc() + wTotalLength));

    /* Reached end of the descriptors - not found */
    return NULL;
}

void USBDevice::connectStateChanged(unsigned int connected)
{
}

void USBDevice::suspendStateChanged(unsigned int suspended)
{
}


bool USBDevice::write(uint8_t endpoint, uint8_t * buffer, uint32_t size, uint32_t maxSize)
{
    return writeNB(endpoint, buffer, size, maxSize);
}


bool USBDevice::writeNB(uint8_t endpoint, uint8_t * buffer, uint32_t size, uint32_t maxSize)
{
    if (size > maxSize)
    {
        return false;
    }

    if(!configured()) {
        return false;
    }

    /* Send report */
    return EndpointStartWrite(endpoint, (uint32_t *)buffer, size);
}



bool USBDevice::readEP(uint8_t endpoint, uint8_t * buffer, uint32_t * size, uint32_t maxSize)
{
    if(!configured()) {
        return false;
    }

    /* Wait for completion */
    printf_dbg("rep:%d l:%d\n", endpoint, *size);
    return true;
}


bool USBDevice::readEP_NB(uint8_t endpoint, uint8_t * buffer, uint32_t * size, uint32_t maxSize)
{
    if(!configured()) {
        return false;
    }

    return true;
}

const uint8_t * USBDevice::deviceDesc() {
    uint8_t deviceDescriptorTemp[] = {
        DEVICE_DESCRIPTOR_LENGTH,       /* bLength */
        DEVICE_DESCRIPTOR,              /* bDescriptorType */
        LSB(USB_VERSION_2_0),           /* bcdUSB (LSB) */
        MSB(USB_VERSION_2_0),           /* bcdUSB (MSB) */
        0x00,                           /* bDeviceClass */
        0x00,                           /* bDeviceSubClass */
        0x00,                           /* bDeviceprotocol */
        64,                             /* bMaxPacketSize0 */
        (uint8_t)(LSB(VENDOR_ID)),                 /* idVendor (LSB) */
        (uint8_t)(MSB(VENDOR_ID)),                 /* idVendor (MSB) */
        (uint8_t)(LSB(PRODUCT_ID)),                /* idProduct (LSB) */
        (uint8_t)(MSB(PRODUCT_ID)),                /* idProduct (MSB) */
        (uint8_t)(LSB(PRODUCT_RELEASE)),           /* bcdDevice (LSB) */
        (uint8_t)(MSB(PRODUCT_RELEASE)),           /* bcdDevice (MSB) */
        STRING_OFFSET_IMANUFACTURER,    /* iManufacturer */
        STRING_OFFSET_IPRODUCT,         /* iProduct */
        STRING_OFFSET_ISERIAL,          /* iSerialNumber */
        0x01                            /* bNumConfigurations */
    };
    //MBED_ASSERT(sizeof(deviceDescriptorTemp) == sizeof(deviceDescriptor));
    memcpy(deviceDescriptor, deviceDescriptorTemp, sizeof(deviceDescriptor));
    return (uint8_t *)deviceDescriptor;
}

uint32_t desctemp[64];
const uint8_t * USBDevice::configurationDesc() {
    uint8_t * Desc = (uint8_t *) desctemp;
    Desc[0] = 9;
    Desc[1] = 2;
    Desc[2] = LSB(ConfigurationDescriptorSize);
    Desc[3] = MSB(ConfigurationDescriptorSize);
    Desc[4] = InterfaceCount;
    Desc[5] = 0x01;
    Desc[6] = 0x00;
    Desc[7] = 0xC0;
    Desc[8] = 200;
    
    int offset = 9;
    for(int i = 0; i < InterfaceCount; i++) {
        memcpy(Desc + offset, Interface[i]->GetInterfaceDescriptor(), Interface[i]->GetInterfaceDescriptorSize());
        offset += Interface[i]->GetInterfaceDescriptorSize();
    }
    return Desc;
}

const uint8_t * USBDevice::stringLangidDesc() {
    static const uint8_t stringLangidDescriptor[] = {
        0x04,               /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        0x09,0x04,          /*bString Lang ID - 0x0409 - English*/
    };
    return (uint8_t *)stringLangidDescriptor;
}

const uint8_t * USBDevice::stringImanufacturerDesc() {
    static const uint8_t stringImanufacturerDescriptor[] = {
        0x12,                                            /*bLength*/
        STRING_DESCRIPTOR,                               /*bDescriptorType 0x03*/
        'm',0,'b',0,'e',0,'d',0,'.',0,'o',0,'r',0,'g',0, /*bString iManufacturer - mbed.org*/
    };
    return stringImanufacturerDescriptor;
}

const uint8_t * USBDevice::stringIserialDesc() {
    static const uint8_t stringIserialDescriptor[] = {
        0x16,                                                           /*bLength*/
        STRING_DESCRIPTOR,                                              /*bDescriptorType 0x03*/
        '0',0,'1',0,'2',0,'3',0,'4',0,'5',0,'6',0,'7',0,'8',0,'9',0,    /*bString iSerial - 0123456789*/
    };
    return stringIserialDescriptor;
}

const uint8_t * USBDevice::stringIConfigurationDesc() {
    static const uint8_t stringIconfigurationDescriptor[] = {
        0x06,               /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        '0',0,'1',0,        /*bString iConfiguration - 01*/
    };
    return stringIconfigurationDescriptor;
}

const uint8_t * USBDevice::stringIinterfaceDesc() {
    static const uint8_t stringIinterfaceDescriptor[] = {
        0x08,               /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        'U',0,'S',0,'B',0,  /*bString iInterface - USB*/
    };
    return stringIinterfaceDescriptor;
}

const uint8_t * USBDevice::stringIproductDesc() {
    static const uint8_t stringIproductDescriptor[] = {
        0x16,                                                       /*bLength*/
        STRING_DESCRIPTOR,                                          /*bDescriptorType 0x03*/
        'U',0,'S',0,'B',0,' ',0,'D',0,'E',0,'V',0,'I',0,'C',0,'E',0 /*bString iProduct - USB DEVICE*/
    };
    return stringIproductDescriptor;
}

USBDevice::~USBDevice() {

}