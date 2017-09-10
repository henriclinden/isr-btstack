#
# Build a BLE discovery application
#
# Henric Lindén, rt-labs AB
#

BTSTACK_ROOT = btstack

# Settings
VPATH += ${BTSTACK_ROOT}/src
VPATH += ${BTSTACK_ROOT}/src/ble
VPATH += ${BTSTACK_ROOT}/src/ble/gatt-service
VPATH += ${BTSTACK_ROOT}/platform/posix
VPATH += ${BTSTACK_ROOT}/platform/libusb

CFLAGS += -I.
CFLAGS += -I${BTSTACK_ROOT}/src
CFLAGS += -I${BTSTACK_ROOT}/src/ble
CFLAGS += -I${BTSTACK_ROOT}/platform/posix

CFLAGS += -g -Wall -Wmissing-prototypes -Wstrict-prototypes -Wshadow -Werror -Wunused-parameter -Wredundant-decls -Wsign-compare

# Needed files
CORE += \
	btstack_memory.c            \
	btstack_linked_list.c	    \
	btstack_memory_pool.c       \
	btstack_run_loop.c		    \
	btstack_util.c

COMMON += \
	ad_parser.c                 \
	hci.c			            \
	hci_cmd.c		            \
	hci_dump.c		            \
	l2cap.c			            \
	l2cap_signaling.c

GATT_CLIENT += \
	gatt_client.c

ATT += \
	att_dispatch.c

SM += \
	sm.c

# Using Posix platform but exchange UART for libusb as transport.
CORE += main.c btstack_stdin_posix.c
COMMON  += hci_transport_h2_libusb.c btstack_run_loop_posix.c le_device_db_fs.c btstack_link_key_db_fs.c wav_util.c

# Use pkg-config for libusb
CFLAGS  += $(shell pkg-config libusb-1.0 --cflags)
LDFLAGS += $(shell pkg-config libusb-1.0 --libs)

# Collect all necessary object files
SRCS = $(CORE) $(COMMON) $(GATT_CLIENT) $(ATT) $(SM)
OBJS = $(SRCS:.c=.o)

# Applications to build
all: discovery stream

# Build and link the simple discovery app
discovery: $(OBJS)

# Build and link the serialport endpoint app
stream: $(OBJS)

# Clean up
clean:
	-$(RM) -f $(OBJS)

