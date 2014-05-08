#include <windows.h>

#include "IgorUSB.h"

#include "lusb0_usb.h"

#define VENDOR_ATMEL    0x03eb
#define DEVICE_IGORPLUG 0x0002

#define DO_SET_INFRA_BUFFER_EMPTY 0x1
#define DO_GET_INFRA_CODE 0x2

#define HEADER_LENGTH 3
#define MAX_BUFFER_SIZE 256

struct usb_dev_handle* gDevice = NULL;

bool OpenDevice()
{
    if(gDevice)
        return true;

    if(!gDevice)
    {
        usb_init();
        usb_find_busses();
        usb_find_devices();

        struct usb_bus* bus;
        struct usb_device* roottree;
        struct usb_device* device;

        for(bus = usb_busses; bus != NULL; bus = bus->next) 
        {
            roottree = bus->devices;
            for(device = roottree; device; device = device->next)
            {
                if(    device->descriptor.idVendor == VENDOR_ATMEL
                    && device->descriptor.idProduct == DEVICE_IGORPLUG)
                {
                    OutputDebugStringA("Found device at");
                    OutputDebugStringA(device->bus->dirname);
                    OutputDebugStringA(device->filename);

                    gDevice = usb_open(device);
                    return true;
                }
            }
        }
    }

    return false;
}

void CloseDevice()
{
    if(gDevice)
    {
        usb_close(gDevice);
        gDevice = NULL;
    }
}

bool SendToDevice(int fn, int param1, int param2, char* buf, int buf_size, int& recvd_bytes)
{
    if(OpenDevice())
    {
        recvd_bytes = usb_control_msg(gDevice, USB_TYPE_VENDOR | USB_ENDPOINT_IN, fn, param1, param2, buf, buf_size, 500);
        if(recvd_bytes < 0)
        {
            OutputDebugStringA(usb_strerror());
            CloseDevice();
        }

        return true;
    }
    return false;
}

IGORUSB_API int __stdcall DoSetInfraBufferEmpty()
{
    int result = IGORUSB_DEVICE_NOT_PRESENT;

    char buf[2];
    int buf_size = 1;
    int recvd_bytes;

    if(SendToDevice(DO_SET_INFRA_BUFFER_EMPTY, 0, 0, buf, buf_size, recvd_bytes))
        result = NO_ERROR;

    return result;
}

IGORUSB_API int __stdcall DoGetInfraCode(unsigned char* TimeCodeDiagram, int /*DummyInt*/, int* DiagramLength)
{
    char buf[MAX_BUFFER_SIZE];
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

        if(!SendToDevice(DO_GET_INFRA_CODE, i + HEADER_LENGTH, 0, &buf[i], buf_size, recvd))
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
