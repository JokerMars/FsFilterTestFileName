/*++

Module Name:

    FsFilterTestFileName.c

Abstract:

    This is the main module of the FsFilterTestFileName miniFilter driver.

Environment:

    Kernel mode

--*/

#include "Common.h"
#include "Strategy.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


PFLT_FILTER gFilterHandle;
ULONG_PTR OperationStatusCtx = 1;
ULONG offset;

#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

ULONG gTraceFlags = 0;


#define PT_DBG_PRINT( _dbgLevel, _string )          \
    (FlagOn(gTraceFlags,(_dbgLevel)) ?              \
        DbgPrint _string :                          \
        ((int)0))

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

//default left
DRIVER_INITIALIZE DriverEntry;
NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

NTSTATUS
FsFilterTestFileNameInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

VOID
FsFilterTestFileNameInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
FsFilterTestFileNameInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

NTSTATUS
FsFilterTestFileNameUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    );

NTSTATUS
FsFilterTestFileNameInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );


//My designed protocol

FLT_PREOP_CALLBACK_STATUS
PreCreate(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext);

FLT_POSTOP_CALLBACK_STATUS
PostCreate(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_opt_ PVOID CompletionContext, _In_ FLT_POST_OPERATION_FLAGS Flags);

//FLT_PREOP_CALLBACK_STATUS
//PreRead(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects,
//	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext0);
//
//FLT_POSTOP_CALLBACK_STATUS
//PostRead(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects,
//	_In_opt_ PVOID CompletionContext, _In_ FLT_POST_OPERATION_FLAGS Flags);


EXTERN_C_END

//My declared function

ULONG GetProcessNameOffset();

PCHAR GetCurrentProcessName();

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, FsFilterTestFileNameUnload)
#pragma alloc_text(PAGE, FsFilterTestFileNameInstanceQueryTeardown)
#pragma alloc_text(PAGE, FsFilterTestFileNameInstanceSetup)
#pragma alloc_text(PAGE, FsFilterTestFileNameInstanceTeardownStart)
#pragma alloc_text(PAGE, FsFilterTestFileNameInstanceTeardownComplete)
#endif

//
//  operation registration
//

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
	{
		IRP_MJ_CREATE,
		0,
		PreCreate,
		PostCreate
	},

	//{
	//	IRP_MJ_READ,
	//	0,
	//	PreRead,
	//	PostRead
	//},

    { IRP_MJ_OPERATION_END }
};

//
//  This defines what we want to filter with FltMgr
//

CONST FLT_REGISTRATION FilterRegistration = {

    sizeof( FLT_REGISTRATION ),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    FsFilterTestFileNameUnload,                           //  MiniFilterUnload

    FsFilterTestFileNameInstanceSetup,                    //  InstanceSetup
    FsFilterTestFileNameInstanceQueryTeardown,            //  InstanceQueryTeardown
    FsFilterTestFileNameInstanceTeardownStart,            //  InstanceTeardownStart
    FsFilterTestFileNameInstanceTeardownComplete,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL                                //  NormalizeNameComponent

};



NTSTATUS
FsFilterTestFileNameInstanceSetup (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
/*++

Routine Description:

    This routine is called whenever a new instance is created on a volume. This
    gives us a chance to decide if we need to attach to this volume or not.

    If this routine is not defined in the registration structure, automatic
    instances are always created.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Flags describing the reason for this attach request.

Return Value:

    STATUS_SUCCESS - attach
    STATUS_FLT_DO_NOT_ATTACH - do not attach

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );
    UNREFERENCED_PARAMETER( VolumeDeviceType );
    UNREFERENCED_PARAMETER( VolumeFilesystemType );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("FsFilterTestFileName!FsFilterTestFileNameInstanceSetup: Entered\n") );

    return STATUS_SUCCESS;
}


NTSTATUS
FsFilterTestFileNameInstanceQueryTeardown (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This is called when an instance is being manually deleted by a
    call to FltDetachVolume or FilterDetach thereby giving us a
    chance to fail that detach request.

    If this routine is not defined in the registration structure, explicit
    detach requests via FltDetachVolume or FilterDetach will always be
    failed.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Indicating where this detach request came from.

Return Value:

    Returns the status of this operation.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("FsFilterTestFileName!FsFilterTestFileNameInstanceQueryTeardown: Entered\n") );

    return STATUS_SUCCESS;
}


VOID
FsFilterTestFileNameInstanceTeardownStart (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the start of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("FsFilterTestFileName!FsFilterTestFileNameInstanceTeardownStart: Entered\n") );
}


VOID
FsFilterTestFileNameInstanceTeardownComplete (
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
/*++

Routine Description:

    This routine is called at the end of instance teardown.

Arguments:

    FltObjects - Pointer to the FLT_RELATED_OBJECTS data structure containing
        opaque handles to this filter, instance and its associated volume.

    Flags - Reason why this instance is being deleted.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("FsFilterTestFileName!FsFilterTestFileNameInstanceTeardownComplete: Entered\n") );
}


/*************************************************************************
    MiniFilter initialization and unload routines.
*************************************************************************/

NTSTATUS
DriverEntry (
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    This is the initialization routine for this miniFilter driver.  This
    registers with FltMgr and initializes all global data structures.

Arguments:

    DriverObject - Pointer to driver object created by the system to
        represent this driver.

    RegistryPath - Unicode string identifying where the parameters for this
        driver are located in the registry.

Return Value:

    Routine can return non success error codes.

--*/
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("FsFilterTestFileName!DriverEntry: Entered\n") );

    //
    //  Register with FltMgr to tell it our callback routines
    //

    status = FltRegisterFilter( DriverObject,
                                &FilterRegistration,
                                &gFilterHandle );

    FLT_ASSERT( NT_SUCCESS( status ) );

    if (NT_SUCCESS( status )) {

        //
        //  Start filtering i/o
        //

        status = FltStartFiltering( gFilterHandle );

        if (!NT_SUCCESS( status )) {

            FltUnregisterFilter( gFilterHandle );
        }
    }

	offset = GetProcessNameOffset();
	DbgPrint("the process name offset:  %d\n", offset);

	PCHAR str = ".txt=notepad.exe,TxtReader.exe,;.jpg=ImageView.exe,explore.exe,;";

	PFILE_TYPE_PROCESS head = GetStrategyFromString(str);

	OutputStrategy(head);

	FreeStrategy(head);
	
    return status;
}

