Apple Media Service demo application {#ams}
===========================================================================

## Overview

This application is a sample implementation of the Apple Media Service (AMS) service client - Media Remote role
defined in the AMS Profile Specification. The demo supports all optional characteristics of AMS - Remote Command,
Entity Update and Entity Attribute.

## Installation procedure

The project is located in the \b `projects/dk_apps/ble_profiles/ams` folder.

To install the project follow the [General Installation and Debugging Procedure](@ref install_and_debug_procedure).

### UART configuration

The application can be controlled using the serial console exposed over UART2.

GPIO pins configuration is as follows:

| DA1469x GPIO pin | DA1468x GPIO pin | Function
|------------------|------------------|----------
| P0.9             | P1.3             | UART2 TX
| P0.8             | P2.3             | UART2 RX
| P0.7             | P1.6             | UART2 CTS

UART settings are as follows:

Setting      | Value
-------------|--------
Baudrate     | 115200
Data bits    | 8
Stop bits    | 1
Parity       | None
Flow control | RTS/CTS

### LED configuration

The application sets the LED active when playing. Otherwise the LED is turned off. The LED is connected to P1.5
on the DA1468x platform (jumper J5 must be connected) and to P1.1 on the DA1469x platform (P1.1 is connected to
the RED led on the daughterboard).

### BUTTON configuration

The application uses a button to trigger PLAY/PAUSE command. The button is also used to unpair all devices
at startup - user may simply keep the button pressed during reboot. If there is any bonded device, the following
message will be printed on the console (in this example C4:61:8B:80:2B:5E device is in cache):
~~~
Unpairing C4:61:8B:80:2B:5E
Start advertising...
~~~

The button is connected to P1.2 on the DA1468x platform (a connection between J8 connector's first pin and P1.2
pin is needed) and P0.6 on the DA1469x platform (J8 jumper must be connected).

## Suggested Configurable parameters

### Verbose logs

To enable verbose logs containing all data coming from the AMS, set CFG_VERBOSE_LOG in ams_config.h file
to a non-zero value, i.e.

~~~{.c}
#       define CFG_VERBOSE_LOG (1)
~~~

## Manual testing

The application is controlled using command line interface (CLI) which can be accessed using the serial
port available when the platform is connected to a PC using USB connector (e.g. `/dev/ttyUSBx` or `COMx`).

### Quick start

Use a device that supports AMS service, scan for the "Dialog AMS Demo" device and connect to it.
~~~
Device connected
Browsing...
Browse completed
GATT: found
AMS: found
Security level changed (level: 2)
Ready.
~~~

Notifications from Entity Update are displayed in human-readable form as follows:
~~~
Remote commands update
Artist: Some Artist
Duration: 384.113
Title: Some Title
~~~

6. To use full functionality of the application, check the complete list of commands below.

### Available commands

#### `cmd <remote_command>`

Send remote command

It will send a remote command to the AMS Media Source. Remote commands are defined in the AMS Profile
Specification, i.e.:
~~~
cmd 2
Remote command
	Remote command 0x02 sent
Playback state: Paused

~~~
Will send a RemoteCommandIDTogglePlayPause command.

#### `fetch <entity_id> <attribute_id>`

This command allows the user to read the content of a given attribute. I.e. if the negotiated MTU was
not large enough to send Track Name in the Entity Update notification, it can be fetched using the command below:
~~~
fetch 2 2
Fetch attribute
	Pushed to attribute requests queue
Title: Example Title
~~~

\note
Not all attributes may be exposed by Entity Attribute. It depends on the capabilities of the active player.

### Verbose Logs Output

Enabling verbose logs allow the user to see more detailed information about notifications coming from iOS.
Entity Update notifications are decoded as follows:

~~~
Entity Update
	Entity ID: EntityIDTrack (0x02)
	Attribute ID: TrackAttributeIDDuration (0x03)
	Flags: (0x00)
	Value: 291.527
~~~

Read from Entity Attribute:

~~~
Entity Attribute Read Completed
	Entity ID: EntityIDPlayer (0x00)
	Attribute ID: PlayerAttributeIDName (0x00)
	Value: SomePlayerName
~~~

Remote Command notifications:

~~~
Remote commands update
	Number of supported commands: 0x0E
	Commands:
		RemoteCommandIDPlay (0x00)
		RemoteCommandIDPause (0x01)
		RemoteCommandIDTogglePlayPause (0x02)
		RemoteCommandIDNextTrack (0x03)
		RemoteCommandIDPreviousTrack (0x04)
		RemoteCommandIDVolumeUp (0x05)
		RemoteCommandIDVolumeDown (0x06)
		RemoteCommandIDAdvanceRepeatMode (0x07)
		RemoteCommandIDAdvanceShuffleMode (0x08)
		RemoteCommandIDSkipForward (0x09)
		RemoteCommandIDSkipBackward (0x0A)
		RemoteCommandIDLikeTrack (0x0B)
		RemoteCommandIDDislikeTrack (0x0C)
		RemoteCommandIDBookmarkTrack (0x0D)
~~~
