/*
 * main for Bluetooth LE apps.
 * For use with UART modules.
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

#include "bluetooth_company_id.h"

#include "hci.h"
#include "hci_dump.h"
#include "btstack_stdin.h"

#include "btstack_chipset_cc256x.h"


int btstack_main(int argc, const char* argv[]);

static btstack_packet_callback_registration_t hci_event_callback_registration;

static hci_transport_config_uart_t config = {
    HCI_TRANSPORT_CONFIG_UART,
    115200,         // Default baudrate after a reset of WL1273
    0,              // Main baudrate
    1,              // Flow control
    "/dev/ttyO5"    // Bluetooth on ELIN (ODIN-W160)
};

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
            case HCI_EVENT_COMMAND_COMPLETE:
                if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_read_local_name)) {
                    if (hci_event_command_complete_get_return_parameters(packet)[0]) {
                        break;
                    }
                    packet[6 + 248] = '\0';
                    printf("Local name: %.247s<-->\n", &packet[6]);
                }
                if (HCI_EVENT_IS_COMMAND_COMPLETE(packet, hci_read_local_version_information)) {
                    printf("Local version information:\n");
                    uint16_t hci_version    = packet[6];
                    uint16_t hci_revision   = little_endian_read_16(packet, 7);
                    uint16_t lmp_version    = packet[9];
                    uint16_t manufacturer   = little_endian_read_16(packet, 10);
                    uint16_t lmp_subversion = little_endian_read_16(packet, 12);
                    printf("- HCI Version    0x%04x\n", hci_version);
                    printf("- HCI Revision   0x%04x\n", hci_revision);
                    printf("- LMP Version    0x%04x\n", lmp_version);
                    printf("- LMP Subversion 0x%04x\n", lmp_subversion);
                    printf("- Manufacturer   0x%04x\n", manufacturer);
                    if (manufacturer == BLUETOOTH_COMPANY_ID_TEXAS_INSTRUMENTS_INC) {
                        printf("Texas Instruments - CC256x compatible chipset.\n");
                        hci_set_chipset(btstack_chipset_cc256x_instance());
                    }
                    else {
                        printf("This app is made for the ELIN+ODIN-W160 combo.");
                    }
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

int main(int argc, const char* argv[])
{
    // Do necessary stack initialisations.
    btstack_memory_init();
    btstack_run_loop_init(btstack_run_loop_posix_get_instance());

    // Redirect log to packetlogger for use in Wireshark.
    hci_dump_open("hci_dump.pklg", HCI_DUMP_PACKETLOGGER);
    log_info("Program name: %s", argv[0]);

    // Init HCI.
    const btstack_uart_block_t* uart_driver = btstack_uart_block_posix_instance();
    const hci_transport_t* transport = hci_transport_h4_instance(uart_driver);
    //const btstack_link_key_db_t * link_key_db = btstack_link_key_db_fs_instance();
    hci_init(transport, (void*) &config);
    //hci_set_link_key_db(link_key_db);

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
