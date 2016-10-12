#include <windows.h>

#include "IgorUSB.h"

#pragma warning(push)
#pragma warning(disable:4200 4510 4512 4610)
#include "libusb.h"
#pragma warning(pop)

#define VENDOR_ATMEL    0x03eb
#define DEVICE_IGORPLUG 0x0002

#define DO_SET_INFRA_BUFFER_EMPTY 0x1
#define DO_GET_INFRA_CODE 0x2

#define HEADER_LENGTH 3
#define MAX_BUFFER_SIZE 256

libusb_context* gContext = NULL;
libusb_device_handle* gDevice = NULL;

bool OpenDevice()
{
    if(gDevice)
        return true;

    if(!gDevice)
    {
        if(!gContext)
        {
            int err = libusb_init(&gContext);
            if(err)
            {
                OutputDebugStringA("Failed to initialize libusb.");
                gContext = NULL;
                return false;
            }
        }

        gDevice = libusb_open_device_with_vid_pid(gContext, VENDOR_ATMEL, DEVICE_IGORPLUG);
        if(gDevice)
        {
            OutputDebugStringA("Found IgorPlugUSB device.");
            return true;
        }
    }

    return false;
}

void CloseDevice()
{
    if(gContext)
    {
        if (gDevice)
        {
            libusb_close(gDevice);
            gDevice = NULL;
        }
        libusb_exit(gContext);
    }
}

bool SendToDevice(uint8_t fn, uint16_t param1, uint16_t param2, unsigned char* buf, uint16_t buf_size, int& recvd_bytes)
{
    if(OpenDevice())
    {
        recvd_bytes = libusb_control_transfer(gDevice, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, fn, param1, param2, buf, buf_size, 500);
        if(recvd_bytes < 0)
        {
            OutputDebugStringA(libusb_strerror((libusb_error)recvd_bytes));
            CloseDevice();
        }

        return true;
    }
    return false;
}

IGORUSB_API int __stdcall DoSetInfraBufferEmpty()
{
    int result = IGORUSB_DEVICE_NOT_PRESENT;

    unsigned char buf[2];
    uint16_t buf_size = 1;
    int recvd_bytes;

    if(SendToDevice(DO_SET_INFRA_BUFFER_EMPTY, 0, 0, buf, buf_size, recvd_bytes))
        result = NO_ERROR;

    return result;
}

IGORUSB_API int __stdcall DoGetInfraCode(unsigned char* TimeCodeDiagram, int /*DummyInt*/, int* DiagramLength)
{
    unsigned char buf[MAX_BUFFER_SIZE];
    int bytes_to_read = 0;
    int i, buf_size, recvd, msg_idx, j, k, last_written_idx;
    static int last_read = -1;

    memset(buf, 0, sizeof(buf));
    *DiagramLength = 0;

    if(!SendToDevice(DO_GET_INFRA_CODE, 0, 0, buf, 3, recvd))
    {
        return IGORUSB_DEVICE_NOT_PRESENT;
    }

    if(recvd != 3)
    {
        // Nothing to do
        return NO_ERROR;
    }

    bytes_to_read = buf[0];
    if(bytes_to_read == 0)
        return NO_ERROR;
    msg_idx = buf[1];
    last_written_idx = buf[2];

    i = 0;
    while(i < bytes_to_read)
    {
        buf_size = bytes_to_read - i;
        if(buf_size > MAX_BUFFER_SIZE)
        {
            OutputDebugString(L"buffer is too small");
            break;
        }

        if(!SendToDevice(DO_GET_INFRA_CODE, (uint16_t)i + HEADER_LENGTH, 0, &buf[i], (uint16_t)buf_size, recvd))
            return IGORUSB_DEVICE_NOT_PRESENT;

        if(recvd < 0)
        {
            last_read = -1;
            return DoSetInfraBufferEmpty();
        }

        i += recvd;
    }

    if(msg_idx != last_read)
    {
        // new message
        j = last_written_idx % bytes_to_read;
        k = 0;
        for(i=j; i<bytes_to_read; ++i)
        {
            TimeCodeDiagram[k] = buf[i];
            ++k;
        }
        for(i=0; i<j; ++i)
        {
            TimeCodeDiagram[k] = buf[i];
            ++k;
        }
        *DiagramLength = bytes_to_read;
    }
    else
    {
        // message is repeated (has same index as before)
        // -> do nothing
        *DiagramLength = 0;
    }
    last_read = msg_idx;

    return DoSetInfraBufferEmpty();
}

IGORUSB_API int __stdcall DoSetDataPortDirection(unsigned char /*DirectionByte*/)
{
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoGetDataPortDirection(unsigned char * DataDirectionByte)
{
    *DataDirectionByte = 0;
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoSetOutDataPort(unsigned char /*DataOutByte*/)
{
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoGetOutDataPort(unsigned char * DataOutByte)
{
    *DataOutByte = 0;
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoGetInDataPort(unsigned char * DataInByte)
{
    *DataInByte = 0;
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoEEPROMRead(unsigned char /*Address*/, unsigned char * DataInByte)
{
    *DataInByte = 0;
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoEEPROMWrite(unsigned char /*Address*/, unsigned char /*DataOutByte*/)
{
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoRS232Send(unsigned char /*DataOutByte*/)
{
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoRS232Read(unsigned char * DataInByte)
{
    *DataInByte = 0;
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoSetRS232Baud(int /*BaudRate*/)
{
    return IGORUSB_NOT_IMPLEMENTED;
}

IGORUSB_API int __stdcall DoGetRS232Baud(int * BaudRate)
{
    *BaudRate = 0;
    return IGORUSB_NOT_IMPLEMENTED;
}
