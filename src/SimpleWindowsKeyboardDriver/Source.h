#include <ntddk.h>
#pragma once

char* key_ascii[] = { "", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "BACKSPACE", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "Enter", "CTRL", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`", "LShift", "\\", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "RShift", "PRTSC", "ALT", "SPACE", "CAPS", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUM", "SCROLL", "HOME", "UP", "PGUP", "GRAY-", "LEFT", "CENTER", "RIGHT", "GRAY+", "END", "DOWN", "PGDN", "INS", "DEL", "", "", "", "F11", "F12"};

char* key_flags[4] = { "Pressed", "Released", "Extended 0", "Extended 1" };

ULONG pending = 0;

typedef struct {
	PDEVICE_OBJECT LowerKeyboardDevice;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

typedef struct _KEYBOARD_INPUT_DATA {
	USHORT UnitId;
	USHORT MakeCode; // Specifies the scan code associated with a key press.
	USHORT Flags; // KEY_MAKE: The key was pressed		KEY_BREAK: The key was released.
	USHORT Reserved;
	ULONG  ExtraInformation;
} KEYBOARD_INPUT_DATA, * PKEYBOARD_INPUT_DATA;


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

NTSTATUS DriverExit(PDRIVER_OBJECT DriverObject);

NTSTATUS SKDAttachDevice(PDRIVER_OBJECT DriverObject);

NTSTATUS DispatchPass(PDEVICE_OBJECT DeviceObject, PIRP irp);

NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP irp);

NTSTATUS ReadCompleteRoutine(PDEVICE_OBJECT DeviceObject, PIRP irp, PVOID Context);