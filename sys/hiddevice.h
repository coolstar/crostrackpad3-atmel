#if !defined(_ATMLPAD_H_)
#define _ATMLPAD_H_

#pragma warning(disable:4200)  // suppress nameless struct/union warning
#pragma warning(disable:4201)  // suppress nameless struct/union warning
#pragma warning(disable:4214)  // suppress bit field types other than int warning
#include <initguid.h>
#include <wdm.h>

#pragma warning(default:4200)
#pragma warning(default:4201)
#pragma warning(default:4214)
#include <wdf.h>

#pragma warning(disable:4201)  // suppress nameless struct/union warning
#pragma warning(disable:4214)  // suppress bit field types other than int warning
#include <hidport.h>

#include "hidcommon.h"

//
// String definitions
//

#define DRIVERNAME                 "crostrackpad-atmel.sys: "

#define ATMLPAD_POOL_TAG            (ULONG)'PMTA'
#define ATMLPAD_HARDWARE_IDS        L"ACPI\\ATML0000\0\0"
#define ATMLPAD_HARDWARE_IDS_LENGTH sizeof(ATMLPAD_HARDWARE_IDS)

#define NTDEVICE_NAME_STRING       L"\\Device\\ATML0000"
#define SYMBOLIC_NAME_STRING       L"\\DosDevices\\ATML0000"

//
// This is the default report descriptor for the Hid device provided
// by the mini driver in response to IOCTL_HID_GET_REPORT_DBESCRIPTOR.
// 

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

