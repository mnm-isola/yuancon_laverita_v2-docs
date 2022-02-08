#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

// This file is in the Windows DDK available from Microsoft.
#include <hidsdi.h>
#include <SetupAPI.h>

#include "hid_impl.h"

#define DEBUG 0

// GUID_DEVCLASS_HIDCLASS
static GUID hidclass_guid = {0x745a17a0, 0x74d3, 0x11d0, 0xb6, 0xfe, 0x00, 0xa0, 0xc9, 0x0f, 0x57, 0xda};
    
static BOOLEAN get_device_path(char *lPath, uint16_t vid, uint16_t pid)
{
    const GUID *guid = &hidclass_guid;
    HidD_GetHidGuid(&hidclass_guid);
// Get device interface info set handle
// for all devices attached to system
    HDEVINFO hDevInfo = SetupDiGetClassDevs(guid, NULL, NULL,  DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); // Function class devices.
    if(hDevInfo == INVALID_HANDLE_VALUE)
        return FALSE;

// Retrieve a context structure for a device interface of a device information set.
    BYTE                             buf[1024];
    PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)buf;
    SP_DEVICE_INTERFACE_DATA         spdid;
    SP_DEVINFO_DATA                  spdd;
    DWORD                            dwSize;
	char vidpidstr[64];
	
	sprintf(vidpidstr, "vid_%04x&pid_%04x", vid, pid);

	#if DEBUG == 1
printf("\r\nlooking for substring %s in device path\r\n", vidpidstr);	
	#endif
    spdid.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

// Iterate through all the interfaces and try to match one based on
// the device number.
    DWORD device_index = 0;
    
    for(DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, NULL,guid, i, &spdid); i++)
    {
    // Get the device path.
        dwSize = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize, NULL);
        if(dwSize == 0 || dwSize > sizeof(buf))
            continue;
        
        pspdidd->cbSize = sizeof(*pspdidd);
        ZeroMemory((PVOID)&spdd, sizeof(spdd));
        spdd.cbSize = sizeof(spdd);
        if(!SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, pspdidd,
                                            dwSize, &dwSize, &spdd))
            continue;
        
        /* check if the device contains our wanted vid/pid */
        if ( strstr( pspdidd->DevicePath, vidpidstr ) == NULL )
        {
            continue;
        }
#if DEBUG == 1
        printf("\r\nLaverita found at %s\r\n", &pspdidd->DevicePath);
#endif
        //copy devpath into lPath
        strcpy(lPath, pspdidd->DevicePath);
        SetupDiDestroyDeviceInfoList(hDevInfo);
        return TRUE;
     }
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return FALSE;
}

/**
 * Initialize the g_hid_handle global variable
 * It will attempt to read paths from devicepath.dat file if it exists until it finds one which works.
 * If nothing is found, as a last resort it will try the two default paths from the Due and Leonardo
 * versions of the firmware.
 *
 * @return 0 on success, -1 on error
 */
int hid_open_device(HANDLE *device_handle, uint16_t vid, uint16_t pid){
    static uint8_t err_count = 0;
    char path[256];
	
    if (!get_device_path(path, vid, pid))
    {
#if DEBUG == 1
        printf("\r\nLaverita not detected.\r\n");
#endif
        err_count++;
        if (err_count > 2){
            printf("Could not init device after multiple attempts. Exiting.\r\n");
            exit(1);
        }
        return -1;
    }

    *device_handle = CreateFile(path, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    
    if ( *device_handle == INVALID_HANDLE_VALUE )
    {
#if DEBUG == 1
        printf("Could not open detected device (err = %x).\r\n", GetLastError());
#endif
        return -1;
    }
    return 0;
}

int hid_get_report(HANDLE device_handle, uint8_t *buf, uint8_t report_id, uint8_t nb_bytes)
{
    DWORD          bytesRead = 0;
	static uint8_t tmp_buf[128];
	
	if (buf == NULL) return -1;
	
    tmp_buf[0] = report_id;

    ReadFile(device_handle, tmp_buf, nb_bytes*2, &bytesRead, NULL);
    // bytesRead should either be nb_bytes*2 (if it successfully read 2 reports) or nb_bytes (only one)
    if ( bytesRead != nb_bytes*2 && bytesRead != nb_bytes )
    {
#ifdef DEBUG
        printf("HID read error (expected %u (or twice that), but got %u bytes)\n",nb_bytes, bytesRead);
#endif
        return -1;
    }

    /* HID read ok, copy latest report bytes */
    memcpy(buf, tmp_buf + bytesRead - nb_bytes, nb_bytes);
	
    return 0;	
}


int hid_set_report(HANDLE device_handle, const uint8_t *buf, uint8_t report_id, uint8_t nb_bytes)
{
    DWORD     bytesWritten = 0;
	static uint8_t tmp_buf[128];
	
	tmp_buf[0] = report_id;
	memcpy(tmp_buf+1, buf, nb_bytes);
	
    /* send HID Report */
    WriteFile(device_handle, tmp_buf, 63, &bytesWritten, NULL); //TODO: fix this, get 63 value from the device getcaps
	
#if DEBUG == 1
        printf("Wrote %u bytes (expected %u) to report id %d. (err = %d)\r\n", bytesWritten, 63, report_id, GetLastError());
#endif

    if ( bytesWritten == 63 )
        return 0;

    return -1;
}