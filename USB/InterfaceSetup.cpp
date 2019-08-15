#include "InterfaceSetup.h"
#include "USBDevice.h"

InterfaceSetup::InterfaceSetup(USBDevice *deviceBase) : USBInterface(deviceBase, 0, 2, 0, 0, 0)
{
    RegisterEndpoint(EP0ConfigIn,  0, EPDirectIn,  EPTypeControl, 64, 64, SetupBufferIn );
    RegisterEndpoint(EP0ConfigOut, 0, EPDirectOut, EPTypeControl, 64, 64, SetupBufferOut);
    iEPConfig = &EPConfig[EP0ConfigIn];
    oEPConfig = &EPConfig[EP0ConfigOut];
}

bool InterfaceSetup::EndpointCallback(uint8_t epIndex, EPDirect epDirect, uint32_t * buffer, uint32_t bytesTransfer)
{
    if(epIndex == 0){
        if(epDirect == EPDirectIn) {
            return SetupCallbackIn(buffer, bytesTransfer);
        }
        else {
            return SetupCallbackOut(buffer, bytesTransfer);
        }
    }
    return false;
}

bool InterfaceSetup::SetupCallbackIn(uint32_t *buffer, uint32_t bytesWritten)
{
    if (buffer == NULL && bytesWritten == 0) {                   // for transfer 1st Packet
        if (transfer.direction != DEVICE_TO_HOST)
        {
            return false;
        }

        /* Transfer must be less than or equal to the size */
        /* requested by the host */
        if (transfer.remaining > transfer.setup.wLength)
        {
            transfer.remaining = transfer.setup.wLength;
        }

        if (transfer.setup.wLength > transfer.remaining)
        {
            /* Device wishes to transfer less than host requested */
            if ((transfer.remaining % 64) == 0)
            {
                /* Transfer is a multiple of EP0 max packet size */
                transfer.zlp = true;
            }
        }
    }

    /* Check if transfer has completed (status stage transactions */
    /* also have transfer.remaining == 0) */
    if (transfer.remaining == 0)
    {
        if (transfer.direction == HOST_TO_DEVICE) {
            printf_dbg("**** End two-step ****\n");
            return true;
        }

        if (transfer.zlp)
        {
            DeviceBase->EndpointStartWrite(iEPConfig->Index, iEPConfig->Buffer, 0);
            transfer.zlp = false;
            return true;
        }
        /* Transfer completed */
        if (transfer.notify)
        {
            /* Notify class layer. */
            // TODO USBCallback_requestCompleted(NULL, 0);
            transfer.notify = false;
        }
        DeviceBase->EndpointStartRead(oEPConfig->Index, oEPConfig->Buffer, oEPConfig->MaxBuffer);
        /* Completed */
        return true;
    }
    else {
        /* Check we should be transferring data IN */
        if (transfer.direction != DEVICE_TO_HOST)
        {
            return false;
        }

        int transferSize = MIN(iEPConfig->MaxBuffer, transfer.remaining);

        // if(transfer.remaining == transferSize) {
        DeviceBase->EndpointStartRead(oEPConfig->Index, oEPConfig->Buffer, 0);
        // }
        /* Write to endpoint */
        DeviceBase->EndpointStartWrite(iEPConfig->Index, (uint32_t *)transfer.ptr, transferSize);
        /* Update transfer */
        transfer.ptr += transferSize;
        transfer.remaining -= transferSize;
    }
    return true;
}

