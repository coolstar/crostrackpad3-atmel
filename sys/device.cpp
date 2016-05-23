#include "internal.h"
#include "device.h"
#include "hiddevice.h"
#include "spb.h"

//#include "device.tmh"

bool deviceLoaded = false;
NTSTATUS BOOTTRACKPAD(_In_  PDEVICE_CONTEXT  devContext);

/////////////////////////////////////////////////
//
// WDF callbacks.
//
/////////////////////////////////////////////////

NTSTATUS
OnPrepareHardware(
    _In_  WDFDEVICE     FxDevice,
    _In_  WDFCMRESLIST  FxResourcesRaw,
    _In_  WDFCMRESLIST  FxResourcesTranslated
    )
/*++
 
  Routine Description:

    This routine caches the SPB resource connection ID.

  Arguments:

    FxDevice - a handle to the framework device object
    FxResourcesRaw - list of translated hardware resources that 
        the PnP manager has assigned to the device
    FxResourcesTranslated - list of raw hardware resources that 
        the PnP manager has assigned to the device

  Return Value:

    Status

--*/
{
    FuncEntry(TRACE_FLAG_WDFLOADING);

    PDEVICE_CONTEXT pDevice = GetDeviceContext(FxDevice);
    BOOLEAN fSpbResourceFound = FALSE;
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;

    UNREFERENCED_PARAMETER(FxResourcesRaw);

    //
    // Parse the peripheral's resources.
    //

    ULONG resourceCount = WdfCmResourceListGetCount(FxResourcesTranslated);

    for(ULONG i = 0; i < resourceCount; i++)
    {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR pDescriptor;
        UCHAR Class;
        UCHAR Type;

        pDescriptor = WdfCmResourceListGetDescriptor(
            FxResourcesTranslated, i);

        switch (pDescriptor->Type)
        {
            case CmResourceTypeConnection:
                //
                // Look for I2C or SPI resource and save connection ID.
                //
                Class = pDescriptor->u.Connection.Class;
                Type = pDescriptor->u.Connection.Type;
                if (Class == CM_RESOURCE_CONNECTION_CLASS_SERIAL &&
                    Type == CM_RESOURCE_CONNECTION_TYPE_SERIAL_I2C)
                {
                    if (fSpbResourceFound == FALSE)
                    {
						status = STATUS_SUCCESS;
						pDevice->I2CContext.I2cResHubId.LowPart = pDescriptor->u.Connection.IdLowPart;
						pDevice->I2CContext.I2cResHubId.HighPart = pDescriptor->u.Connection.IdHighPart;
                        fSpbResourceFound = TRUE;
                        Trace(
                            TRACE_LEVEL_INFORMATION,
                            TRACE_FLAG_WDFLOADING,
                            "SPB resource found with ID=0x%llx",
							pDevice->I2CContext.I2cResHubId.QuadPart);
                    }
                    else
                    {
                        Trace(
                            TRACE_LEVEL_WARNING,
                            TRACE_FLAG_WDFLOADING,
                            "Duplicate SPB resource found with ID=0x%llx",
							pDevice->I2CContext.I2cResHubId.QuadPart);
                    }
                }
                break;
            default:
                //
                // Ignoring all other resource types.
                //
                break;
        }
    }

    //
    // An SPB resource is required.
    //

    if (fSpbResourceFound == FALSE)
    {
        status = STATUS_NOT_FOUND;
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_FLAG_WDFLOADING,
            "SPB resource not found - %!STATUS!", 
            status);
    }

	status = SpbTargetInitialize(FxDevice, &pDevice->I2CContext);
	BOOTTRACKPAD(pDevice);
	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_FLAG_WDFLOADING,
			"Error in Spb initialization - %!STATUS!",
			status);

		return status;
	}

    FuncExit(TRACE_FLAG_WDFLOADING);

    return status;
}

NTSTATUS
OnReleaseHardware(
    _In_  WDFDEVICE     FxDevice,
    _In_  WDFCMRESLIST  FxResourcesTranslated
    )
