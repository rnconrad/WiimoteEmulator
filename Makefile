ifeq ($(origin CUSTOM_BUILD),undefined)
LBLUETOOTH=-lbluetooth
CFLAGS=
else
BLUEZ_DIST=$(shell pwd)/bluez-4.101/dist
LBLUETOOTH=-L"$(BLUEZ_DIST)/lib" -I"$(BLUEZ_DIST)/include" -Wl,-rpath="$(BLUEZ_DIST)/lib" -lbluetooth
CFLAGS=-D SDP_SERVER
endif
LDBUS=`pkg-config --cflags dbus-1` -ldbus-1

all: wmemulator packedtest wmmitm
clean:
	rm -f wmemulator packedtest wmmitm
wmemulator: wmemulator.c wiimote.c input.c motion.c input_sdl.c input_socket.c wm_crypto.c wm_reports.c wm_print.c sdp.c bdaddr.c adapter.c
	gcc $(CFLAGS) -o wmemulator wmemulator.c wiimote.c input.c motion.c input_sdl.c input_socket.c wm_crypto.c wm_reports.c wm_print.c sdp.c bdaddr.c adapter.c $(LBLUETOOTH) -lSDL -lpthread -lm $(LDBUS) -Wall
wmmitm: wmmitm.c wm_print.c sdp.c bdaddr.c adapter.c
	gcc $(CFLAGS) -o wmmitm wmmitm.c wm_print.c sdp.c bdaddr.c adapter.c $(LBLUETOOTH) -lpthread -lm $(LDBUS) -Wall
packedtest: packedtest.c
	gcc -o packedtest packedtest.c
