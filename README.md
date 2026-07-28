# Fallout Community Edition — WASD Movement Fork

This is a fork of [alexbatalov/fallout1-ce](https://github.com/alexbatalov/fallout1-ce) that adds **WASD walking** to Fallout 1. Contributions and playtesting feedback are welcome — open an issue or PR against the `feature/wasd-movement` branch.

Yes this is vibe-coded. I know just enough python to be dangerous but anything dealing with this game has been helped with using AI. I am just starting to learn C and would appreciate any help with this project. The goal is to modernize travel and eventually combat. 

## WASD controls

| Key | Action |
| --- | --- |
| `W` `A` `S` `D` | Walk continuously (exploration only; hold two keys, e.g. `W`+`D`, for the hex diagonals) |
| `Shift` (held) | Invert your click-to-move run/walk preference |
| `F` | Enter combat (moved from `A`, which now walks) |
| `Tab` | Toggle the inventory open/closed (was automap; `I` still works too) |
| `M` | Toggle the automap (was the cursor-mode toggle, which stays on right-click) |

Notes on the design:

- **Follow-cam:** during exploration the view stays centered on your character while he moves (WASD or click-to-move), gliding smoothly at pixel granularity rather than snapping tile by tile. Stand still and mouse-edge scrolling free-looks as usual; the camera stops at map bounds; combat and scripted scenes keep their own vanilla camera.
- Fallout's world is a hex grid with six movement directions, so 8-way WASD is mapped onto it: the four two-key diagonals map directly, and `W`/`S` alone alternate between the two upper/lower diagonals each step so travel reads as roughly straight up/down.
- Movement is exploration-only — combat is completely untouched and keeps vanilla turn-based action-point movement.
- `A` (was Enter Combat) and `S` (was Skilldex) are repurposed for walking. Enter Combat is now `F`; Skilldex remains available via its interface-bar button.

The upstream project description follows.

---

Fallout Community Edition is a fully working re-implementation of Fallout, with the same original gameplay, engine bugfixes, and some quality of life improvements, that works (mostly) hassle-free on multiple platforms.

There is also [Fallout 2 Community Edition](https://github.com/alexbatalov/fallout2-ce).

## Installation

You must own the game to play. Purchase your copy on [GOG](https://www.gog.com/game/fallout) or [Steam](https://store.steampowered.com/app/38400). Download latest [release](https://github.com/alexbatalov/fallout1-ce/releases) or build from source. You can also check latest [debug](https://github.com/alexbatalov/fallout1-ce/actions) build intended for testers.

### Windows

Download and copy `fallout-ce.exe` to your `Fallout` folder. It serves as a drop-in replacement for `falloutw.exe`.

### Linux

- Use Windows installation as a base - it contains data assets needed to play. Copy `Fallout` folder somewhere, for example `/home/john/Desktop/Fallout`.

- Alternatively you can extract the needed files from the GoG installer:

```console
$ sudo apt install innoextract
$ innoextract ~/Downloads/setup_fallout_2.1.0.18.exe -I app
$ mv app Fallout
```

- Download and copy `fallout-ce` to this folder.

- Install [SDL2](https://libsdl.org/download-2.0.php):

```console
$ sudo apt install libsdl2-2.0-0
```

- Run `./fallout-ce`.

### macOS

> **NOTE**: macOS 10.11 (El Capitan) or higher is required. Runs natively on Intel-based Macs and Apple Silicon.

- Use Windows installation as a base - it contains data assets needed to play. Copy `Fallout` folder somewhere, for example `/Applications/Fallout`.

- Alternatively you can use Fallout from MacPlay/The Omni Group as a base - you need to extract game assets from the original bundle. Mount CD/DMG, right click `Fallout` -> `Show Package Contents`, navigate to `Contents/Resources`. Copy `GameData` folder somewhere, for example `/Applications/Fallout`.

- Or if you're a Terminal user and have Homebrew installed you can extract the needed files from the GoG installer:

```console
$ brew install innoextract
$ innoextract ~/Downloads/setup_fallout_2.1.0.18.exe -I app
$ mv app /Applications/Fallout
```

- Download and copy `fallout-ce.app` to this folder.

- Run `fallout-ce.app`.

## Configuration

The main configuration file is `fallout.cfg`. There are several important settings you might need to adjust for your installation. Depending on your Fallout distribution main game assets `master.dat`, `critter.dat`, and `data` folder might be either all lowercased, or all uppercased. You can either update `master_dat`, `critter_dat`, `master_patches` and `critter_patches` settings to match your file names, or rename files to match entries in your `fallout.cfg`.

The `sound` folder (with `music` folder inside) might be located either in `data` folder, or be in the Fallout folder. Update `music_path1` setting to match your hierarchy, usually it's `data/sound/music/` or `sound/music/`. Make sure it match your path exactly (so it might be `SOUND/MUSIC/` if you've installed Fallout from CD). Music files themselves (with `ACM` extension) should be all uppercased, regardless of `sound` and `music` folders.

The second configuration file is `f1_res.ini`. Use it to change game window size and enable/disable fullscreen mode.

```ini
[MAIN]
SCR_WIDTH=1280
SCR_HEIGHT=720
WINDOWED=1
```

Use any size you see fit. In time this stuff will receive in-game interface, right now you have to do it manually.

## Contributing

Here is a couple of current goals. Open up an issue if you have suggestion or feature request.

- **Update to v1.2**. This project is based on Reference Edition which implements v1.1 released in November 1997. There is a newer v1.2 released in March 1998 which at least contains important multilingual support.

- **Backport some Fallout 2 features**. Fallout 2 (with some Sfall additions) added many great improvements and quality of life enhancements to the original Fallout engine. Many deserve to be backported to Fallout 1. Keep in mind this is a different game, with slightly different gameplay balance (which is a fragile thing on its own).

## License

The source code is this repository is available under the [Sustainable Use License](LICENSE.md).