bool InterfaceSetup::SetupCallbackOut(uint32_t *buffer, uint32_t bytesRead)
{
    EPConfigStruct *epConfig = &EPConfig[EP0ConfigOut];

    if(bytesRead & BIT(31)) {                                   // for Setup Transfer
        printf_dbg("======== Setup ========\n");
        bool success = false;
        /* Initialise control transfer state */
        DecodeSetupPacket((uint8_t *)buffer, &transfer.setup);
        transfer.ptr = NULL;
        transfer.remaining = 0;
        transfer.direction = 0;
        transfer.zlp = false;
        transfer.notify = false;

        //EP0write(buffer, 8);
        //return true;
        /*printf_dbg(
            "dataTransferDirection: %d\r\nType: %d\r\nRecipient: %d\r\nbRequest: %d\r\nwValue: %d\r\nwIndex: %d\r\nwLength: %d\r\n",
            transfer.setup.bmRequestType.dataTransferDirection,
            transfer.setup.bmRequestType.Type,
            transfer.setup.bmRequestType.Recipient,
            transfer.setup.bRequest,
            transfer.setup.wValue,
            transfer.setup.wIndex,
            transfer.setup.wLength
        );*/

        if (!success)
        {
            /* Standard requests */
            if (!RequestSetup())
            {
                printf_dbg("Setup request failed!\n");
                return false;
            }
        }

        /* Check transfer size and direction */
        if (transfer.setup.wLength > 0)
        {
            if (transfer.setup.bmRequestType.dataTransferDirection == DEVICE_TO_HOST)
            {
                /* IN stage */
                return SetupCallbackIn(NULL, 0);
            }
            else
            {
                /* OUT data stage is required */
                if (transfer.direction != HOST_TO_DEVICE)
                {
                    return false;
                }

                /* Transfer must be equal to the size requested by the host */
                if (transfer.remaining != transfer.setup.wLength)
                {
                    return false;
                }
                transfer.ptr = (uint8_t *)epConfig->Buffer;
                DeviceBase->EndpointStartRead(epConfig->Index, (uint32_t *)transfer.ptr, MIN(epConfig->MaxBuffer, transfer.remaining));
            }
        }
        else
        {
            /* two-step Stage */
            if (transfer.remaining != 0)
            {
                return false;
            }
            DeviceBase->EndpointStartRead(oEPConfig->Index, oEPConfig->Buffer, oEPConfig->MaxBuffer);
            DeviceBase->EndpointStartWrite(iEPConfig->Index, iEPConfig->Buffer, 0);
        }
        return true;
    }
    else {                                                      // for Setup Out Transfer
        /* Check we should be transferring data OUT */
        if (transfer.direction != HOST_TO_DEVICE)
        {
            DeviceBase->EndpointStartRead(oEPConfig->Index, oEPConfig->Buffer, oEPConfig->MaxBuffer);
            printf_dbg("******* End DI *******\n");
            /* for other platforms, count on the HAL to handle this case */
            return true;
        }

        /* Check if transfer size is valid */
        if (bytesRead > transfer.remaining)
        {
            /* Too big */
            return false;
        }

        /* Update transfer */
        transfer.ptr += bytesRead;
        transfer.remaining -= bytesRead;

        /* Check if transfer has completed */
        if (transfer.remaining == 0)
        {
            /* Transfer completed */
            if (transfer.notify)
            {
                /* Notify class layer. */
                // TODO USBCallback_requestCompleted(buffer, packetSize);
                transfer.notify = false;
            }
            printf_dbg("******* End DO *******\n");
        }
        else
        {
            // Meano 20190626 Change to Continue Read
            DeviceBase->EndpointStartRead(epConfig->Index, (uint32_t *)transfer.ptr, MIN(epConfig->MaxBuffer, transfer.remaining));
        }

        return true;
    }
    
}

void InterfaceSetup::DecodeSetupPacket(uint8_t *data, SETUP_PACKET *packet)
{
    /* Fill in the elements of a SETUP_PACKET structure from raw data */
    packet->bmRequestType.dataTransferDirection = (data[0] & 0x80) >> 7;
    packet->bmRequestType.Type = (data[0] & 0x60) >> 5;
    packet->bmRequestType.Recipient = data[0] & 0x1f;
    packet->bRequest = data[1];
    packet->wValue = (data[2] | (uint16_t)data[3] << 8);
    packet->wIndex = (data[4] | (uint16_t)data[5] << 8);
    packet->wLength = (data[6] | (uint16_t)data[7] << 8);
}

bool InterfaceSetup::RequestSetup(void)
{
    bool success = false;

    /* Process standard requests */
    if ((transfer.setup.bmRequestType.Type == STANDARD_TYPE))
    {
        switch (transfer.setup.bRequest)
        {
             case GET_STATUS:
                 success = DeviceBase->RequestGetStatus(&transfer);
                 break;
             case CLEAR_FEATURE:
                 success = DeviceBase->RequestClearFeature(&transfer);
                 break;
             case SET_FEATURE:
                 success = DeviceBase->RequestSetFeature(&transfer);
                 break;
             case SET_ADDRESS:
                success = DeviceBase->RequestSetAddress(&transfer);
                 break;
             case GET_DESCRIPTOR:
                 success = DeviceBase->RequestGetDescriptor(&transfer);
                 break;
             case SET_DESCRIPTOR:
                 /* TODO: Support is optional, not implemented here */
                 success = false;
                 break;
             case GET_CONFIGURATION:
                 success = DeviceBase->RequestGetConfiguration(&transfer);
                 break;
             case SET_CONFIGURATION:
                 success = DeviceBase->RequestSetConfiguration(&transfer);
                 break;
             case GET_INTERFACE:
                 success = DeviceBase->RequestGetInterface(&transfer);
                 break;
             case SET_INTERFACE:
                 success = DeviceBase->RequestSetInterface(&transfer);
                 break;
             default:
                 break;
        }
    }
    return success;
}

InterfaceSetup::~InterfaceSetup()
{

}
