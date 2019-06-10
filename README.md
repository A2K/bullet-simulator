# Bullet Simulator

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
* **;** or **F10** - toggle debug overlay
* **'** or **F11** - toggle events history overlay
#### Window
* **F12** or **Alt+Enter** - toggle fullscreen
* **Escape** - exit

### Networking

Press **~** to open the console. Type `help`.
#### Example commands:
* `bind 1234` - start a server at port 1234
* `connect 127.0.0.1 1234` - connect to port 1234 at 127.0.0.1
* `sync` - refresh world state from server


### Libraries used
* SDL
* SDL_ttf
* ENet