/*++
 
  Routine Description:

  Arguments:

    FxDevice - a handle to the framework device object
    FxResourcesTranslated - list of raw hardware resources that 
        the PnP manager has assigned to the device

  Return Value:

    Status

--*/
{
    FuncEntry(TRACE_FLAG_WDFLOADING);
    
    PDEVICE_CONTEXT pDevice = GetDeviceContext(FxDevice);
    NTSTATUS status = STATUS_SUCCESS;
    
    UNREFERENCED_PARAMETER(FxResourcesTranslated);

	ExFreePoolWithTag(pDevice->core.buf, ATMLPAD_POOL_TAG);

	pDevice->core.buf = NULL;

	pDevice->msgprocobj = NULL;
	pDevice->cmdprocobj = NULL;

	SpbTargetDeinitialize(FxDevice, &pDevice->I2CContext);

	deviceLoaded = false;

    FuncExit(TRACE_FLAG_WDFLOADING);

    return status;
}

bool IsAtmlPadLoaded(){
	return deviceLoaded;
}

static size_t mxt_obj_size(const struct mxt_object *obj)
{
	return obj->size_minus_one + 1;
}

static size_t mxt_obj_instances(const struct mxt_object *obj)
{
	return obj->instances_minus_one + 1;
}

static
struct mxt_object *
	mxt_findobject(struct mxt_rollup *core, int type)
{
	int i;

	for (i = 0; i < core->nobjs; ++i) {
		if (core->objs[i].type == type)
			return(&core->objs[i]);
	}
	return NULL;
}

static NTSTATUS
mxt_read_reg(PDEVICE_CONTEXT  devContext, uint16_t reg, void *rbuf, int bytes)
{
	uint8_t wreg[2];
	wreg[0] = reg & 255;
	wreg[1] = reg >> 8;

	uint16_t nreg = ((uint16_t *)wreg)[0];

	NTSTATUS error = SpbReadDataSynchronously16(&devContext->I2CContext, nreg, rbuf, bytes);

	return error;
}

static NTSTATUS
mxt_write_reg_buf(PDEVICE_CONTEXT  devContext, uint16_t reg, void *xbuf, int bytes)
{
	uint8_t wreg[2];
	wreg[0] = reg & 255;
	wreg[1] = reg >> 8;

	uint16_t nreg = ((uint16_t *)wreg)[0];
	return SpbWriteDataSynchronously16(&devContext->I2CContext, nreg, xbuf, bytes);
}

static NTSTATUS
mxt_write_reg(PDEVICE_CONTEXT  devContext, uint16_t reg, uint8_t val)
{
	return mxt_write_reg_buf(devContext, reg, &val, 1);
}

static NTSTATUS
mxt_write_object_off(PDEVICE_CONTEXT  devContext, struct mxt_object *obj,
	int offset, uint8_t val)
{
	uint16_t reg = obj->start_address;

	reg += offset;
	return mxt_write_reg(devContext, reg, val);
}

static
void
atmel_reset_device(PDEVICE_CONTEXT  devContext)
{
	mxt_write_object_off(devContext, devContext->cmdprocobj, MXT_CMDPROC_RESET_OFF, 1);
}

static NTSTATUS mxt_read_t9_resolution(PDEVICE_CONTEXT devContext)
{
	struct t9_range range;
	unsigned char orient;

	mxt_rollup core = devContext->core;
	mxt_object *resolutionobject = mxt_findobject(&core, MXT_TOUCH_MULTI_T9);

	mxt_read_reg(devContext, resolutionobject->start_address + MXT_T9_RANGE, &range, sizeof(range));

	mxt_read_reg(devContext, resolutionobject->start_address + MXT_T9_ORIENT, &orient, 1);

	/* Handle default values */
	if (range.x == 0)
		range.x = 1023;

	if (range.y == 0)
		range.y = 1023;

	if (orient & MXT_T9_ORIENT_SWITCH) {
		devContext->max_x = range.y + 1;
		devContext->max_y = range.x + 1;
	}
	else {
		devContext->max_x = range.x + 1;
		devContext->max_y = range.y + 1;
	}
	AtmlPadPrint(DEBUG_LEVEL_INFO, DBG_PNP, "Screen Size: X: %d Y: %d\n", devContext->max_x, devContext->max_y);
	return STATUS_SUCCESS;
}