#ifdef DEFINEDESCRIPTOR
HID_REPORT_DESCRIPTOR DefaultReportDescriptor[] = {
	//
	// Relative mouse report starts here
	//
	0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                         // USAGE (Mouse)
	0xa1, 0x01,                         // COLLECTION (Application)
	0x85, REPORTID_RELATIVE_MOUSE,      //   REPORT_ID (Mouse)
	0x09, 0x01,                         //   USAGE (Pointer)
	0xa1, 0x00,                         //   COLLECTION (Physical)
	0x05, 0x09,                         //     USAGE_PAGE (Button)
	0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)
	0x29, 0x05,                         //     USAGE_MAXIMUM (Button 5)
	0x15, 0x00,                         //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
	0x75, 0x01,                         //     REPORT_SIZE (1)
	0x95, 0x05,                         //     REPORT_COUNT (5)
	0x81, 0x02,                         //     INPUT (Data,Var,Abs)
	0x95, 0x03,                         //     REPORT_COUNT (3)
	0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                         //     USAGE (X)
	0x09, 0x31,                         //     USAGE (Y)
	0x15, 0x81,                         //     Logical Minimum (-127)
	0x25, 0x7F,                         //     Logical Maximum (127)
	0x75, 0x08,                         //     REPORT_SIZE (8)
	0x95, 0x02,                         //     REPORT_COUNT (2)
	0x81, 0x06,                         //     INPUT (Data,Var,Rel)
	0x05, 0x01,                         //     Usage Page (Generic Desktop)
	0x09, 0x38,                         //     Usage (Wheel)
	0x15, 0x81,                         //     Logical Minimum (-127)
	0x25, 0x7F,                         //     Logical Maximum (127)
	0x75, 0x08,                         //     Report Size (8)
	0x95, 0x01,                         //     Report Count (1)
	0x81, 0x06,                         //     Input (Data, Variable, Relative)
	// ------------------------------  Horizontal wheel
	0x05, 0x0c,                         //     USAGE_PAGE (Consumer Devices)
	0x0a, 0x38, 0x02,                   //     USAGE (AC Pan)
	0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)
	0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)
	0x75, 0x08,                         //     REPORT_SIZE (8)
	0x95, 0x01,                         //     Report Count (1)
	0x81, 0x06,                         //     Input (Data, Variable, Relative)
	0xc0,                               //   END_COLLECTION
	0xc0,                               // END_COLLECTION

	0x06, 0x00, 0xff,                    // USAGE_PAGE (Vendor Defined Page 1)
	0x09, 0x01,                          // USAGE (Vendor Usage 1)
	0xa1, 0x01,                          // COLLECTION (Application)
	0x85, REPORTID_SCROLL,              //   REPORT_ID (Scroll)
	0x15, 0x00,                          //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,                    //   LOGICAL_MAXIMUM (256)
	0x75, 0x08,                          //   REPORT_SIZE  (8)   - bits
	0x95, 0x01,                          //   REPORT_COUNT (1)  - Bytes
	0x09, 0x02,                          //   USAGE (Vendor Usage 1)
	0x81, 0x02,                          //   INPUT (Data,Var,Abs)
	0x15, 0x00,                          //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0xff,                    //   LOGICAL_MAXIMUM (65535)
	0x75, 0x10,                          //   REPORT_SIZE  (16)   - bits
	0x95, 0x04,                          //   REPORT_COUNT (4)  - Bytes
	0x09, 0x03,                          //   USAGE (Vendor Usage 2)
	0x81, 0x02,                          //   INPUT (Data,Var,Abs)
	0xc0,                                // END_COLLECTION

	0x06, 0x00, 0xff,                    // USAGE_PAGE (Vendor Defined Page 1)
	0x09, 0x02,                          // USAGE (Vendor Usage 2)
	0xa1, 0x01,                          // COLLECTION (Application)
	0x85, REPORTID_SCROLLCTRL,              //   REPORT_ID (Scroll Controls)
	0x15, 0x00,                          //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,                    //   LOGICAL_MAXIMUM (256)
	0x75, 0x08,                          //   REPORT_SIZE  (8)   - bits
	0x95, 0x01,                          //   REPORT_COUNT (1)  - Bytes
	0x09, 0x02,                          //   USAGE (Vendor Usage 1)
	0x91, 0x02,                          //   OUTPUT (Data,Var,Abs)
	0xc0,                                // END_COLLECTION

	0x06, 0x00, 0xff,                    // USAGE_PAGE (Vendor Defined Page 1)
	0x09, 0x03,                          // USAGE (Vendor Usage 3)
	0xa1, 0x01,                          // COLLECTION (Application)
	0x85, REPORTID_SETTINGS,              //   REPORT_ID (Settings)
	0x15, 0x00,                          //   LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,                    //   LOGICAL_MAXIMUM (256)
	0x75, 0x08,                          //   REPORT_SIZE  (8)   - bits
	0x95, 0x01,                          //   REPORT_COUNT (1)  - Bytes
	0x09, 0x02,                          //   USAGE (Vendor Usage 1)
	0x91, 0x02,                          //   OUTPUT (Data,Var,Abs)
	0x09, 0x03,                          //   USAGE (Vendor Usage 2)
	0x91, 0x02,                          //   OUTPUT (Data,Var,Abs)
	0x95, 0x40,                          //   REPORT_COUNT (64)  - Bytes
	0x09, 0x02,                          //   USAGE (Vendor Usage 1)
	0x81, 0x02,                          //   INPUT (Data,Var,Abs)
	0xc0,                                // END_COLLECTION

	//
	// Keyboard report starts here
	//    
	0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)
	0x09, 0x06,                         // USAGE (Keyboard)
	0xa1, 0x01,                         // COLLECTION (Application)
	0x85, REPORTID_KEYBOARD,            //   REPORT_ID (Keyboard)    
	0x05, 0x07,                         //   USAGE_PAGE (Keyboard)
	0x19, 0xe0,                         //   USAGE_MINIMUM (Keyboard LeftControl)
	0x29, 0xe7,                         //   USAGE_MAXIMUM (Keyboard Right GUI)
	0x15, 0x00,                         //   LOGICAL_MINIMUM (0)
	0x25, 0x01,                         //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,                         //   REPORT_SIZE (1)
	0x95, 0x08,                         //   REPORT_COUNT (8)
	0x81, 0x02,                         //   INPUT (Data,Var,Abs)
	0x95, 0x01,                         //   REPORT_COUNT (1)
	0x75, 0x08,                         //   REPORT_SIZE (8)
	0x81, 0x03,                         //   INPUT (Cnst,Var,Abs)
	0x95, 0x05,                         //   REPORT_COUNT (5)
	0x75, 0x01,                         //   REPORT_SIZE (1)
	0x05, 0x08,                         //   USAGE_PAGE (LEDs)
	0x19, 0x01,                         //   USAGE_MINIMUM (Num Lock)
	0x29, 0x05,                         //   USAGE_MAXIMUM (Kana)
	0x91, 0x02,                         //   OUTPUT (Data,Var,Abs)
	0x95, 0x01,                         //   REPORT_COUNT (1)
	0x75, 0x03,                         //   REPORT_SIZE (3)
	0x91, 0x03,                         //   OUTPUT (Cnst,Var,Abs)
	0x95, 0x06,                         //   REPORT_COUNT (6)
	0x75, 0x08,                         //   REPORT_SIZE (8)
	0x15, 0x00,                         //   LOGICAL_MINIMUM (0)
	0x25, 0x65,                         //   LOGICAL_MAXIMUM (101)
	0x05, 0x07,                         //   USAGE_PAGE (Keyboard)
	0x19, 0x00,                         //   USAGE_MINIMUM (Reserved (no event indicated))
	0x29, 0x65,                         //   USAGE_MAXIMUM (Keyboard Application)
	0x81, 0x00,                         //   INPUT (Data,Ary,Abs)
	0xc0,                               // END_COLLECTION
};


