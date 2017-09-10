/*
 * Generic main for Bluetooth LE apps.
 * Currently tailored for use with USB HCI dongles.
 *
 * Henric Lindén, rt-labs AB
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "btstack_config.h"

#include "btstack_debug.h"
#include "btstack_event.h"
#include "btstack_memory.h"
#include "btstack_run_loop.h"
#include "btstack_run_loop_posix.h"

#include "hci.h"
#include "hci_dump.h"
#include "btstack_stdin.h"

int btstack_main(int argc, const char* argv[]);

static btstack_packet_callback_registration_t hci_event_callback_registration;

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != BTSTACK_EVENT_STATE) {
        return;
    }
    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
        return;
    }

    printf("BTstack up and running.\n");
}

static void sigint_handler(int param)
{
    UNUSED(param);

    printf("CTRL-C - SIGINT received, shutting down.\n");
    log_info("sigint_handler: shutting down");

    // Reset anyway
    btstack_stdin_reset();

    // Power down
    hci_power_control(HCI_POWER_OFF);
    hci_close();

    exit(0);
}

#define USB_MAX_PATH_LEN 7
int main(int argc, const char* argv[])
{
    uint8_t usb_path[USB_MAX_PATH_LEN];
    int usb_path_len = 0;

    // Parse command line options for USB port string ("-u 11:22:33")
    if (argc >= 3 && strcmp(argv[1], "-u") == 0) {
        const char* port_str = argv[2];
        printf("Specified USB Path: ");
        while (1) {
            char* delimiter;
            int port = strtol(port_str, &delimiter, 16);
            usb_path[usb_path_len] = port;
            usb_path_len++;
            printf("%02x ", port);
            if (!delimiter) {
                break;
            }
            if (*delimiter != ':' && *delimiter != '-') {
                break;
            }
            port_str = delimiter + 1;
        }
        printf("\n");
        argc -= 2;
        memmove(&argv[0], &argv[2], argc * sizeof(char*));
    }

    // Do necessary stack initialisations.
    btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_posix_get_instance());

    if (usb_path_len) {
        hci_transport_usb_set_path(usb_path_len, usb_path);
    }

    // Use logger: format HCI_DUMP_PACKETLOGGER, HCI_DUMP_BLUEZ or HCI_DUMP_STDOUT
    char pklg_path[100];
    strcpy(pklg_path, "/tmp/hci_dump");
    if (usb_path_len) {
        strcat(pklg_path, "_");
        strcat(pklg_path, argv[2]);
    }
    strcat(pklg_path, ".pklg");
    printf("Packet Log: %s\n", pklg_path);
    hci_dump_open(pklg_path, HCI_DUMP_PACKETLOGGER);

    // Init HCI
    hci_init(hci_transport_usb_instance(), NULL);

    // Inform about BTstack state
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Handle CTRL-C
    signal(SIGINT, sigint_handler);

    // Setup app
    btstack_main(argc, argv);

    // Do runloop
    btstack_run_loop_execute();

    // Shouldn't end up here...
    return 0;
}
