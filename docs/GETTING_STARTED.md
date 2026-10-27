# Getting started: your own copy

For anyone who has just found this repository and wants to play: how to run the
site on your own computer and play MVP Baseball 2005 in your browser, from your
own disc image. No programming needed, and one command to start it.

What you get is the same site the owner runs (`deploy/Dockerfile`): the engine
built for WebAssembly, the page, and the small server behind it. It runs inside
Docker and is reachable from your computer only.

## What you need

- **Your own disc image** of MVP Baseball 2005 for GameCube, USA (`GV4E69`), the
  original release (revision 0), as `.iso`, `.gcm` or `.ciso`. This repository
  holds no game data, and the page refuses any other disc (other regions,
  revisions or games).
  - Dump it from a disc you own. This project never downloads game files, and you
    shouldn't either.
  - An `.rvz` from Dolphin must become an ISO first: in Dolphin, right-click the
    game, *Convert File...*, format *ISO*.
- **Docker**:
  - Windows or macOS (Apple silicon or Intel): [Docker Desktop](https://docs.docker.com/get-docker/).
    On Windows it uses Linux containers (the default).
  - Linux: Docker Engine with the Compose plugin.
- **About 8 GB of free disk space** for the first build. The finished image is
  about 350 MB. The rest is the build tools and the build cache, which Docker keeps
  so that later builds are quick.
- **A desktop browser with WebGPU:** a current Chrome or Edge, or Firefox 141 or
  newer on Windows (Safari 26 or newer should work but is untested), with hardware
  acceleration on.
- **A reasonably recent PC.** The development PC (an i7-14700K) runs the game at
  full speed with room to spare, and so does a machine half as fast. One about
  four times slower slows the game down.

## 1. Get the code

With git:

```
git clone https://github.com/ptitty12/mvp2005-decomp.git
cd mvp2005-decomp
```

Or, on the GitHub page, *Code*, *Download ZIP*, and unzip it.

## 2. Start it

- **Windows:** double-click `run-local.cmd` in the folder (or run `run-local` in a
  terminal there).
- **macOS, Linux:** `./run-local.sh` in a terminal in the folder.

The first start downloads the build tools (about 1 GB) and compiles the engine,
usually 5 to 15 minutes depending on the connection and the computer. Later starts
take seconds. When it's ready, it opens
http://localhost:8080 and prints three links:

| Link | What it's for |
|---|---|
| `http://localhost:8080/` | The game. Pick your disc image on the page |
| `http://localhost:8080/?admin=...` | Yours alone. The Server disc page (upload the disc once), roster packs kept on the server, save states, a memory card kept on the server |
| `http://localhost:8080/?key=...` | The player link. It lets a browser use the disc you uploaded |

The keys in those links are made on the first start and kept between runs.

## 3. Play

1. Open http://localhost:8080 and pick your disc image. The page checks it, then
   translates the game into WebAssembly in your browser. That takes a few seconds,
   and the result is cached for next time. The image never leaves your computer.
2. Sound starts after your first click or key press on the page. **M** turns it
   off and on.
3. Play with the keyboard, or connect a controller and press a button on it. The
   Controls panel changes any key and has a guided setup for controllers the
   browser doesn't recognize.

| Keyboard | GameCube |
|---|---|
| Enter | START |
| X, Z, C, V | A, B, X, Y |
| Space | Z |
| Q, E | L, R |
| I, J, K, L | Main stick |
| Arrow keys | D-pad |
| Numeric keypad 8, 4, 2, 6 | C-stick |

The page's own keys: **F** fullscreen, **M** sound, **P** pause, **`** statistics,
**F2** save state, **F4** load state.

**To skip picking the disc every time:** open the admin link once, follow *Server
disc* at the bottom of the page, and upload the image there. It is kept in Docker's
volume on your computer, and from then on the page loads it without asking: press
*Play*. A browser that has opened the admin link expects that uploaded disc and no
longer shows the picker.

## Online play

- **To try it on one computer:** upload the disc once (above), then *Create a
  game* in one browser window, copy the invite link and open it in a second
  window.
- **With a friend somewhere else:** both browsers must reach the same server over
  HTTPS, so your local copy (on your computer only) can't host them. Put the site
  on a server instead: [`engine/DEPLOY_DOKPLOY.md`](engine/DEPLOY_DOKPLOY.md) (a
  VPS with Dokploy) or [`engine/NETPLAY.md`](engine/NETPLAY.md) ("Deploying on a
  VPS"). The game still runs in each player's browser. The server serves the page,
  connects the two browsers, and relays their inputs when a network blocks a
  direct connection.
- The player link lets a browser play with the disc on your server. Send it only
  to people who own the game.

## Roster packs and gameplay mods

- **Roster packs** change rosters, players and team names. Add pack `.zip` files
  in the page's Roster packs panel before the game starts, or upload them on the
  Server disc page so the page offers them in a list. In an online game both
  players need the same packs. The format: [`MODDING.md`](MODDING.md).
- **Gameplay mods** are in the Gameplay mods panel. The one shipped so far is the
  ABS challenge: [`mods/ABS_CHALLENGE.md`](mods/ABS_CHALLENGE.md).

## Everyday use

| Windows | macOS, Linux | What it does |
|---|---|---|
| `run-local` | `./run-local.sh` | Build what changed, start, print the links, open the page |
| `run-local links` | `./run-local.sh links` | Print the links again |
| `run-local logs` | `./run-local.sh logs` | Follow the server's log |
| `run-local stop` | `./run-local.sh stop` | Stop it. The keys and your uploads are kept |
| `run-local reset` | `./run-local.sh reset` | Stop it and delete the keys, the uploaded disc, packs and save states (it asks first) |

- **Another port:** set `MVP_LOCAL_PORT` first, for example
  `MVP_LOCAL_PORT=8081 ./run-local.sh`, or `set MVP_LOCAL_PORT=8081` then
  `run-local` on Windows.
- **Updating:** `git pull` (or download the ZIP again), then start it again. Only what
  changed is rebuilt. After an engine update the browser translates the game
  once more.
- **Removing it:** `reset`, then delete the folder. Docker Desktop's *Images* and
  *Builds* pages (or `docker image rm mvp2005-web:local` and
  `docker builder prune`) free the space Docker kept.

## If something goes wrong

- **"Docker is installed but not running":** start Docker Desktop and wait until it
  says it is running.
- **Port 8080 is already in use:** use another port (above).
- **The page says WebGPU is unavailable:** update the browser and the graphics
  driver. In Chrome or Edge, `chrome://gpu` (or `edge://gpu`) should list WebGPU as
  hardware accelerated.
- **The page refuses the disc:** it runs only the USA disc `GV4E69`, revision 0.
  A different region or revision, or an image changed by a tool, is refused.
- **It works on this computer but not from another one on your network:** on
  purpose, the copy listens on this computer only. It couldn't work there anyway:
  a page loaded over plain HTTP from a network address gets no WebGPU and no
  browser storage. To play across computers, use a server (above).
- **The game is slow or choppy:** press **`** for statistics, and set *Picture*
  to *1x* under the game. Close other heavy programs.
- **Anything else:** `run-local logs` shows what the server did.

## For developers

- **The engine without Docker:** the browser build is in
  [`engine/WEB.md`](engine/WEB.md) §1 and the tool versions in
  [`engine/TOOLCHAIN.md`](engine/TOOLCHAIN.md). Those notes follow the owner's
  Windows machine. On Linux or macOS, `deploy/Dockerfile` is the exact recipe:
  Emscripten 6.0.12, emdawnwebgpu from Dawn release `v20261013.193955`, Ninja,
  `npm ci` in `engine/web`, then from `engine/`
  `cmake --preset emscripten -DMVP_EMDAWNWEBGPU_DIR=<emdawnwebgpu_pkg>` and
  `cmake --build --preset emscripten --target mvp-web`. Serve the result with
  `node engine/web/tools/serve.mjs` and open http://127.0.0.1:8000.
- **The engine's tests** need no game data:
  `ctest --preset emscripten` after a full build ([`engine/WEB.md`](engine/WEB.md) §1).
- **The decompilation** builds on Windows only (the original compilers are Win32
  programs) and needs the disc's files in `orig/GV4E69/`. See *Building the
  decomp* in the [README](../README.md). You don't need it to build, run or mod
  the engine.
- **The boundary between them:** the engine reads the decomp's symbol map only
  from its own pinned copy in `engine/decomp_pin/`, so decomp work never changes
  the engine until someone updates that copy on purpose
  ([`engine/decomp_pin/README.md`](../engine/decomp_pin/README.md)).
- **Rules** for anyone contributing, people and agents alike, are in
  [`CLAUDE.md`](../CLAUDE.md). The first one: never commit anything from the disc.
