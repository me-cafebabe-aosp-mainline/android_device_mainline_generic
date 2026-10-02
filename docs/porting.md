# Porting

This page describes the general steps for porting Android OS to a new
target device using this device tree.

If the device needs its own device tree instead, see the bringup guide in
`device/mainline/common/docs/README.md`.

## Ensure the device can already boot Linux

If there is an existing Linux port that works for your target device,
use it as a reference.

## Gather the necessary information from the working Linux OS

You will need to know:

- The list of hardware components and what each requires to function.
- How the bootloader starts a Linux OS.
- Which firmware files the hardware needs.
- Which partitions can be used to store Android OS.

You will need to obtain:

- Firmware files required by the hardware, if any.
- Kernel source code.
- Source code for any out-of-tree kernel modules, if present.
- A toolchain for compiling the kernel, if needed.

## Create a custom kernel build configuration

If the target device will not use the default kernel build
configuration, create its own kernel build configuration as follows.

Create a `.mk` file in the Android source tree at the location
designated for kernel build configuration, and specify the path to it
with:

```
export MAINLINE_GENERIC_KERNEL_BOARDCONFIG_MK=<path to the kernel build configuration file relative to the Android source tree>
```

For the available kernel build configuration options, see
`vendor/lineage/build/tasks/kernel.mk` and
`vendor/lineage/config/BoardConfigKernel.mk`. Note that some options
are still set by this device tree by default even when a custom build
configuration is used.

The kernel configuration must include Android-specific options. These
can be enabled by including kernel config fragments from
`kernel/configs` and `kernel/mainline/configs`. The file
`Generic_arm64/kernels/asahi/board.mk` in this device tree is an
example of how this is done.

Some options in kernel config fragments may not be applied to the main
kernel config due to unsatisfied dependencies. To automatically detect
unapplied options, use the utility at
`kernel/mainline/configs/utilities/validate_kernel_config.py`.

## Kernel modules configuration

### Kernel modules installation

Some kernel modules must be installed into the ramdisk. For details,
see [Loading basic kernel modules in
ramdisk](booting-process.md#loading-basic-kernel-modules-in-ramdisk).

By default, the set of kernel modules installed in the ramdisk is
determined using an approach similar to Debian's `initramfs-tools`,
which is mostly based on the module's source directory. You can view
and modify the implementation in
`configs/kernel/boot_kernel_modules_finder.sh` in this device tree.

All kernel modules are installed in the `vendor_dlkm` partition.

### Kernel modules loading list

By default, kernel modules are loaded on demand in response to kernel
requests. You can also specify particular modules to load by editing:

- `configs/modprobe/modules.load.ramdisk` for ramdisk modules.
- `configs/modprobe/modules.load` and
  `configs/modprobe/modules.load_prefix` for post-ramdisk modules.

### Kernel modules load options

If the target device needs to load kernel modules with specific
options, specify them in a file named `modules.options`.

See `configs/modprobe/modules.options` in this device tree for the
file's syntax.

If modules loaded in the ramdisk need options, install the file to
`/lib/modules/modules.options` in the ramdisk.

If modules loaded after the ramdisk need options, install the file to
`/lib/modules/modules.options` in the `vendor_dlkm` partition.

## Include firmware files in the ramdisk

Kernel modules loaded in the ramdisk may request firmware files. Such
firmware files should be included in the ramdisk.

To include firmware files, copy them to
`prebuilts/ramdisk/vendor/firmware` (preserving the directory
structure) in this device tree.

## Build

See [Build](build.md).

## Install

See [Installation](installation.md).

## Troubleshooting boot issues

For debugging techniques, see [Debugging](debugging.md). Reading the
[Booting process](booting-process.md) page may also help you
understand the issue.

The following table lists common issues encountered during initial
bring-up:

| Issue | Suggestion |
|-------|------------|
| Instant crash right after the bootloader stage | Check the bootloader configuration and kernel build configuration. |
| No display after the kernel starts | Confirm whether the system is still responding via other means. If it is, the display driver may be missing or failing to load. |
| Kernel log shows repeating `generic_init: CanQuitUeventd: ` lines | Check for issues related to the installation. |
| Android does not play the boot animation | Try using framebuffer display as a temporary workaround. |
