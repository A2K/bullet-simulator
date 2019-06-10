# Bullet Simulator

![animation.gif](https://github.com/A2K/bullet-simulator/blob/master/animation.gif)

### Controls
##### Movement
* **WASD** or **Arrows** - up/left/down/right
* **LMB** - shoot
* **RMB** - place wall
#### Time control
* **Tab** - reverse time
* **1**...**9** - time acceleration
* **Shift**+**1**...**9** - time deceleration
* **0** - freeze time
#### Debug
* **~** - open console
* **;** or **F9** - toggle debug overlay
* **'** or **F10** - toggle events history overlay
* **F5** - reset world
#### Window
* **F11** or **Alt+Enter** - toggle fullscreen
* **Escape** - exit

### Networking

Press **~** to open the console. Type `help`.
#### Example commands:
* `host 1234` - start a server at port 1234
* `join 127.0.0.1 1234` - connect to port 1234 at 127.0.0.1
* `sync` - refresh world state from server


### Libraries used
* SDL
* SDL_ttf
* ENet