//
// This is the default HID descriptor returned by the mini driver
// in response to IOCTL_HID_GET_DEVICE_DESCRIPTOR. The size
// of report descriptor is currently the size of DefaultReportDescriptor.
//

CONST HID_DESCRIPTOR DefaultHidDescriptor = {
	0x09,   // length of HID descriptor
	0x21,   // descriptor type == HID  0x21
	0x0100, // hid spec release
	0x00,   // country code == Not Specified
	0x01,   // number of HID class descriptors
	{ 0x22,   // descriptor type 
	sizeof(DefaultReportDescriptor) }  // total length of report descriptor
};
#endif

//
// Function definitions
//

NTSTATUS
AtmlPadGetHidDescriptor(
IN WDFDEVICE Device,
IN WDFREQUEST Request
);

NTSTATUS
AtmlPadGetReportDescriptor(
IN WDFDEVICE Device,
IN WDFREQUEST Request
);

NTSTATUS
AtmlPadGetDeviceAttributes(
IN WDFREQUEST Request
);

NTSTATUS
AtmlPadGetString(
IN WDFREQUEST Request
);

NTSTATUS
AtmlPadProcessVendorReport(
IN PDEVICE_CONTEXT DevContext,
IN PVOID ReportBuffer,
IN ULONG ReportBufferLen,
OUT size_t* BytesWritten
);

NTSTATUS
AtmlPadReadReport(
IN PDEVICE_CONTEXT DevContext,
IN WDFREQUEST Request,
OUT BOOLEAN* CompleteRequest
);

NTSTATUS
AtmlPadWriteReport(
	IN PDEVICE_CONTEXT DevContext,
	IN WDFREQUEST Request
	);

NTSTATUS
AtmlPadGetFeature(
IN PDEVICE_CONTEXT DevContext,
IN WDFREQUEST Request,
OUT BOOLEAN* CompleteRequest
);

PCHAR
DbgHidInternalIoctlString(
IN ULONG        IoControlCode
);

//
// Helper macros
//

#define DEBUG_LEVEL_ERROR   1
#define DEBUG_LEVEL_INFO    2
#define DEBUG_LEVEL_VERBOSE 3

#define DBG_INIT  1
#define DBG_PNP   2
#define DBG_IOCTL 4

#if 0

static ULONG AtmlPadPrintDebugLevel = 100;
static ULONG AtmlPadPrintDebugCatagories = DBG_INIT || DBG_PNP || DBG_IOCTL;

#define AtmlPadPrint(dbglevel, dbgcatagory, fmt, ...) {          \
    if (AtmlPadPrintDebugLevel >= dbglevel &&                         \
        (AtmlPadPrintDebugCatagories && dbgcatagory))                 \
	    {                                                           \
        DbgPrint(DRIVERNAME);                                   \
        DbgPrint(fmt, __VA_ARGS__);                             \
	    }                                                           \
}
#else
#define AtmlPadPrint(dbglevel, fmt, ...) {                       \
}
#endif

#endif
