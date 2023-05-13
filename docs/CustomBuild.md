# Custom Build

This is a reference for what the `build-custom.sh` script does.

The custom build downloads and builds a patched version of bluez-4.101 with
the following changes:

  - The built-in SDP server is disabled, as a Wii-specific SDP handler is used
    in the emulator instead.
  - A PIN code handler plugin is installed to respond to PIN code requests from
    the Wii. This plugin is typically only needed to connect to a Wii without
    doing the sync/discovery process (e.g. spoofing a synced Wiimote's address).
    The plugin only works on older versions of bluez, so this is why 4.101 is
    used.

Run the build script (in the project directory):

  > source ./build-custom.sh

Stop any running Bluetooth service, e.g.:

  > sudo service bluetooth stop

Start the custom bluetooth stack (e.g. from the project directory):

  > sudo ./bluez-4.101/dist/sbin/bluetoothd

Then use the emulator normally.