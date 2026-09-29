# Build dependencies

This page lists the additional repositories required before building.

## Android device tree repositories

1. Clone the corresponding branch of [mainline
   local_manifests](https://github.com/me-cafebabe-aosp-mainline/local_manifests)
   to `.repo/local_manifests`.

2. Sync the source tree again with `repo sync`.

## Kernel source repositories

These are unlikely to be included in the LineageOS organization and
must be cloned manually.

The following kernel source repositories are required for the default
configuration of target devices in this device tree:

| Source | Destination | Branch |
|--------|-------------|--------|
| https://salsa.debian.org/kernel-team/linux/ | `external/debian-linux` | `debian/latest` |
| https://android.googlesource.com/kernel/common | `kernel/mainline/android-mainline` | `android-mainline` |

If you will use a different kernel configuration, some of these
repositories may not be needed.
