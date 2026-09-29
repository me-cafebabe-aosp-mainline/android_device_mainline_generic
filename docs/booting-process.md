# Booting process

This document describes the boot sequence from the kernel through the
mounted Android OS, with emphasis on the role of `generic_init`.

## Kernel

This is device-dependent.

Use a kernel built from a kernel tree that supports the target device,
with Android configurations (and possibly some Android-specific
patches) included.

Boot the kernel the same way any other working Linux OS does, or use
another method supported by the device.

The kernel command line comes from the output of
`get_build_var BOARD_KERNEL_CMDLINE` in the Android source tree after
selecting the target device, optionally with additional custom
parameters (see [Boot parameters](boot-parameters.md)).

Use `ramdisk-all-combined.img` from the Android target device output
directory as the ramdisk.

Ways to boot the kernel include but are not limited to:

- Boot loaders (U-Boot, device-specific boot loader, etc.)
- Boot managers (GRUB, rEFInd, syslinux, systemd-boot, etc.)
- Direct kernel execution in an EFI environment (if the kernel has
  EFI stub support enabled)

## Ramdisk

The ramdisk may be a concatenation of multiple ramdisk images in the
same format.

The ramdisk contains the init program and its configuration, and
possibly kernel modules and firmware files.

The kernel executes the init program specified by the `rdinit=`
parameter. By default this is `/init`; here it is set to
`/system/bin/generic_init`.

Standard Android uses `/init`, built from `system/core/init`. This
device tree uses a fork named `generic_init`, which is more flexible
and includes additional features. Its source code is at
`vendor/mainline/services/generic_init`.

`generic_init` is essentially a merge of AOSP `init_first_stage` and
`ueventd`, with additional wiring between them and a mount handler
(replacing `first_stage_mount.cpp`) that automatically scans
partitions and generates fstab entries on the fly.

### Basic environment setup in ramdisk

This is done by the init program, and the implementation in
`generic_init` does not differ significantly from AOSP `/init`.

### Loading basic kernel modules in ramdisk

Kernel modules required to probe the boot media, basic hardware
platform support, and output devices (serial device drivers or basic
display drivers) should be installed to `/lib/modules` in the ramdisk.

`generic_init` first loads the modules listed in `modules.load` and
the modules whose names match a prefix listed in `modules.load_prefix`,
both located in `/lib/modules`.

It then starts the `ueventd` component to listen for kernel uevents
and load modules based on the `MODALIAS` field in each uevent.

If a loaded kernel module requests firmware, the `ueventd` component
provides it from directories specified in the ueventd configuration at
`/system/etc/ueventd.ramdisk.rc` in the ramdisk.

The `ueventd` component also informs the mount handler about newly
added block devices. The mount handler scans these block devices and
caches their information.

Once the mount handler has collected enough block devices to mount
every required Android partition (depending on the chosen boot
parameters), the `ueventd` component exits and the next step begins.

### Mounting Android partitions

The mount handler is invoked again to perform preparations (mounting
base partitions and setting up loop devices), then constructs a virtual
fstab (including entries from the specified addon fstab), and passes
the resulting fstab to `MountHelpers::MountPartitions()`. This
function is a copy of `FirstStageMountVBootV2::MountPartitions()` from
AOSP `first_stage_mount.cpp` and performs the remaining mounts.

After mounting completes, the root directory is switched over to the
mounted Android OS.

### Executing custom vendor init program

`generic_init` executes `/vendor/bin/vendor_init`. Preparations such
as generating firmware files can be done in that program.

### Loading kernel modules from the mounted partitions

`generic_init` again loads modules listed in `modules.load` and
modules matching a prefix in `modules.load_prefix`, this time from
`/vendor/lib/modules` instead of the ramdisk's `/lib/modules`.

The `ueventd` component starts again, but now uses configuration from
`/system/etc/ueventd.rc` in the mounted partitions instead of the
ramdisk.

`/system/etc/ueventd.rc` imports `/vendor/etc/ueventd.rc`, which in
this device tree is copied from
`device/mainline/common/init/ueventd.rc` and imports
hardware-specific configuration files based on properties. Because
properties are not yet loaded at this stage, `androidboot.*`
parameters are used as a substitute. See `TranslatePropName()` in
`ExpandProps()` in `util.cpp` within `generic_init` for details.

The `ueventd` component loads modules based on received uevents once
again.

Additionally, because the partitions are now mounted, a usermode
helper at `/vendor/bin/modprobe_kernel` can handle kernel requests for
modules by exact name. Some non-basic drivers use this method.

Firmware requests are handled the same way as before, but searching
the directories specified by the newly loaded ueventd configuration.

Loading as many modules as possible early is important, because a
service that runs very early in the mounted Android OS performs HAL
selection based on probed hardware.

If no new uevents arrive for a consecutive 5 seconds, the `ueventd`
component exits and the next step begins.

### Start the mounted Android OS

`generic_init` executes `/system/bin/init`, the same way AOSP `/init`
does.

## The mounted Android OS

The following sections document only the parts that differ from
standard Android.

### The hardware_detect service

`hardware_detect` is a service that performs hardware-related
configuration and selects the HALs to use. Its service name is
`init_dev_config` and its binary is at `/vendor/bin/hardware_detect`.
Its source code is at `services/hardware_detect` in this repository.

If this service does not execute successfully, Android will crash due
to unselected APEXes.

### The get_display_ppi service

This service calculates the PPI (pixels-per-inch) value from the EDID
of the first connected connector on the selected DRM card, and sets
the result to the LCD density property. Its source code is at
`services/get_display_ppi` in this repository.
