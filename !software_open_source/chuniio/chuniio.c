#include <windows.h>

#include <process.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "hidsdi.h"

#include "chuniio.h"
#include "config.h"
#include "hid_impl.h"

#define INPUT_REPORT_ID 0x00
#define OUTPUT_REPORT_ID 0x00

#define DEBUG 0

static unsigned int __stdcall chuni_io_slider_thread_proc(void *ctx);

static bool chuni_io_coin;
static uint16_t chuni_io_coins;
static uint8_t chuni_io_hand_pos;
static HANDLE chuni_io_slider_thread;
static bool chuni_io_slider_stop_flag;
static struct chuni_io_config chuni_io_cfg;

static HANDLE g_device_handle;

#pragma pack(1)
typedef struct input_report_s {
	uint8_t  report_id; // 0x00
	uint8_t  beams; 
	uint8_t  opbtn;     // 0b00000CST (Left = Coin, Middle = Service, Right = Test)
	uint8_t  slider[32];
} input_report_t;

input_report_t g_controller_data;

__declspec(dllexport) uint16_t __cdecl chuni_io_get_api_version(void)
{
    return 0x0101;
}

static int controller_read_buttons(){
	/* read hid report */
	(void) hid_get_report(g_device_handle, (uint8_t *)&g_controller_data, INPUT_REPORT_ID, sizeof(input_report_t));

	
	return 0;
}


static int controller_write_leds(const uint8_t *lamp_bits){
	//static uint8_t led_data3[6]; /* tower leds? */

	uint8_t yuanled[62] = {0};
	
	for (int i = 0; i<31; i++)
	{
		yuanled[2*i]    = (lamp_bits[3*i]   >> 3);      /* 5 bit blue */
		yuanled[2*i]   |= (lamp_bits[3*i+1] >> 2) << 5; /* 3 bit red */  
		yuanled[2*i+1]  = (lamp_bits[3*i+1] >> 5);      /* 3 bit red cont. */  
		yuanled[2*i+1] |= (lamp_bits[3*i+2] >> 3) << 3; /* 6 bit green */  
	}

	hid_set_report(g_device_handle, (const uint8_t *)yuanled, OUTPUT_REPORT_ID, 62);
	
	return 0;
}

__declspec(dllexport) HRESULT __cdecl chuni_io_jvs_init(void)
{
    chuni_io_config_load(&chuni_io_cfg, L".\\segatools.ini");
	if ( hid_open_device(&g_device_handle, 0x1973, 0x2001) != 0 )
	{
        return -1;		
	}

	int hidres = HidD_SetNumInputBuffers(g_device_handle, 2);
    if (!hidres)
    {
        printf("Error %d setnuminputbuff\r\n",GetLastError());
        return -1;
    }
	
    return S_OK;
}

__declspec(dllexport) void __cdecl chuni_io_jvs_read_coin_counter(uint16_t *out)
{
    if (out == NULL) {
        return;
    }

    if (GetAsyncKeyState(chuni_io_cfg.vk_coin) || (g_controller_data.opbtn>>2)&1 ) {
        if (!chuni_io_coin) {
            chuni_io_coin = true;
            chuni_io_coins++;
        }
    } else {
        chuni_io_coin = false;
    }

    *out = chuni_io_coins;
}

__declspec(dllexport) void __cdecl chuni_io_jvs_poll(uint8_t *opbtn, uint8_t *beams)
{
    size_t i;

    if (GetAsyncKeyState(chuni_io_cfg.vk_test)) {
        *opbtn |= 0x01; /* Test */
    }

    if (GetAsyncKeyState(chuni_io_cfg.vk_service)) {
        *opbtn |= 0x02; /* Service */
    }
	
	*opbtn |= g_controller_data.opbtn & 0x03;
	
    if (GetAsyncKeyState(chuni_io_cfg.vk_ir)) {
        if (chuni_io_hand_pos < 6) {
            chuni_io_hand_pos++;
        }
    } else {
        if (chuni_io_hand_pos > 0) {
            chuni_io_hand_pos--;
        }
    }

    for (i = 0 ; i < 6 ; i++) {
        if (chuni_io_hand_pos > i) {
            *beams |= (1 << i);
        }
    }
	
	*beams |= g_controller_data.beams & 0x3F;
}

__declspec(dllexport) HRESULT __cdecl chuni_io_slider_init(void)
{
    return S_OK;
}

__declspec(dllexport) void __cdecl chuni_io_slider_start(chuni_io_slider_callback_t callback)
{
    if (chuni_io_slider_thread != NULL) {
        return;
    }

    chuni_io_slider_thread = (HANDLE) _beginthreadex(
            NULL,
            0,
            chuni_io_slider_thread_proc,
            callback,
            0,
            NULL);
}

__declspec(dllexport) void __cdecl chuni_io_slider_stop(void)
{
    if (chuni_io_slider_thread == NULL) {
        return;
    }

    chuni_io_slider_stop_flag = true;

    WaitForSingleObject(chuni_io_slider_thread, INFINITE);
    CloseHandle(chuni_io_slider_thread);
    chuni_io_slider_thread = NULL;
    chuni_io_slider_stop_flag = false;
}

__declspec(dllexport) void __cdecl chuni_io_slider_set_leds(const uint8_t *rgb)
{
	controller_write_leds(rgb);
}

static unsigned int __stdcall chuni_io_slider_thread_proc(void *ctx)
{
    chuni_io_slider_callback_t callback;
    uint8_t pressure[32];
    size_t i;

    callback = ctx;

    while (!chuni_io_slider_stop_flag) {
		controller_read_buttons();
		for (int i=0; i<32; i++)
		{
			pressure[i] = g_controller_data.slider[31-i];
		}
        for (i = 0 ; i < 32; i++) {
            if (GetAsyncKeyState(chuni_io_cfg.vk_cell[i]) & 0x8000) {
                pressure[i] = 128;
            }
        }
        callback(pressure);
    }

    return 0;
}
