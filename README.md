# isr-btstack
Bluetooth stack experiments on macOS, Linux, and embedded systems. The Bluetooth stack used is the btstack from BlueKitchen. (https://github.com/bluekitchen/btstack)

# Hardware
Test apps in this archive are tested using the ASUS BT-400 USB dongle and the Bluetooth part of the Texas Instruments WL1273 multi radio module.

# BLE stack
The Bluetooth stack is from BlueKitchen GmbH and is mounted in the project tree as a git submodule. Before building the test apps, the Bluetooth stack needs to be cloned. 

    cd btstack
    git submodule init
    git submodule update

# Test systems

## macOS
On Mac the built in Bluetooth stack will automatically assign all new adapter to itself. Hence, any new adapters will not be available for tests. To stop this the following nvram configuration can be used.

    sudo nvram bluetoothHostControllerSwitchBehavior=never

This tells the macOS Bluetooth stack not to use any other Bluetooth hardware except for the builtin.

The test apps depends on libusb. This can be installed using Homebrew.

    brew install libusb
    
The ASUS-BT400 is a H2 USB dongle. Use the USB Makefile for building.

    make -f Makefile_usb

## Linux
For Linux the following packages needs to be installed:

    sudo apt install git build-essential libusb-1.0-0-dev
    
The ASUS-BT400 is a H2 USB dongle. Use the USB Makefile for building.

    make -f Makefile_usb

## Embedded
For embedded (Yocto Linux) unload current Bluetooth stacks (bluez) and drop the HCI driver.

The Bluetooth part of WL1273 is conencted to ttyO5 on the ELIN and thus need a serial driver (H4) instead of USB (H2). Use Makefile_uart to build for ELIN:

    make -f Makefile_uart
    
Note; you need to be root to run the apps.
