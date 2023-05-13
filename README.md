# Wiimote Emulator

Emulates a Bluetooth Wii controller in software.

![Raspberry Pi 3 running the emulator in Raspbian](rpi_ss.png)

### Features

- Emulate the Wiimote's many features and extensions
- Allows use of different input devices (keyboard etc.)

### Build/Install

The following dependencies/packages are required (if not already installed):

- libdbus-1-dev
- libglib2.0-dev
- libsdl1.2-dev

Run the build script (in the project directory):

> source ./build-custom.sh

For more information on the build script, see [this explainer](docs/CustomBuild.md).

### Using the Emulator

Stop any running Bluetooth service, e.g.:

> sudo service bluetooth stop

Start the custom Bluetooth stack (e.g. from the project directory):

> sudo ./bluez-4.101/dist/sbin/bluetoothd

Run the emulator (in the project directory):

> ./wmemulator

With no arguments, the emulator will listen for incoming connections (similar to
syncing a real Wiimote). Pressing the sync button on a Wii should cause it to
connect.

You can also supply the address of a Wii to directly connect to it as long as
you have connected to it before (or you change your device's address to the
address of a trusted Wiimote).

> ./wmemulator XX:XX:XX:XX:XX:XX

You will need to run the custom Bluetooth stack (as described above) whenever
using the emulator (it won't persist after e.g. a device restart). Also, the
custom stack generally won't be useful for anything besides Wiimote emulation.

To stop the custom stack and restore the original Bluetooth service, e.g.:

> sudo killall bluetoothd

> sudo service bluetooth start

For more information on bluetooth addresses, see [this explainer](docs/BluetoothAddresses.md).

### Connecting via UDP sockets

To connect via sockets it is expected that you know the Wii consoles address.

#### UNIX

> ./wmemulator XX:XX:XX:XX:XX:XX unix /tmp/some-path-here

#### IP

> ./wmemulator XX:XX:XX:XX:XX:XX ip {some-port-number-here}

#### Data Format

Sockets use the format `type status action`. For example, to press and hold the Wiimote + button you would send `button 1 WIIMOTE_PLUS`. To release the + button you would send `button 0 WIIMOTE_PLUS`.

**`type`** This can be `analog_motion`, `button`, `hotplug`, or `emulator_control`.

**`status`** This is the enabled / disabled state of the action. '0' = turn off, '1' = turn on. If you send a 1 the button will stay "pressed" until a 0 is sent.

**`action`** This is the equivalent to the physical control you want to invoke. For example `WIIMOTE_PLUS` or `IR_UP`

For more information on available types and actions, see [this explainer](docs/SocketActions.md).
