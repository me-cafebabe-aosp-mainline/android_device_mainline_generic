# Build

Before getting started with this, you should have synced the LineageOS source tree from a branch that this repository have.

## Obtaining the build dependencies

Please visit [Build dependencies](build-dependencies.md) page for the build dependencies to be obtained.

## Setting up the build environment

Enter LineageOS source tree, and then run:

```
source build/envsetup.sh
```

## Decide the target device to build

This device tree currently have the following target devices:
- `Generic_arm64`
- `Generic_x86_64`

The chosen target device will be referred as `<device>` in below.

## Applying patches

Please visit [Patches](patches.md) page for the patches be applied.

## Select the target device

Run this command:

```
breakfast <device>
```

## Build the wanted target

Here is a table of the supported build targets:

| Target | Output path | Description |
|--------|-------------|-------------|
| `all_images` | `$ANDROID_PRODUCT_OUT/*.img` | Builds all images that this device tree provides. |
| `bootmgr-configs` | `$ANDROID_PRODUCT_OUT/bootmgr-configs` | Builds every boot manager configurations which this device tree currently have template of. |
| `liveisoimage` | `$ANDROID_PRODUCT_OUT/*-live.iso` | Live ISO image which can be put onto CD-ROM type of media and boots full Android without installation. Currently only supported for `Generic_x86_64`. |
| `ramdisk_all_combined` | `$ANDROID_PRODUCT_OUT/ramdisk-all-combined.img` | The ramdisk image with all fragments concatenated, usually used for booting. |
| `ramdisk_custom` | `$ANDROID_PRODUCT_OUT/ramdisk-custom.img` | The custom fragment of ramdisk image. It includes all the files in `device/mainline/generic/prebuilts/ramdisk`. |
| `ukify_build` | `$ANDROID_PRODUCT_OUT/*.EFI` | Builds Unified Kernel Image, which is combination of kernel + cmdline + ramdisk, for booting in EFI environment. |

To build a target, run `m <target>`.

Note that during the build of some modules, it may fail due to missing dependencies from the build host OS.
If you encountered this, please install the mentioned dependencies and restart the build.
