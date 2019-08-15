#ifndef USBENDPOINTS_H
#define USBENDPOINTS_H

#include "target.h"

// Type Define
typedef enum {
    EPTypeControl               = 0x00,
    EPTypeISO     = 0x01,
    EPTypeBulk                  = 0x02,
    EPTypeInterrupt                   = 0x03,
} EPType;

typedef enum {
    EPDirectOut                 = 0x00,
    EPDirectIn                  = 0x01,
} EPDirect;

typedef enum {
    EPCompleted,                                                // Transfer completed
    EPPending,                                                  // Transfer in progress
    EPInvalid,                                                  // Invalid parameter
    EPStalled,                                                  // Endpoint stalled
} EPStatus;

typedef struct EPConfigStruct_s EPConfigStruct;
typedef struct EPStruct_s EPStruct;
class USBInterface;

struct EPStruct_s{
    bool            IsConfigured;                               // Endpoint is Configured
    uint32_t        FifoIndex;                                  // Fifo Index
    EPStatus        Status;                                     // Endpoint Status
    uint32_t        *Buffer;                                    // Buffer Address
    uint32_t        *TransferPtr;                               // Buffer Point
    uint32_t        TransferSize;                               // Transfer Size
    EPConfigStruct  *Config;                                    // Endpoint Init Config
};

struct EPConfigStruct_s{
    uint8_t         Index;
    EPDirect        Direct;
    EPType          Type;                                       // Endpoint Tyep : Control / Interrupt / Bulk  / ISO
    uint32_t        MaxPacket;                                  // Endpoint Max Packet Size
    uint32_t        MaxBuffer;                                  // Endpoint Max Single Transfer Size
    uint32_t        *Buffer;
    USBInterface    *Interface;
};

/* Chip supported endpoint count                              */
#define USB_ENDPOINT_COUNT      (4)

/* SETUP packet size                                          */
#define SETUP_PACKET_SIZE       (8)

/* Maximum Packet sizes */
#define MAX_CONTROL_EP_COUNT    (1)
#define MAX_RX_PACKET_SIZE      (256)
#define MAX_OUT_EP_COUNT        (4)

#define MAX_FIFO_SIZE           (1024 - 128)                    // Hardware defined
#define RX_FIFO_SIZE            (((5 * MAX_CONTROL_EP_COUNT + 8) + ((MAX_RX_PACKET_SIZE / 4) + 1) + (2 * MAX_OUT_EP_COUNT) + 1) * 4)   //(384)

#endif