static NTSTATUS mxt_read_t100_config(PDEVICE_CONTEXT devContext)
{
	uint16_t range_x, range_y;
	uint8_t cfg, tchaux;
	uint8_t aux;

	mxt_rollup core = devContext->core;
	mxt_object *resolutionobject = mxt_findobject(&core, MXT_TOUCH_MULTITOUCHSCREEN_T100);

	/* read touchscreen dimensions */
	mxt_read_reg(devContext, resolutionobject->start_address + MXT_T100_XRANGE, &range_x, sizeof(range_x));

	mxt_read_reg(devContext, resolutionobject->start_address + MXT_T100_YRANGE, &range_y, sizeof(range_y));

	/* read orientation config */
	mxt_read_reg(devContext, resolutionobject->start_address + MXT_T100_CFG1, &cfg, 1);

	if (cfg & MXT_T100_CFG_SWITCHXY) {
		devContext->max_x = range_y + 1;
		devContext->max_y = range_x + 1;
	}
	else {
		devContext->max_x = range_x + 1;
		devContext->max_y = range_y + 1;
	}

	mxt_read_reg(devContext, resolutionobject->start_address + MXT_T100_TCHAUX, &tchaux, 1);

	aux = 6;

	if (tchaux & MXT_T100_TCHAUX_VECT)
		devContext->t100_aux_vect = aux++;

	if (tchaux & MXT_T100_TCHAUX_AMPL)
		devContext->t100_aux_ampl = aux++;

	if (tchaux & MXT_T100_TCHAUX_AREA)
		devContext->t100_aux_area = aux++;
	AtmlPadPrint(DEBUG_LEVEL_INFO, DBG_PNP, "Screen Size T100: X: %d Y: %d\n", devContext->max_x, devContext->max_y);
	return STATUS_SUCCESS;
}

