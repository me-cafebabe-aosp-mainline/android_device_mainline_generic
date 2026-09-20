# Build dependencies

## Android device tree related repositories

1. Clone the corresponding branch of [mainline local_manifests](https://github.com/me-cafebabe-aosp-mainline/local_manifests) to `.repo/local_manifests` path.

2. Sync your source tree again using `repo sync` command.

## Kernel source related repositories

These would likely never get into LineageOS org. You'll need to clone these manually.

Here is table of required kernel source repositories for the default configuration of target devices in this device tree:

| Source | Destination | Branch |
|--------|-------------|--------|
| https://salsa.debian.org/kernel-team/linux/ | `external/debian-linux` | `debian/latest` |
| https://android.googlesource.com/kernel/common | `kernel/mainline/android-mainline` | `android-mainline` |

If you will use a different kernel configuration, then these repositories might not be needed.
