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
  - [x] Animation     |22:25-04:02+1d|
  - [x] Tilemap view  |04:30-05:40+1d|
  - [x] GUI           |??:??-05:28+2d|
  - [ ] Editor        |07:01-xx:xx|
    - [x] Tilemap     |07:01-17:08+2d|
  - [x] Saving        |??:??-00:00+8d|
    - [x] Components  |??:??-02:00+1w|
  - [ ] Shaders       |xx:xx-xx:xx|

### Notes
Refactor code put ALL data in components.
Editor needs entity picker. Also adding physics to editor entities.

### Steam relevant stats
  - https://store.steampowered.com/hwsurvey/
    - OS:    Windows 10 + Windows 11 is 96% of all players, so must run immaculately on them.
    - RAM:   Must run on 8GB of ram.
    - Cores: Must run on 4 cores.
    - Res:   1920 x 1080 and 2560 x 1440 make up 75% of all screens, test on them especially.
    - Langs: Most popular (% of all users): English - 36.50%, Simplified Chinese - 25.04%, Russian - 8.92%. 
  - https://store.steampowered.com/stats/content/
    - US and China have the most active user bases.

