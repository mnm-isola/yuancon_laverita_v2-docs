#pragma once

#include <stddef.h>
#include <stdint.h>

int hid_open_device(HANDLE *device_handle, uint16_t vid, uint16_t pid);
int hid_get_report(HANDLE device_handle, uint8_t *buf, uint8_t report_id, uint8_t nb_bytes);
int hid_set_report(HANDLE device_handle, const uint8_t *buf, uint8_t report_id, uint8_t nb_bytes);