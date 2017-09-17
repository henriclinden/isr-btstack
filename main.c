/*
 * main for Bluetooth LE apps.
 * For use with USB HCI dongles.
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

// A HCI event handler informing about the current state. Not needed
// for normal operation.
static void hci_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size)
{
    UNUSED(channel);
    UNUSED(size);

    if (packet_type == HCI_EVENT_PACKET) {
        switch (hci_event_packet_get_type(packet)) {
            case BTSTACK_EVENT_STATE:
                switch (btstack_event_state_get_state(packet)) {
                    case HCI_STATE_OFF:
                        fprintf(stderr, "[HCI_STATE_OFF]\n");
                        exit(0);
                        break;
                    case HCI_STATE_INITIALIZING:
                        fprintf(stderr, "[HCI_STATE_INITIALIZING]\n");
                        break;
                    case HCI_STATE_WORKING:
                        fprintf(stderr, "[HCI_STATE_WORKING]\n");
                        break;
                    case HCI_STATE_HALTING:
                        fprintf(stderr, "[HCI_STATE_HALTING]\n");
                        break;
                    case HCI_STATE_SLEEPING:
                        fprintf(stderr, "[HCI_STATE_SLEEPING]\n");
                        break;
                    case HCI_STATE_FALLING_ASLEEP:
                        fprintf(stderr, "[HCI_STATE_FALLING_ASLEEP]\n");
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

static void sigint_handler(int param)
{
    UNUSED(param);

    fprintf(stderr, "CTRL-C - SIGINT received, shutting down.\n");
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
    // Do necessary stack initialisations.
    btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_posix_get_instance());

    // Redirect log to packetlogger for use in Wireshark.
    hci_dump_open("hci_dump.pklg", HCI_DUMP_PACKETLOGGER);
    log_info("Program name: %s", argv[0]);

    // Init HCI. This will use the first available Bluetooth dongle.
    hci_init(hci_transport_usb_instance(), NULL);

    // Add a hci event handler just for showing btstatck state to user.
    hci_event_callback_registration.callback = &hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Handle CTRL-C
    signal(SIGINT, sigint_handler);

    // Setup app
    btstack_main(argc, argv);

    // Execute runloop
    btstack_run_loop_execute();

    // Shouldn't end up here...
    return 0;
}
