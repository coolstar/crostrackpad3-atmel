#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#pragma warning(push)
#pragma warning(disable:4512)
#pragma warning(disable:4480)

#define SPBT_POOL_TAG ((ULONG) 'TBPS')

/////////////////////////////////////////////////
//
// Common includes.
//
/////////////////////////////////////////////////

#include <ntddk.h>
#include <wdm.h>
#include <wdf.h>
#include <ntstrsafe.h>

#include "spb.h"

#define RESHUB_USE_HELPER_ROUTINES
#include "reshub.h"

#include "trace.h"

#include "atmel_mxt.h"
#include "gesturerec.h"

//
// Forward Declarations
//

typedef struct _DEVICE_CONTEXT  DEVICE_CONTEXT,  *PDEVICE_CONTEXT;
typedef struct _REQUEST_CONTEXT  REQUEST_CONTEXT,  *PREQUEST_CONTEXT;

struct _DEVICE_CONTEXT 
{
    //
    // Handle back to the WDFDEVICE
    //

    WDFDEVICE FxDevice;

    //
    // Handle to the sequential SPB queue
    //

    WDFQUEUE SpbQueue;

    //
    // Connection ID for SPB peripheral
    //

	SPB_CONTEXT I2CContext;
    
    //
    // Interrupt object and wait event
    //

    WDFINTERRUPT Interrupt;
    KEVENT IsrWaitEvent;

    //
    // Setting indicating whether the interrupt should be connected
    //

    BOOLEAN ConnectInterrupt;

	BOOLEAN RegsSet;

    //
    // Client request object
    //

    WDFREQUEST ClientRequest;

    //
    // WaitOnInterrupt request object
    //

    WDFREQUEST WaitOnInterruptRequest;

	WDFTIMER Timer;

	WDFQUEUE ReportQueue;

	BYTE DeviceMode;

	ULONGLONG LastInterruptTime;

	csgesture_softc sc;

	mxt_message_t lastmsg;

	mxt_rollup core;

	struct mxt_object	*msgprocobj;
	struct mxt_object	*cmdprocobj;

	mxt_id_info info;

	UINT32 TouchCount;

	uint8_t      Flags[20];

	USHORT    XValue[20];

	USHORT    YValue[20];

	USHORT    AREA[20];

	uint16_t max_x;
	uint16_t max_y;

	uint8_t num_touchids;
	uint8_t multitouch;

	struct t7_config t7_cfg;

	uint8_t t100_aux_ampl;
	uint8_t t100_aux_area;
	uint8_t t100_aux_vect;

	/* Cached parameters from object table */
	uint16_t T5_address;
	uint8_t T5_msg_size;
	uint8_t T6_reportid;
	uint16_t T6_address;
	uint16_t T7_address;
	uint8_t T9_reportid_min;
	uint8_t T9_reportid_max;
	uint8_t T19_reportid;

	bool T19_buttonstate;

	uint16_t T44_address;
	uint8_t T100_reportid_min;
	uint8_t T100_reportid_max;

	uint8_t max_reportid;

	uint8_t last_message_count;
};

struct _REQUEST_CONTEXT
{    
    //
    // Associated framework device object
    //

    WDFDEVICE FxDevice;

    //
    // Variables to track write length for a sequence request.
    // There are needed to complete the client request with
    // correct bytesReturned value.
    //

    BOOLEAN IsSpbSequenceRequest;
    ULONG_PTR SequenceWriteLength;
};

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext);
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT, GetRequestContext);

#pragma warning(pop)

#endif // _INTERNAL_H_
