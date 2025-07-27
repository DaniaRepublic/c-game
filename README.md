Common pipelines in Makefile.

### Setup instructions
```bash
git clone https://github.com/DaniaRepublic/c-game.git
cd c-game
git submodule update --init --recursive
# to build and run the game, simply:
make
```

### Very general targets
  - [x] Animation         |22:25-04:02+1d|
  - [x] Tilemap view      |04:30-05:40+1d|
  - [x] GUI               |??:??-05:28+2d|
  - [x] Editor
    - [x] Tilemap         |07:01-17:08+2d|
  - [x] Saving
    - [x] Components      |??:??-02:00+1w|
    - [x] Complete state  |??:??-??:??+2w|
  - [ ] Load system       |xx:xx-xx:xx|
  - [ ] Shaders           |xx:xx-xx:xx|

### Notes
Raylib and Box2D have different directions of y-axis. \
All values are from perspective of Raylib as it's what i see. \
Conversions for Box2D and back are made when necessary. \

Conversions (0.1m is minimum distance in Box2D): \
0.1m in Box2D = 1px in Raylib

This means that 1 unit is one pixel in my game.

Virtual resolution is 640x360: \
2x = 1280x720 \
3x = 1920x1080 \
4x = 2560x1440 \
6x = 3840x2160

Physical dimensions of virtual world are 64m x 36m

### Steam relevant stats
  - https://store.steampowered.com/hwsurvey/
    - OS:    Windows 10 + Windows 11 is 96% of all players, so must run immaculately on them.
    - RAM:   Must run on 8GB of ram.
    - Cores: Must run on 4 cores.
    - Res:   1920 x 1080 and 2560 x 1440 make up 75% of all screens, test on them especially.
    - Langs: Most popular (% of all users): English - 36.50%, Simplified Chinese - 25.04%, Russian - 8.92%. 
  - https://store.steampowered.com/stats/content/
    - US and China have the most active user bases.

