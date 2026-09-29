# Asahi platform

This platform is for devices supported by Asahi Linux.

Support for this platform is based on Asahi Linux.

This documentation was initially written based on the
`Apple Mac mini (M1, 2020)` device. More testing and feedback are
welcome.

## Before getting started with Android on the target device

You must first install and run a variant of Asahi Linux on the target
device. For details, see [the Asahi Linux website](https://asahilinux.org).

These instructions use the "Fedora Linux Asahi Remix" variant as an
example.

During installation, create an empty partition that will be used to
store Android OS later. It should be at least 8 GiB. We will refer to
it as the "Android partition" below.

After booting into the Linux OS, format the Android partition as
either ext4 or F2FS and mount it.

If you want USB support in U-Boot and GRUB, you must roll back U-Boot
with the following command:

```
sudo dnf install uboot-images-armv8-1:2025.10-101.fc42
```

## Build Android

Follow the instructions on the [Build](../build.md) page.

You will need to build these two targets: `all_images` and
`bootmgr-configs`.

Exceptions:

1. Copy the contents of `/usr/lib/firmware/vendor` on the Linux OS on
   the target device to `prebuilts/ramdisk/vendor/firmware` in this
   device tree, right after [this
   step](../build.md#obtaining-the-build-dependencies).
2. Before [this step](../build.md#select-the-target-device), clone the
   Asahi Linux kernel source from [here](https://github.com/AsahiLinux/linux)
   to `kernel/apple/asahi`, apply the patches listed in this
   [table](https://github.com/LineageOS/android_device_xiaomi_mi7150-mainline/#kernel-patches),
   make the edits described
   [here](https://github.com/LineageOS/android_device_xiaomi_mi7150-mainline/#kernel-edits),
   and finally specify the Asahi Linux kernel build configuration by
   running:
   `export MAINLINE_GENERIC_KERNEL_USE=asahi`.

## Install Android

Perform the following on the target device:

1. Enter the mount point of the Android partition.

2. Create a directory named `android-Generic_arm64` and enter it.

3. Copy `kernel`, `ramdisk-all-combined.img`, `system.img`,
   `vendor.img`, `system_dlkm.img`, and `vendor_dlkm.img` from the
   Android build output to this directory.

4. Create the following empty directories: `metadata`, `userdata`.

5. Copy `/usr/lib/firmware/vendor/` to `firmware`.

6. Copy `bootmgr-configs/single-partition/grub.cfg` from the Android
   build output to `/boot/grub2/custom.cfg`.

7. Edit `/boot/grub2/custom.cfg`:

   - Create an empty line right after the line
     `function boot_android {`, and write
     `    search --file --no-floppy --set=root /android-Generic_arm64/kernel`.
   - On the line beginning with `    linux`, move the cursor to right
     before `$@` at the end of the line, and write
     `androidboot.mount_firmware=only_android_dir androidboot.prefer_drm_render_name=asahi `.
   - Remove the entire block containing `Mount userdata from images`,
     as it does not match this setup.

## Boot Android

Restart the target device to the GRUB menu and select the option
containing `Mounting userdata from subdirs`.