NTSTATUS BOOTTRACKPAD(
	_In_  PDEVICE_CONTEXT  devContext
	)
{
	int blksize;
	int totsize;
	uint32_t crc;
	mxt_rollup core = devContext->core;

	AtmlPadPrint(DEBUG_LEVEL_INFO, DBG_PNP, "Initializing Touch Screen.\n");

	mxt_read_reg(devContext, 0, &core.info, sizeof(core.info));

	core.nobjs = core.info.num_objects;

	if (core.nobjs < 0 || core.nobjs > 1024) {
		AtmlPadPrint(DEBUG_LEVEL_ERROR, DBG_PNP, "init_device nobjs (%d) out of bounds\n",
			core.nobjs);
	}

	blksize = sizeof(core.info) +
		core.nobjs * sizeof(struct mxt_object);
	totsize = blksize + sizeof(struct mxt_raw_crc);

	core.buf = (uint8_t *)ExAllocatePoolWithTag(NonPagedPool, totsize, ATMLPAD_POOL_TAG);

	mxt_read_reg(devContext, 0, core.buf, totsize);

	crc = obp_convert_crc((mxt_raw_crc *)((uint8_t *)core.buf + blksize));

	if (obp_crc24(core.buf, blksize) != crc) {
		AtmlPadPrint(DEBUG_LEVEL_ERROR, DBG_PNP,
			"init_device: configuration space "
			"crc mismatch %08x/%08x\n",
			crc, obp_crc24(core.buf, blksize));
	}
	else {
		AtmlPadPrint(DEBUG_LEVEL_ERROR, DBG_PNP, "CRC Matched!\n");
	}

	core.objs = (mxt_object *)((uint8_t *)core.buf +
		sizeof(core.info));

	devContext->msgprocobj = mxt_findobject(&core, MXT_GEN_MESSAGEPROCESSOR);
	devContext->cmdprocobj = mxt_findobject(&core, MXT_GEN_COMMANDPROCESSOR);

	devContext->core = core;

	int reportid = 1;
	for (int i = 0; i < core.nobjs; i++) {
		mxt_object *obj = &core.objs[i];
		uint8_t min_id, max_id;

		if (obj->num_report_ids) {
			min_id = reportid;
			reportid += obj->num_report_ids *
				mxt_obj_instances(obj);
			max_id = reportid - 1;
		}
		else {
			min_id = 0;
			max_id = 0;
		}

		switch (obj->type) {
		case MXT_GEN_MESSAGE_T5:
			if (devContext->info.family == 0x80 &&
				devContext->info.version < 0x20) {
				/*
				* On mXT224 firmware versions prior to V2.0
				* read and discard unused CRC byte otherwise
				* DMA reads are misaligned.
				*/
				devContext->T5_msg_size = mxt_obj_size(obj);
			}
			else {
				/* CRC not enabled, so skip last byte */
				devContext->T5_msg_size = mxt_obj_size(obj) - 1;
			}
			devContext->T5_address = obj->start_address;
			break;
		case MXT_GEN_COMMAND_T6:
			devContext->T6_reportid = min_id;
			devContext->T6_address = obj->start_address;
			break;
		case MXT_GEN_POWER_T7:
			devContext->T7_address = obj->start_address;
			break;
		case MXT_TOUCH_MULTI_T9:
			devContext->multitouch = MXT_TOUCH_MULTI_T9;
			devContext->T9_reportid_min = min_id;
			devContext->T9_reportid_max = max_id;
			devContext->num_touchids = obj->num_report_ids
				* mxt_obj_instances(obj);
			break;
		case MXT_SPT_MESSAGECOUNT_T44:
			devContext->T44_address = obj->start_address;
			break;
		case MXT_SPT_GPIOPWM_T19:
			devContext->T19_reportid = min_id;
			break;
		case MXT_TOUCH_MULTITOUCHSCREEN_T100:
			devContext->multitouch = MXT_TOUCH_MULTITOUCHSCREEN_T100;
			devContext->T100_reportid_min = min_id;
			devContext->T100_reportid_max = max_id;

			/* first two report IDs reserved */
			devContext->num_touchids = obj->num_report_ids - 2;
			break;
		}
		AtmlPadPrint(DEBUG_LEVEL_INFO, DBG_PNP, "Obj Type: %d\n", obj->type);
	}

	if (devContext->multitouch == MXT_TOUCH_MULTI_T9)
		mxt_read_t9_resolution(devContext);
	else if (devContext->multitouch == MXT_TOUCH_MULTITOUCHSCREEN_T100)
		mxt_read_t100_config(devContext);

	csgesture_softc *sc = &devContext->sc;
	sc->resx = devContext->max_x;
	sc->resy = devContext->max_y;

	sc->phyx = devContext->max_x;
	sc->phyy = devContext->max_y;

	sprintf(sc->product_id, "%d.0", core.info.family, core.info.variant);
	sprintf(sc->firmware_version, "%u.%u.%02X", core.info.version >> 4, core.info.version & 0xf, core.info.build);

	atmel_reset_device(devContext);

	return STATUS_SUCCESS;
}

NTSTATUS
OnD0Entry(
    _In_  WDFDEVICE               FxDevice,
    _In_  WDF_POWER_DEVICE_STATE  FxPreviousState
    )
