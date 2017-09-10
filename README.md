# isr-btstack
Bluetooth stack experiments on macOS, Linux, and embedded systems. The Bluetooth stack used is the btstack from BlueKitchen. (https://github.com/bluekitchen/btstack)

# Hardware
Test apps in this archive are tested using the ASUS BT-400 USB dongle and the Bluetooth part of the Texas Instruments WL1273 multi radio module.

# Test systems

## macOS
On Mac the built in Bluetooth stack will automatically assign all new adapter to itself. Hence, any new adapters will not be available for tests. To stop this the following nvram configuration can be used.

    sudo nvram bluetoothHostControllerSwitchBehavior=never

This tells the macOS Bluetooth stack not to use any other Bluetooth hardware except for the builtin.

The test apps depends on libusb. This can be installed using Homebrew.

    brew install libusb

## Linux
TBD

## Embedded
TBD

# Building
To build the test apps use make.

    make
