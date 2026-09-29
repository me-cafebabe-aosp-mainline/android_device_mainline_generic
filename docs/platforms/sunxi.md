# Sunxi platform

## sun50i

Required additional boot parameters (as tested on `Remix Mini PC`):

```
androidboot.prefer_drm_render_name=lima memtest=2
```

Graphics sometimes works and sometimes does not. If it does not work,
try rebooting.