/*++
 
  Routine Description:

    This routine allocates objects needed by the driver.

  Arguments:

    FxDevice - a handle to the framework device object
    FxPreviousState - previous power state

  Return Value:

    Status

--*/
{
    FuncEntry(TRACE_FLAG_WDFLOADING);
    
    UNREFERENCED_PARAMETER(FxPreviousState);

    PDEVICE_CONTEXT pDevice = GetDeviceContext(FxDevice);
    NTSTATUS status = STATUS_SUCCESS;

	WdfTimerStart(pDevice->Timer, WDF_REL_TIMEOUT_IN_MS(10));

	pDevice->RegsSet = false;
	pDevice->ConnectInterrupt = true;

    FuncExit(TRACE_FLAG_WDFLOADING);

    return status;
}

NTSTATUS
OnD0Exit(
    _In_  WDFDEVICE               FxDevice,
    _In_  WDF_POWER_DEVICE_STATE  FxPreviousState
    )
/*++
 
  Routine Description:

    This routine destroys objects needed by the driver.

  Arguments:

    FxDevice - a handle to the framework device object
    FxPreviousState - previous power state

  Return Value:

    Status

--*/
{
    FuncEntry(TRACE_FLAG_WDFLOADING);
    
    UNREFERENCED_PARAMETER(FxPreviousState);

    PDEVICE_CONTEXT pDevice = GetDeviceContext(FxDevice);

	WdfTimerStop(pDevice->Timer, TRUE);

	pDevice->ConnectInterrupt = false;

    FuncExit(TRACE_FLAG_WDFLOADING);

    return STATUS_SUCCESS;
}

VOID
OnTopLevelIoDefault(
    _In_  WDFQUEUE    FxQueue,
    _In_  WDFREQUEST  FxRequest
    )
/*++

  Routine Description:

    Accepts all incoming requests and pends or forwards appropriately.

  Arguments:

    FxQueue -  Handle to the framework queue object that is associated with the
        I/O request.
    FxRequest - Handle to a framework request object.

  Return Value:

    None.

--*/
{
    FuncEntry(TRACE_FLAG_SPBAPI);
    
    UNREFERENCED_PARAMETER(FxQueue);

    WDFDEVICE device;
    PDEVICE_CONTEXT pDevice;
    WDF_REQUEST_PARAMETERS params;
    NTSTATUS status;

    device = WdfIoQueueGetDevice(FxQueue);
    pDevice = GetDeviceContext(device);

    WDF_REQUEST_PARAMETERS_INIT(&params);

    WdfRequestGetParameters(FxRequest, &params);

	status = WdfRequestForwardToIoQueue(FxRequest, pDevice->SpbQueue);

	if (!NT_SUCCESS(status))
	{
		AtmlPadPrint(
			DEBUG_LEVEL_ERROR,
			DBG_IOCTL,
			"Failed to forward WDFREQUEST %p to SPB queue %p - %!STATUS!",
			FxRequest,
			pDevice->SpbQueue,
			status);
		
		WdfRequestComplete(FxRequest, status);
	}

    FuncExit(TRACE_FLAG_SPBAPI);
}

VOID
OnIoDeviceControl(
    _In_  WDFQUEUE    FxQueue,
    _In_  WDFREQUEST  FxRequest,
    _In_  size_t      OutputBufferLength,
    _In_  size_t      InputBufferLength,
    _In_  ULONG       IoControlCode
    )
