#include "Source.h"

// Our new device object
PDEVICE_OBJECT SKDDevice = NULL;

// System keyboard device name path
UNICODE_STRING TargetDeviceName = RTL_CONSTANT_STRING(L"\\Device\\KeyboardClass0");

// Our device object name
UNICODE_STRING SKDDeviceName = RTL_CONSTANT_STRING(L"SKD Device");


// When driver loaded invoke DriverEntry function
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {

	NTSTATUS ntstatus;
	DriverObject->DriverUnload = DriverExit;

	// Set all IRP request handler
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DispatchPass;
	}

	// Set read IRP request handler
	DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;

	ntstatus = SKDAttachDevice(DriverObject);

	if (!NT_SUCCESS(ntstatus)) {
		DbgPrint("SKD> Attachment Faild ...\r\n");
	}
	else {
		DbgPrint("SKD> Attachment Success ...\r\n");
		DbgPrint("SKD> Simple Keyboard Driver (SKD) Loaded ...\r\n");
	}
	return ntstatus;
}

// When driver unloaded invoke DriverExit function
NTSTATUS DriverExit(PDRIVER_OBJECT DriverObject) {
	LARGE_INTEGER interval = { 0 };

	interval.QuadPart = -10 * 1000 * 1000;

	IoDetachDevice(((PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension)->LowerKeyboardDevice);

	while (pending) {
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}
	IoDeleteDevice(SKDDevice);
	
	DbgPrint("SKD> Simple Keyboard Driver (SKD) Unloaded ...\r\n");
	
	return STATUS_SUCCESS;
}

NTSTATUS SKDAttachDevice(PDRIVER_OBJECT DriverObject) {

	NTSTATUS ntstatus;

	// Create our own device
	ntstatus = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_KEYBOARD, 0, FALSE, &SKDDevice);

	// Creation faild
	if (!NT_SUCCESS(ntstatus)) {
		DbgPrint("SKD> SKD Device Creation Faild ...\r\n");
		return ntstatus;
	}

	// Assign new created device flags
	SKDDevice->Flags |= DO_BUFFERED_IO;
	SKDDevice->Flags &= ~DO_DEVICE_INITIALIZING;

	// Initialize our new device extension structure
	RtlZeroMemory(SKDDevice->DeviceExtension, sizeof(DEVICE_EXTENSION));

	// Attach our new device (SKDKeyboardDevice) to system device (KeyboardClass0)
	ntstatus = IoAttachDevice(SKDDevice, &TargetDeviceName, &((PDEVICE_EXTENSION)SKDDevice->DeviceExtension)->LowerKeyboardDevice);

	// Attachment faild
	if (!NT_SUCCESS(ntstatus)) {
		DbgPrint("SKD> SKD Device Attachment Faild ...\r\n");
		IoDeleteDevice(SKDDevice);
		return ntstatus;
	}

	return ntstatus;
}

// When have a other IRP invoke this function
NTSTATUS DispatchPass(PDEVICE_OBJECT DeviceObject, PIRP irp) {

	NTSTATUS ntstatus;

	// Prepare the next stack location for next IRP
	IoCopyCurrentIrpStackLocationToNext(irp);

	// Pass IRP to next level
	ntstatus = IoCallDriver(((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerKeyboardDevice, irp);

	return ntstatus;
}

// When have a read IRP invoke this function
NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP irp) {

	NTSTATUS ntstatus;

	// Prepare the next stack location for next IRP
	IoCopyCurrentIrpStackLocationToNext(irp);

	IoSetCompletionRoutine(irp, ReadCompleteRoutine, NULL, TRUE, TRUE, TRUE);

	pending++;

	// Pass IRP to next level
	ntstatus = IoCallDriver(((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerKeyboardDevice, irp);

	return ntstatus;
}

// Main routine when invoke if any key press
NTSTATUS ReadCompleteRoutine(PDEVICE_OBJECT DeviceObject, PIRP irp, PVOID Context) {
	PKEYBOARD_INPUT_DATA Keys;
	Keys = (PKEYBOARD_INPUT_DATA)irp->AssociatedIrp.SystemBuffer;

	// TODO: Keys might be array of keys which can handle multiple key press at the same time
	// int keys_num = irp->IoStatus.Information / sizeof(KEYBOARD_INPUT_DATA);
	// for (int i = 0; i < keys_num; i++) {}

	// Check IRP request status
	if (irp->IoStatus.Status == STATUS_SUCCESS) {
		// Print the scan code of pressed key
		DbgPrint("SKD> Key %s %s\n", key_ascii[Keys->MakeCode], key_flags[Keys->Flags]);
		
	}

	if (irp->PendingReturned) {
		IoMarkIrpPending(irp);
	}

	pending--;

	return irp->IoStatus.Status;
}