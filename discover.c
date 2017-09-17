/*
 * Discovery of BLE advertisements
 * Use passive scans to discover BLE devices.
 *
 * Henric Lindén, rt-labs AB
 */

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "btstack.h"


static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_timer_source_t timer;
static int scan_time = 2000;

static void timer_handler(btstack_timer_source_t* ts)
{
    UNUSED(ts);

    hci_power_control(HCI_POWER_OFF);
    hci_close();
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);

    if (packet_type == HCI_EVENT_PACKET) {
        switch (hci_event_packet_get_type(packet)) {
            case BTSTACK_EVENT_STATE:
                if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                    gap_start_scan();
                    btstack_run_loop_set_timer(&timer, scan_time);
                    btstack_run_loop_set_timer_handler(&timer, timer_handler);
                    btstack_run_loop_add_timer(&timer);
                }
                break;

            case GAP_EVENT_ADVERTISING_REPORT: {
                bd_addr_t address;
                gap_event_advertising_report_get_address(packet, address);
                uint8_t event_type = gap_event_advertising_report_get_advertising_event_type(packet);
                uint8_t address_type = gap_event_advertising_report_get_address_type(packet);
                int8_t rssi = gap_event_advertising_report_get_rssi(packet);
                uint8_t length = gap_event_advertising_report_get_data_length(packet);
                const uint8_t* data = gap_event_advertising_report_get_data(packet);

                //printf("evt-type %u, addr-type %u, addr %s, rssi %d, data[%u] ", event_type, address_type, bd_addr_to_str(address), rssi, length);
                printf("%u, %u, %s, %d, %u, ", event_type, address_type, bd_addr_to_str(address), rssi, length);
                for (int i = 0; i < length; i++) {
                    printf("%02x ", data[i]);
                }
                for (int i = 0; i < length; i++) {
                    printf("%c", isprint(data[i]) ? data[i] : '.');
                }
                printf("\n");
                break;
            }
            default:
                break;
        }
    }
}

int btstack_main(int argc, const char* argv[]);
int btstack_main(int argc, const char* argv[])
{
    int st;

    // Check if scan time is specified and is within a reasonable range.
    if (argc == 2) {
        if (sscanf(argv[1], "%u", &st)) {
            if (st > 0 && st < 60*1000) {
                scan_time = st;
                fprintf(stderr, "Scan time: %u\n", scan_time);
            }
        }
    }

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Active scanning, 100% (scan interval = scan window)
    gap_set_scan_parameters(1, 48, 48);

    // Power on = start
    hci_power_control(HCI_POWER_ON);

    return 0;
}