/*++
Routine Description:

    This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
    requests from the system.

Arguments:

    FxQueue - Handle to the framework queue object that is associated
        with the I/O request.
    FxRequest - Handle to a framework request object.
    OutputBufferLength - length of the request's output buffer,
        if an output buffer is available.
    InputBufferLength - length of the request's input buffer,
        if an input buffer is available.
    IoControlCode - the driver-defined or system-defined I/O control code
        (IOCTL) that is associated with the request.

Return Value:

   VOID

--*/
{
    FuncEntry(TRACE_FLAG_SPBAPI);

    WDFDEVICE device;
    PDEVICE_CONTEXT pDevice;
    BOOLEAN fSync = FALSE;
    NTSTATUS status = STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

	device = WdfIoQueueGetDevice(FxQueue);
	pDevice = GetDeviceContext(device);

	AtmlPadPrint(
		DEBUG_LEVEL_INFO, DBG_IOCTL,
        "DeviceIoControl request %p received with IOCTL=%lu",
        FxRequest,
        IoControlCode);
	AtmlPadPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
		"%s, Queue:0x%p, Request:0x%p\n",
		DbgHidInternalIoctlString(IoControlCode),
		FxQueue,
		FxRequest
		);

    //
    // Translate the test IOCTL into the appropriate 
    // SPB API method.  Open and close are completed 
    // synchronously.
    //

    switch (IoControlCode)
    {
	case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
		//
		// Retrieves the device's HID descriptor.
		//
		status = AtmlPadGetHidDescriptor(device, FxRequest);
		fSync = TRUE;
		break;

	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
		//
		//Retrieves a device's attributes in a HID_DEVICE_ATTRIBUTES structure.
		//
		status = AtmlPadGetDeviceAttributes(FxRequest);
		fSync = TRUE;
		break;

	case IOCTL_HID_GET_REPORT_DESCRIPTOR:
		//
		//Obtains the report descriptor for the HID device.
		//
		status = AtmlPadGetReportDescriptor(device, FxRequest);
		fSync = TRUE;
		break;

	case IOCTL_HID_GET_STRING:
		//
		// Requests that the HID minidriver retrieve a human-readable string
		// for either the manufacturer ID, the product ID, or the serial number
		// from the string descriptor of the device. The minidriver must send
		// a Get String Descriptor request to the device, in order to retrieve
		// the string descriptor, then it must extract the string at the
		// appropriate index from the string descriptor and return it in the
		// output buffer indicated by the IRP. Before sending the Get String
		// Descriptor request, the minidriver must retrieve the appropriate
		// index for the manufacturer ID, the product ID or the serial number
		// from the device extension of a top level collection associated with
		// the device.
		//
		status = AtmlPadGetString(FxRequest);
		fSync = TRUE;
		break;

	case IOCTL_HID_WRITE_REPORT:
	case IOCTL_HID_SET_OUTPUT_REPORT:
		//
		//Transmits a class driver-supplied report to the device.
		//
		status = AtmlPadWriteReport(pDevice, FxRequest);
		fSync = TRUE;
		break;

	case IOCTL_HID_READ_REPORT:
	case IOCTL_HID_GET_INPUT_REPORT:
		//
		// Returns a report from the device into a class driver-supplied buffer.
		// 
		status = AtmlPadReadReport(pDevice, FxRequest, &fSync);
		break;

	case IOCTL_HID_GET_FEATURE:
		//
		// returns a feature report associated with a top-level collection
		//
		status = AtmlPadGetFeature(pDevice, FxRequest, &fSync);
		break;
	case IOCTL_HID_ACTIVATE_DEVICE:
		//
		// Makes the device ready for I/O operations.
		//
	case IOCTL_HID_DEACTIVATE_DEVICE:
		//
		// Causes the device to cease operations and terminate all outstanding
		// I/O requests.
		//
    default:
        fSync = TRUE;
		status = STATUS_NOT_SUPPORTED;
		AtmlPadPrint(
			DEBUG_LEVEL_INFO, DBG_IOCTL,
            "Request %p received with unexpected IOCTL=%lu",
            FxRequest,
            IoControlCode);
    }

    //
    // Complete the request if necessary.
    //

    if (fSync)
    {
		AtmlPadPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
			"%s completed, Queue:0x%p, Request:0x%p\n",
			DbgHidInternalIoctlString(IoControlCode),
			FxQueue,
			FxRequest
			);

        WdfRequestComplete(FxRequest, status);
	}
	else {
		AtmlPadPrint(DEBUG_LEVEL_INFO, DBG_IOCTL,
			"%s deferred, Queue:0x%p, Request:0x%p\n",
			DbgHidInternalIoctlString(IoControlCode),
			FxQueue,
			FxRequest
			);
	}

    FuncExit(TRACE_FLAG_SPBAPI);
}