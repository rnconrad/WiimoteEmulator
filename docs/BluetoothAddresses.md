# Bluetooth Addresses

This is a reference for techniques involving Bluetooth addresses. In most cases,
these are no longer needed, but are left here in case they may be useful.

## Determine Wiimote Address

There are lots of ways to find the address of a Wiimote. Below are the steps
for one method.
  - use hcitool to search for discoverable devices

        > hcitool scan

  - press the red sync button under the battery cover of the Wiimote
  - the Wiimote should be listed (Nintendo RVL-CNT-01) with its address

## Determine Wii Address

There are multiple ways to do this as well, including using a Bluetooth packet 
sniffer or using a utility via custom firmware on the Wii.

### Sync Emulator

Unless the emulator is not working, the emulator should automatically connect
to your Wii after pressing the controller sync button on the console. Once
connected, it will print the address of the Wii.

### Homebrew method

If you have installed the Homebrew Channel on your console or are willing to do
so, then you can use [identify-mii](https://github.com/ThatsJustCheesy/identify-mii)
to display your console's address on the screen.

### Battery method (hacky but usually works eventually)

  - change your Bluetooth address to a real Wiimote's using the bdaddr utility:

        > sudo bdaddr (wiimote address)

  - reset your adapter to apply the changes

        > sudo hciconfig hci0 reset

  - run the hcidump utility (any BT activity monitor can be substituted)

        > hcidump

  - press the red sync button on the Wii
  - press the red sync button on the Wiimote and after the status LEDs on the
    Wiimote blink 1-5 times, disconnect one of the batteries
  - if done correctly (with a bit of luck) the Wii will connect to your device,
    revealing its address
  - this may take few tries -- note that you must press both sync buttons again
    each time

