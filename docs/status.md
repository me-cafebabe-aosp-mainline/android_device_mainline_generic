# Status

This page describes the current support status of each hardware
subsystem on this device tree.

## Audio

Works if the kernel sound driver functions and `alsa-ucm-conf`
contains the appropriate configuration.

## Backlight

The first backlight device that appears during boot is selected in
most cases.

## Battery

Works if the kernel has a driver and exposes a `/sys/class/power_supply`
interface.

If no battery devices appear in `/sys/class/power_supply`, a fake
battery reporting 85% state-of-charge is presented to Android.

## Bluetooth

Works for Bluetooth cards that support recent Bluetooth versions.

Older Bluetooth cards may have issues.

## Booting

Should boot as long as the device can boot any other Linux OS and has
matching or sufficient specs for Android.

## Camera

Not yet implemented.

## Codecs

Software codecs are available from AOSP Codec2.

Hardware codecs are not yet available.

## Display

Display may work via the DRM subsystem and should always work via the
framebuffer subsystem.

## Ethernet

Works.

There is currently no option to configure a static IP address for
Ethernet.

## GNSS

Not yet implemented.

## Graphics

3D-accelerated graphics is not guaranteed to work on every graphics
card.

CPU-rendered graphics is always available as a fallback, but may need
to be selected manually (framebuffer display always selects it).

Hybrid GPU setups (one GPU for rendering and another for display) are
not currently supported; only the display GPU is used.

## Input

Most common input device types work as long as they are recognized by
the Linux kernel.

For tablet-type input devices, Android does not support them natively.
A service is provided that attempts to translate their input events to
touchscreen events, but it currently only functions for the few tablet
input devices listed in it.

## Power management

Not yet implemented.

## Radio

Not yet implemented.

## Sensors

Sensor devices exposed via the Linux IIO subsystem or the Linux Input
subsystem may be supported.

## USB

Host mode should work like any standard Android device.

Device mode requires the controller to support it and the controller
driver to expose USB ConfigFS.

## Vibration

Works for vibrator devices that expose a `/dev/input` interface.

## Wi-Fi

Works with most Wi-Fi cards, though some issues may occur.

VirtWifi is available if the device has no Wi-Fi card but does have an
Ethernet card.
