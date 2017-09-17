/*
 * 1. Connect to an serial port adapter from connecBlue / u-blox
 * 2. Get/download 10k of data
 * 3. Disconenct
 *
 * Henric Lindén, rt-labs AB
 */

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#include "btstack.h"


static btstack_packet_callback_registration_t hci_event_callback_registration;
static bd_addr_t cmdline_addr = { };
static hci_con_handle_t connection_handle;

static int fp = 0;
static int nr_received_bytes = 0;

// connectBlue serialport service.
static uint8_t serialport_service_uuid[] = {0x24, 0x56, 0xe1, 0xb9, 0x26, 0xe2, 0x8f, 0x83, 0xe7, 0x44, 0xf3, 0x4f, 0x01, 0xe9, 0xd7, 0x01};
static uint8_t serialport_fifo_characteristic_uuid[] = {0x24, 0x56, 0xe1, 0xb9, 0x26, 0xe2, 0x8f, 0x83, 0xe7, 0x44, 0xf3, 0x4f, 0x01, 0xe9, 0xd7, 0x03};
//static uint8_t serialport_credits_characteristic_uuid[] = {0x24, 0x56, 0xe1, 0xb9, 0x26, 0xe2, 0x8f, 0x83, 0xe7, 0x44, 0xf3, 0x4f, 0x01, 0xe9, 0xd7, 0x04};

static gatt_client_service_t serialport_service;
static gatt_client_characteristic_t fifo_characteristic;
static gatt_client_notification_t notification_registration;


static void gatt_data_event_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size)
{
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    const uint8_t* data;
    int len;

    switch (hci_event_packet_get_type(packet)) {
        case GATT_EVENT_NOTIFICATION:
            data = gatt_event_notification_get_value(packet);
            len = gatt_event_notification_get_value_length(packet);
            write(fp, data, len);
            nr_received_bytes += len;
            printf("data[%2u] ", len);
            for (int i = 0; i < len; i++) {
                printf("%02x ", data[i]);
            }
            for (int i = 0; i < len; i++) {
                printf("%c", isprint(data[i]) ? data[i] : '.');
            }
            printf("\n");
            if (nr_received_bytes > 1024*10) {
                printf("%u bytes received. Disconnecting.\n", nr_received_bytes);
                close(fp);
                gap_disconnect(connection_handle);
            }
            break;
        case GATT_EVENT_QUERY_COMPLETE:
            //gatt_client_write_value_of_characteristic(gatt_data_event_handler, connection_handle, fifo_characteristic.value_handle, 13, (uint8_t*)"Hello World\n\r");
            break;
        default:
            printf("Unhandled event: 0x%02x\n", hci_event_packet_get_type(packet));
            break;
    }
}

static void gatt_characteristic_event_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size)
{
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    switch (hci_event_packet_get_type(packet)) {
        case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
            printf("Characteristic found.\n");
            gatt_event_characteristic_query_result_get_characteristic(packet, &fifo_characteristic);
            break;
        case GATT_EVENT_QUERY_COMPLETE:
            printf("Register notification handler and configure serialport FIFO characteristic for notify.\n");
            fp = open("data.bin", O_WRONLY|O_CREAT, S_IRUSR);
            gatt_client_listen_for_characteristic_value_updates(&notification_registration, gatt_data_event_handler, connection_handle, &fifo_characteristic);
            gatt_client_write_client_characteristic_configuration(gatt_data_event_handler, connection_handle, &fifo_characteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
            break;
        default:
            break;
    }
}

static void gatt_service_event_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size)
{
    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    switch (hci_event_packet_get_type(packet)) {
        case GATT_EVENT_SERVICE_QUERY_RESULT:
            printf("Service found.\n");
            gatt_event_service_query_result_get_service(packet, &serialport_service);
            break;
        case GATT_EVENT_QUERY_COMPLETE:
            printf("Search for serialport FIFO characteristic.\n");
            gatt_client_discover_characteristics_for_service_by_uuid128(gatt_characteristic_event_handler, connection_handle, &serialport_service, serialport_fifo_characteristic_uuid);
            break;
        default:
            break;
    }
}

static void shutdown_handler(btstack_timer_source_t* ts)
{
    UNUSED(ts);
    hci_power_control(HCI_POWER_OFF);
    hci_close();
}

static void hci_event_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    switch (hci_event_packet_get_type(packet)) {
        case BTSTACK_EVENT_STATE:
            switch (btstack_event_state_get_state(packet)) {
                case HCI_STATE_OFF:
                case HCI_STATE_INITIALIZING:
                    break;
                case HCI_STATE_WORKING:
                    // BLE stack activated. Connect to given device.
                    printf("Connecting to %s\n", bd_addr_to_str(cmdline_addr));
                    gap_connect(cmdline_addr, 0);
                    break;
                case HCI_STATE_HALTING:
                case HCI_STATE_SLEEPING:
                case HCI_STATE_FALLING_ASLEEP:
                    break;
            }
            break;

        case HCI_EVENT_LE_META:
            // Wait for connection complete
            if (hci_event_le_meta_get_subevent_code(packet) ==  HCI_SUBEVENT_LE_CONNECTION_COMPLETE) {
                printf("Connected\n");
                connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                printf("Search for serial port primary service.\n");
                gatt_client_discover_primary_services_by_uuid128(gatt_service_event_handler, connection_handle, serialport_service_uuid);
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            printf("Disconnected %s\n", bd_addr_to_str(cmdline_addr));
            {
                static btstack_timer_source_t timer;
                btstack_run_loop_set_timer(&timer, 0);
                btstack_run_loop_set_timer_handler(&timer, shutdown_handler);
                btstack_run_loop_add_timer(&timer);
            }
            break;

        default:
            break;
    }
}

int btstack_main(int argc, const char* argv[]);
int btstack_main(int argc, const char* argv[])
{
    // This app require stdin
    if (argc != 2) {
        printf("Usage: %s 00:12:F3:33:44:55 [... more addresses. max 4 in total]\n", argv[0]);
        exit(1);
    } else {
        if (sscanf_bd_addr(argv[1], cmdline_addr)) {
            printf("Connecting to: ");
            printf_hexdump(cmdline_addr, 6);
        }
    }

    // Register HCI event callback
    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Init L2CAP
    l2cap_init();

    // GATT Client setup
    gatt_client_init();

    // Security Manager needed? Probably not.
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    // Power on = start
    printf("Power on\n");
    hci_power_control(HCI_POWER_ON);

    return 0;
}
