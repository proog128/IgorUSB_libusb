#ifdef __cplusplus
extern "C" {
#endif

#ifdef IGORUSB_EXPORTS
#define IGORUSB_API __declspec(dllexport)
#else
#define IGORUSB_API __declspec(dllimport)
#endif

#define IgorUSBDLL    "IgorUSB.dll";

//return values from IgorUSBDLL functions:
#define IGORUSB_NO_ERROR                0
#define IGORUSB_DEVICE_NOT_PRESENT      1
#define IGORUSB_NO_DATA_AVAILABLE       2
#define IGORUSB_INVALID_BAUDRATE        3
#define IGORUSB_OVERRUN_ERROR           4
#define IGORUSB_NOT_IMPLEMENTED         5

IGORUSB_API int __stdcall DoSetInfraBufferEmpty();
IGORUSB_API int __stdcall DoGetInfraCode(unsigned char * TimeCodeDiagram, int DummyInt, int * DiagramLength);
IGORUSB_API int __stdcall DoSetDataPortDirection(unsigned char DirectionByte);
IGORUSB_API int __stdcall DoGetDataPortDirection(unsigned char * DataDirectionByte);
IGORUSB_API int __stdcall DoSetOutDataPort(unsigned char DataOutByte);
IGORUSB_API int __stdcall DoGetOutDataPort(unsigned char * DataOutByte);
IGORUSB_API int __stdcall DoGetInDataPort(unsigned char * DataInByte);
IGORUSB_API int __stdcall DoEEPROMRead(unsigned char Address, unsigned char * DataInByte);
IGORUSB_API int __stdcall DoEEPROMWrite(unsigned char Address, unsigned char DataOutByte);
IGORUSB_API int __stdcall DoRS232Send(unsigned char DataOutByte);
IGORUSB_API int __stdcall DoRS232Read(unsigned char * DataInByte);
IGORUSB_API int __stdcall DoSetRS232Baud(int BaudRate);
IGORUSB_API int __stdcall DoGetRS232Baud(int * BaudRate);

#ifdef __cplusplus
}
#endif