NTSTATUS
FsFilterTestFileNameUnload (
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
    )
/*++

Routine Description:

    This is the unload routine for this miniFilter driver. This is called
    when the minifilter is about to be unloaded. We can fail this unload
    request if this is not a mandatory unload indicated by the Flags
    parameter.

Arguments:

    Flags - Indicating if this is a mandatory unload.

Return Value:

    Returns STATUS_SUCCESS.

--*/
{
    UNREFERENCED_PARAMETER( Flags );

    PAGED_CODE();

    PT_DBG_PRINT( PTDBG_TRACE_ROUTINES,
                  ("FsFilterTestFileName!FsFilterTestFileNameUnload: Entered\n") );

    FltUnregisterFilter( gFilterHandle );

    return STATUS_SUCCESS;
}


ULONG GetProcessNameOffset()
{
	ULONG i;

	PEPROCESS curproc = PsGetCurrentProcess();
	for (i = 0; i < 3 * PAGE_SIZE; i++)
	{
		if (!strncmp("System", (PCHAR)curproc + i, strlen("System")))
		{
			return i;
		}
	}
	return 0;
}

PCHAR GetCurrentProcessName()
{
	PCHAR name = NULL;
	PEPROCESS curproc = PsGetCurrentProcess();

	if (offset)
	{
		name = (PCHAR)curproc + offset;
	}
	return name;
}


FLT_PREOP_CALLBACK_STATUS
PreCreate(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext)
{
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	FLT_PREOP_CALLBACK_STATUS retValue = FLT_PREOP_SUCCESS_WITH_CALLBACK;

	if (KeGetCurrentIrql() >= DISPATCH_LEVEL)
		return retValue;
	return retValue;
}


FLT_POSTOP_CALLBACK_STATUS
PostCreate(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_opt_ PVOID CompletionContext, _In_ FLT_POST_OPERATION_FLAGS Flags)
{
	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(Flags);

	FLT_POSTOP_CALLBACK_STATUS retVal = FLT_POSTOP_FINISHED_PROCESSING;

	if (KeGetCurrentIrql() >= DISPATCH_LEVEL)
	{
		return retVal;
	}

	//Find which process is dealing with the current file
	PCHAR procName = GetCurrentProcessName();

	if (strncmp(procName, "notepad.exe", strlen("notepad.exe")) != 0)
		return retVal;

	DbgPrint("Post Create: %s", procName);
	

	//Get the file name we are dealing with
	PFLT_FILE_NAME_INFORMATION nameInfo = NULL;

	BOOLEAN isDir;
	NTSTATUS status;
	status = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &isDir);

	if (NT_SUCCESS(status))
	{
		if (isDir)
		{
			DbgPrint("It's a directory");
			return retVal;
		}
		else
		{
			status = FltGetFileNameInformation(Data, FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &nameInfo);
			if (!NT_SUCCESS(status))
			{
				DbgPrint("Get file name info error");
				return retVal;
			}

			FltParseFileNameInformation(nameInfo);

			DbgPrint("file name is %wZ", &(nameInfo->Name));

			
		}
	}
	return retVal;
}

//
//FLT_PREOP_CALLBACK_STATUS
//PreRead(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects,
//	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext)
//{
//
//}
//
//
//FLT_POSTOP_CALLBACK_STATUS
//PostRead(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects,
//	_In_opt_ PVOID CompletionContext, _In_ FLT_POST_OPERATION_FLAGS Flags)
//{
//
//}