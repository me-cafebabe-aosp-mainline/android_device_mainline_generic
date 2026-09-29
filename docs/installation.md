# Installation

## Before getting started

Before starting the actual installation, understand the components
involved and decide which installation methods to use.

The components are: kernel, ramdisk, system, userdata, and firmware.

Of these, firmware can be borrowed from an existing Linux OS
installation, created during installation, or omitted. Userdata is
created during installation or omitted. The remaining components come
from the Android build output.

Installation of the kernel and ramdisk depends on how the bootloader
loads them, which varies by device and is outside our control.

Installation of the other components is defined by us and described
below.

### Details of each component

The system component consists of the following Android partitions:
`odm`, `odm_dlkm`, `product`, `system`, `system_ext`, `system_dlkm`,
`vendor`, `vendor_dlkm`.

In the current configuration, `product` and `system_ext` are included
in `system`, `odm` and `odm_dlkm` are included in `vendor`, and
`system_dlkm` and `vendor_dlkm` are standalone.

The userdata component consists of the following Android partitions:
`cache` (ignored for now), `userdata`, and `metadata`.

The firmware component is the set of all firmware files required by
device drivers, either flattened into a subdirectory or packed into an
image.

### Available installation methods

The "Android directory" is a directory containing the Android-specific
components. Its name can be customized via a boot parameter (see
[Boot parameters](boot-parameters.md) → `androidboot.android_dir`). If
multiple block devices contain an Android directory, the last one
found is used.

System, userdata, and firmware can be installed as images inside the
Android directory. For system and userdata (but not firmware), there
is also an option to copy the images to RAM and mount them from there
instead of directly from the storage media. The corresponding boot
parameter values are `img` and `img_ram` (firmware), `imgs` and
`imgs_ram` (system and userdata).

Userdata and firmware can be installed as subdirectories inside the
Android directory. For userdata, this is unavailable if the filesystem
holding the Android directory does not support Linux file attributes
and permissions. The corresponding boot parameter value for userdata
is `bind_mount_dir`, and for firmware is `only_android_dir`.

System and userdata can be installed onto partitions on disks with a
GPT partition table. The partition name on the GPT table identifies
which Android partition the disk partition corresponds to (for
example, the partition hosting the `system` Android partition should
have the GPT partition name `system`). The corresponding boot
parameter value is `std_parts`.

System can be installed onto any block devices, which may be
partitions or entire disks. The corresponding boot parameter value is
`blk_devices`.

Firmware can be pre-existing in certain directories on the filesystem
of any block device. The corresponding boot parameter value is
`all_possible_dirs`.

Firmware can be omitted entirely, in which case firmware files are not
available. The corresponding boot parameter value is `disable`.

Userdata can be omitted, in which case user data is stored in RAM and
does not persist across reboots. The corresponding boot parameter
value is `tmpfs`.

## Kernel and ramdisk

These should be placed at a location the bootloader can access.

The kernel should be booted with a kernel command line and a ramdisk,
both described below.

The kernel command line comes from two sources:

1. The common part, obtained from the Android source tree by running
   `get_build_var BOARD_KERNEL_CMDLINE` after selecting the target
   device.
2. The custom part, which you construct yourself according to [Boot
   parameters](boot-parameters.md).

For the ramdisk, use `ramdisk-all-combined.img`.

Additionally, some SoC platforms may require booting the kernel with a
device tree blob.

The exact steps to configure the bootloader depend on the bootloader
itself; consult the bootloader's documentation for details.

This device tree provides example boot manager configuration files and
a build target that generates a combined kernel + command line +
ramdisk image for EFI boot environments. See [Build](build.md#build-the-wanted-target)
for details.

## System

| Method | Steps |
|--------|-------|
| Block devices | Write the Android system images to the block devices you wish to install to. |
| Images | Copy the Android system images to the Android directory. |
| Standard partitions | Write the Android system images to the GPT partitions you wish to install to, and set each partition's GPT name to the corresponding Android system partition name. |

## Userdata

Android typically requires at least 1 GiB of space for userdata.

| Method | Steps |
|--------|-------|
| Ignore (tmpfs) | Ensure the target system has enough total RAM to hold userdata in memory. |
| Images | Create empty images named after each Android userdata partition inside the Android directory. These images should either contain only zeroes or be pre-formatted with an ext4 or F2FS filesystem. |
| Standard partitions | Set the GPT partition name of each userdata partition to the corresponding Android userdata partition name, then either ensure the partitions contain only zeroes or format them with ext4 or F2FS. |
| Subdirectories | Create empty subdirectories named after each Android userdata partition inside the Android directory. The filesystem holding the Android directory must be ext4 or F2FS. |

## Firmware

| Method | Steps |
|--------|-------|
| Directories | Ensure at least one block device contains a firmware directory. The directories checked on each block device (in order) are: `<android_dir>/firmware`, `usr/lib/firmware`, `lib/firmware`, `linux-firmware`, and `firmware`. Some of these are normally present on a block device that contains a GNU/Linux installation. |
| Ignore | Ensure that no peripherals require firmware, or that they load their firmware through other means. |
| Image | Build an image containing the firmware files and place it in the Android directory as `firmware.img` or `linux-firmware.img`. Tips: (1) the image filesystem can be EROFS; (2) if obtaining firmware from the linux-firmware repository, use its `copy-firmware.sh` script. |
| Only Android directory | Ensure the `firmware` subdirectory exists inside the Android directory and contains the needed firmware files. |
