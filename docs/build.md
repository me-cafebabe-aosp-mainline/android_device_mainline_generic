# Build

Before getting started, you should have synced the LineageOS source
tree from a branch that this repository targets.

## Obtaining the build dependencies

See [Build dependencies](build-dependencies.md) for the required
repositories and how to obtain them.

## Setting up the build environment

Enter the LineageOS source tree, then run:

```
source build/envsetup.sh
```

## Decide the target device to build

This device tree currently has the following target devices:

- `Generic_arm64`
- `Generic_x86_64`

The chosen target device is referred to as `<device>` below.

## Applying patches

See [Patches](patches.md) for the patches that must be applied.

## Select the target device

Run:

```
breakfast <device>
```

## Build the wanted target

Here is a table of the supported build targets:

| Target | Output path | Description |
|--------|-------------|-------------|
| `all_images` | `$ANDROID_PRODUCT_OUT/*.img` | Builds all images that this device tree provides. |
| `bootmgr-configs` | `$ANDROID_PRODUCT_OUT/bootmgr-configs` | Builds every boot manager configurations which this device tree currently have template of. |
| `liveisoimage` | `$ANDROID_PRODUCT_OUT/*-live.iso` | Live ISO image that can be written to CD-ROM-type media and boots a full Android environment without installation. Currently only supported for `Generic_x86_64`. |
| `ramdisk_all_combined` | `$ANDROID_PRODUCT_OUT/ramdisk-all-combined.img` | The ramdisk image with all fragments concatenated. This is the ramdisk typically used for booting. |
| `ramdisk_custom` | `$ANDROID_PRODUCT_OUT/ramdisk-custom.img` | The custom fragment of the ramdisk image. Includes all files in `device/mainline/generic/prebuilts/ramdisk`. |
| `ukify_build` | `$ANDROID_PRODUCT_OUT/*.EFI` | Builds a Unified Kernel Image (kernel + command line + ramdisk combined) for booting in an EFI environment. |

To build a target, run `m <target>`.

During the build of some modules, it may fail due to missing
dependencies on the build host. If this happens, install the mentioned
dependencies and restart the build.
