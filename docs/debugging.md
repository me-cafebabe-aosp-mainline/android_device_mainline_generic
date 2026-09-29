# Debugging

This document describes common debugging techniques for this device
tree.

## Gathering shell access on a specified serial console

Specify the serial device via the `androidboot.console=` parameter,
using the same syntax as the Linux `console=` parameter.

## Gaining ADB access via Ethernet or VirtIO VSOCK during a system service crash loop

Boot with the parameter `androidboot.insecure_adb=true`.

If connecting via Ethernet, gain shell access first and run the
following commands (assuming the Ethernet interface is `eth0` and the
IPv4 address to assign is `192.168.1.100/24`):

```
stop
ip link set eth0 up
ip address add 192.168.1.100/24 dev eth0
ip rule add from all lookup main
```

Then connect:

- Via Ethernet: `adb connect <IPv4 address>` (e.g.
  `adb connect 192.168.1.100`)
- Via VirtIO VSOCK: `adb connect vsock:<Guest CID>:5555` (the Guest
  CID is usually visible in the VM configuration)

## Pausing at a shell during early boot

See the `androidboot.mode=console` parameter in [Boot
parameters](boot-parameters.md).

## Printing the kernel log to a serial console

Use the standard Linux `console=` parameter.

## Printing logcat to a serial console

See the `androidboot.seriallogging` parameters in [Boot
parameters](boot-parameters.md).

## Saving early logs to the metadata partition

See the `androidboot.save_early_logs` parameter in [Boot
parameters](boot-parameters.md).
