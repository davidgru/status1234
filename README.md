# status1234

An extensible status bar implementation written in `C` for window managers like `i3wm` or `dwm`.

Example status:
```
+-------------------------------------------------------------------------------------+
| CPU 2%  RAM 1803/11722MiB(15%) |    5     100  | 03:03 28.05.2022 | dwm-6.3 |
+-------------------------------------------------------------------------------------+
```

Each block can decide on it's own, when it wants to update itsself. That enables a block to react immediately to certain events (e.g. changes in audio volume) as opposed to polling periodically (e.g. every second) which may lead to undesirable lag.

If a block updated itsself it can then force the statusbar to update. So the statusbar only redraws if and only if there is an actual update to a block.

Combined, this reduces redundant checks and redraws of the statusbar.

For an example of a block that updates ...
 * ... asynchronously, see the [audio](./src/block/saudio.c) block implementation.
 * ... periodically, see the [battery](./src/block/sbattery.c) block implementation.


## Dependencies
 * `pthread`
 * `X11` for displaying the status
 * `alsa` for [audio](./src/block/saudio.c) and [microphone](./src/block/smic.c)
 * `Font Awesome 6` for icons


## Installation
Install with
```
sudo make install
```
or
```
make && sudo make install
```

Run with
```
x11status
```

To run it with `startx`, add
```
x11status &
```
to your `.xinitrc` file.

Uninstall with
```
sudo make uninstall
```
