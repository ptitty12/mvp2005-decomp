# Determinism inventory and main loop (K3, K4)

Why this exists: netplay runs the retail game code in two browsers and exchanges only
controller inputs ([`ARCHITECTURE.md`](ARCHITECTURE.md) §8). Both machines must compute
the same state from the same inputs. So every source of difference must be known and
controlled by the engine's runtime: time, random seeds, async completions, callbacks
and threads. This file lists them (knowledge target **K3**) and records how the main
loop and frame pacing work (**K4**). E0b and E4 implement the requirements in §6.

- Written 2026-10-14 against `main` at `fce2ead3`, reading the split assembly of the
  built tree and the matched C in `src/`. SDK names come from `config/GV4E69/symbols.txt`
  and, where ours is still `fn_`, from the mitsevox/mvp2005 symbols (marked `*`).
- Evidence tags: **V** = seen in the assembly or C (address given); **I** = inferred from
  structure, strings or call patterns, not proven. Nothing here was run.
- Static limits: virtual calls and callbacks hide edges from the call graph, so "no
  caller found" never proves "not called". Where that matters it is stated.
- Nothing was renamed. Proposed names are in §7.

## 1. Verdicts

### 1.1 Frame rate: gameplay is 60 fps (59.94 Hz), with a catch-up path

**Verdict: 60 fps nominal, not 30.** The simulation advances in fixed ticks of
1/60 s. One tick is normally followed by one render and one wait for a VI retrace
(59.94 Hz for 480i NTSC, 60.00 Hz in 480p). Under load the game does **not** drop to
a fixed 30 fps: it runs up to 3 ticks before rendering again, so game time keeps pace
with the clock down to about 20 rendered frames per second. Evidence:

1. **Boot prints the rate.** `fn_8014BEB8` calls `fn_803FF888(0x3C)` and then prints
   `"UPDATEhz = %d\n"` (8014C09C..8014C0B8). That function starts a 60 Hz periodic
   OS alarm (§3.5). **V**
2. **The game clock is integer ticks of 1/60 s.** `Owner::m4C` is the tick counter,
   `fn_801A08D8` increments it, `fn_801A0744` returns a time in seconds, and
   `fn_801A0874` tests `(int)(t * 60.0f) + 2 <= m4C` (constant `lbl_805CE630` = 60.0f,
   `lbl_805CE628` = 0.016666668f). **V**
3. **There is no per-frame float dt.** Nothing passes a measured dt to the sim. The
   constant 1/60 = 0.016666668f appears as a private literal in **405 functions**
   (it converts tick counts to seconds). 1/30 = 0.033333335f appears in 24 functions,
   so it is a secondary constant, not the step. (Counted per function literal pool in
   the split asm; the 664 / 55 reference counts quoted elsewhere count each use.) **V**
4. **The frame function counts how many ticks to run, from elapsed time.**
   `fn_80148AB4` keeps an accumulator in units of 60 Hz frames (`lbl_8061B918`),
   adds the time since the previous frame start, takes the integer part as the tick
   count `n`, keeps the remainder, forces `n >= 1`, caps `n` at 3 (80148BC0..80148BC8)
   and forces `n = 3` on the first frame after init (`lbl_8061B91C`, set in
   `fn_80148A40`). Details in §2.4. **V**
5. **Display: one wait for the retrace per rendered frame.** The EA graphics device
   uses a single external frame buffer, copies the EFB into it once per frame, and blocks
   on a one-slot message queue that the VI post-retrace callback fills (§2.5). It
   never waits two retraces. **V**
6. **Video modes are the full-rate ones.** The NTSC render mode (`lbl_80634AA0`) is
   `VI_TVMODE_NTSC_INT`, 640x480, `xFBmode = VI_XFBMODE_DF`, frame rendering
   (`field_rendering = 0`); the 480p mode is selectable in the frontend
   (`fn_80313634`/`fn_803136A0`, called from `fn_804EB23C`). The game copies a new
   image every retrace. **V**

Consequence for netplay: a lockstep "frame" is one pass of the outer loop (one `PADRead`),
not one logic tick. The tick count `n` per pass must be identical on both machines;
it is derived from the time base, so the virtual clock decides it (R1, R2, R4).

### 1.2 The five riskiest sources

1. **Frame catch-up from `mftb` deltas** (`fn_80148AB4`): the number of sim ticks per
   frame, the render skip and the forced `n = 3` all depend on measured time. If the
   two machines disagree on `n` once, the sim diverges.
2. **Time-seeded game RNG**: the simulation PRNG is seeded from `mftb * 12` when a game
   is created (`fn_8014CBE8`, 8 sites; `fn_8017AC7C`), and libc `srand(mftb * 12)` runs at
   the top of every session loop (`fn_8014C8BC`). Different virtual time means a
   different game from the first pitch.
3. **The EA 60 Hz OS-alarm tick** (`fn_803FF888` -> alarm `fn_803FFA3C` -> thread
   `fn_8040024C` -> counter `lbl_806EEAE0`): an asynchronous counter that the main
   thread samples mid-computation in boot waits, loading screens, UI timers and the
   input code. It is delivered by a high-priority thread, so its exact preemption
   point matters.
4. **Five threads with spin-wait loops** (main, tick, file I/O, audio, memory-card
   worker): the main thread busy-waits on state that other threads change
   (`fn_803F99E8`, `fn_80400E18`, the 240-tick wait in `fn_8014C8BC`, `fn_802D77F0`).
   The result depends on where the scheduler preempts, and on DVD completion time.
5. **Shared global `rand()` stream consumed by presentation code**: 153 functions call
   libc `rand`, including crowd-audio selection (`fn_80040E9C`, `bank_ump` /
   `CROWD4A.ABK`), effects and frontend code. The SDK memory-card code also calls
   `rand`. Any difference in how often these paths run (audio state, frame skip, card
   state) shifts the stream for everything else that uses it.

### 1.3 What is **not** a problem (verified)

- A scan of game/EA code for `lis 0x8000` / `0xC000` / `0xCC0x` / `0xE000` / `0x7E00`
  followed by a load or store found only the GX write-gather FIFO (0xCC008000) and two
  low-memory words, the bus and CPU clocks at 0x800000F8 and 0x800000FC (5 functions,
  §3.1). **V** for that pattern; other forms of absolute addressing were not searched.
- No game code uses `fres`, `ps_res` or `ps_rsqrte`. `frsqrte` appears only in the SDK
  (`GXInitFogAdjTable`, `PADClampCircle`). `lwarx`/`stwcx.`, `dcbz` and `tw` do not
  appear in game/EA code. **V**
- Neither random stream is touched by the audio thread, the file thread or the tick
  thread: no function in `snd/`, and none of the thread entry points, calls `rand`
  or the ADC generator (direct callers only). **V** (direct calls)

## 2. Main loop and frame pacing (K4)

### 2.1 Boot (before the loop)

`__start` (0x80003100) clears BSS, reads the boot-info pointer at 0x800000F4 for
`argc` / `argv` (`fn_80003254`: 0 and 0 when the pointer is null), calls `OSInit`, and calls
`fn_8014D63C` = `main` at 8000324C. **V** `main` first calls `__main` (static
constructors), then `fn_8014C5A4`, which calls the boot init `fn_8014BEB8` and then
builds the managers, and then `main` enters the loop. The boot path in order (names are
strings the code prints, or SDK calls):

| Step | Function | What it does | Tag |
|---|---|---|---|
| 1 | `fn_8014BEB8` (boot init, called first by `fn_8014C5A4`) | `OSInit`, `DVDInit`, `VIInit`, `PADInit`; `"IOP initialized."`; `fn_8000D650` (calls `fn_8000E624`: `VMInit`, §3.7 AC9); `"SKU Mem Initialized."`; `fn_8000E65C` (creates the VM heap); `fn_803FFE14` (records the main thread handle in `lbl_806EF47C`); `"Threads initialized."`; **`fn_803FF888(0x3C)`** (60 Hz tick); `"UPDATEhz = %d"`; `"Beast memory initialized"`; `"Init cPad object"` (`Unk802D73B4_W` ctor, the pad manager); `fn_80400DE4` + `fn_80400E18` (a timed wait); `fn_8000D6D4` (first `PADRead`, reset code and progressive-scan check); `fn_80316310("firstscr tex")` (first splash screen, stores a tick in `lbl_8061FF3C`); `DVDChangeDir("DATA")`; `"Init complete!"` | V |
| 2 | `fn_8014C5A4` (rest) | Registers a shutdown callback (`fn_80400690`), builds the debug-text classes and the object managers, then prints `"Entering game loop."` | V |

### 2.2 The session loop in `main`

The loop at 8014D664..8014D6A0 is **not** the frame loop. One pass of it is a whole
session (frontend, then one game, then back). The frame loop is inside
`fn_80148930`.

```
main(argc, argv):                              // fn_8014D63C
    __main(); fn_8014C5A4(argc, argv);         // boot, "Entering game loop."
    for (;;) {
        fn_8014C8BC();                         // 8014D664  begin session
        fn_80148930(lbl_8068B618);             // 8014D670  RUN: frontend frame loop
        if (fn_8014D1EC() == 1)                // 8014D674  start game; 1 = game created
            while (fn_80148930(lbl_8068B618) - 2 <= 1u)   // 8014D688  RUN: game frame loop
                fn_8014D5C0();                 // result 2 or 3: restart / rematch
        fn_8014D488();                         // 8014D69C  end game, tear down
    }
```
(Matches `src/game/tu_8014BEA4.cpp`. `lbl_8068B618` is the application object whose
`+0x20` is the "exit loop" result and `+0x08..+0x10` are three player slots.)

| Phase | Function | What it does (evidence) | Tag |
|---|---|---|---|
| begin session | `fn_8014C8BC` (812 B) | Prints `"Initialize new game"`, `"Referencing INI file for the first time."`, `"Run Frontend..."`; **`srand(mftb * 12)`** (8014C91C..8014C92C) and stores the seed in `lbl_8061B994` (write-only: no reader found). If `lbl_8061B9A4` != 0 (initial value 1: first session only): busy-waits until the EA tick reaches `lbl_8061FF3C + 0xF0` (240 ticks = 4 s after the first splash screen, 8014C970..8014C984), then runs its own `fn_80148930` loop between calls on the frontend manager (I: the startup sequence, intro screens). Otherwise prints `"Enter normal frontend."` and builds the frontend (manager `lbl_8061A32C` = the `cFEManager` singleton). Finally resets the game RNG state to seed 0 (`fn_8014D860(.., 0)` at 8014CBC0) | V |
| frontend run | `fn_80148930` | Frame loop, §2.3. Returns `app->m20` | V |
| start game | `fn_8014D1EC` (668 B) | Clears `lbl_8061B9A4`, destroys the previous objects (`lbl_8061B998`, `lbl_8061B9A0`), calls `fn_801498C0` (**LoadGameAssets**; its body prints `"end of LoadGameAssets"`), returns 0 if loading failed. Otherwise calls `fn_8014CBE8` (creates the game by type, §3.2) and returns 1 | V |
| game run | `fn_80148930` | Same frame loop, now driving the game objects | V |
| restart | `fn_8014D5C0` | `fn_802D7538` (pad manager `s11(-1, -1)` and clear `lbl_8061EE1C`), then loops over the player slots (`fn_80148404` counts the non-null ones, `fn_8006A070` per player), then `fn_80148438` (virtual call on each slot) | V |
| end game | `fn_8014D488` | Calls the frontend manager (vtable), deletes the two objects `StartGame` created (`lbl_8061B99C`, `lbl_8061B998`), then `fn_802B832C`, `fn_80149DD4` (cleanup), `fn_80312018`, `fn_802BB1E4(.., 0)`, `fn_8014C4E8` | V |

`fn_8014CBE8` (1540 B) picks the game type with strings such as `"Batter/Pitcher duel!"`,
`"Batting practice!"`, `"TPGAMETYPE_PITCHINGMINIGAME!"`, `"Pitching practice!"`,
`"Scouting Report!"`, `"Homerun Showdown Mode!"`, `"Baseball mode!"` and
`"Preview Stadium mode!"`, and creates the game object (`__nw__FUii(0x798)`).

### 2.3 The frame loop

> **Correction (E0c, 2026-10-14, measured):** `PADRead`'s only caller is fn_8000A44C. It is reached
> through the per-tick update (fn_80148698 → fn_802D75D4 → fn_8035F3A0 → `Unk802D73B4_Mgr::s13`), so
> **PADRead runs once per logic tick** (1-3 per frame), plus once at boot: 1,190 PADReads in 701 frame
> polls. The path through fn_802D7588/s6 described below never reaches PADRead in measured runs.
> The engine's input queue is still keyed by the frame poll (fn_8014865C, E0c-IN1); every tick in a
> frame sees that frame's input.

`fn_80148930(self)` (272 B), `self = lbl_8068B618`: prints `"top of game loop!"`, zeroes
the counters `lbl_8061B8F0/8F4/8C4`, creates the `cFEManager` if missing, calls
`fn_80148604` (clears `self+0x20` and `self+0x24`), prints `"after init!"`, calls
`fn_80148A40` (frame-timer init) and then:

```
do {
    fn_8014865C();         // lock; fn_802D7588 (poll controllers); unlock
    fn_80148AB4(self);     // one frame (3588 B)
} while (self->m20 == 0);
// then: lock; fn_80400498(fn_8014865C); unlock; return self->m20
```
(V, 80148930..80148A3C.) `fn_80400498(fn_8014865C)` removes `fn_8014865C` from the EA tick
callback table `lbl_8062EE40` (§3.6, CB10). No code adds it, so in retail it is a no-op; it
suggests a dev build ran the pad poll from the 60 Hz tick thread (**I**). The lock is an `OSMutex` wrapper (`fn_80400828` = `OSLockMutex`,
`fn_8040084C` = `OSUnlockMutex`) on `lbl_8068B5FC`.

**Controller polling happens once per pass of this loop**, not once per logic tick:
`fn_8014865C` -> `fn_802D7588` -> `Unk802D73B4_Mgr::s6` -> each pad's poll
(`fn_8000A44C`, which calls `PADRead`, `PADClampCircle` and the dead-zone code).
`PADRead` has only two game-side callers, `fn_8000A44C` and the boot-time
`fn_8000D6D4`. The lockstep input boundary is therefore the pass of `fn_80148930`'s
loop. **V** for the call chain to `fn_802D7588`; the link from `s6` to `fn_8000A44C`
goes through a vtable and is **I** (the C comment on `Unk800085B0::vslot42` calls it
the PAD poll).

### 2.4 One frame (`fn_80148AB4`): variable tick count

Time units: `fn_8014849C` = `mftb * 12` (CPU cycles: the time base runs at 40.5 MHz
and the CPU at 486 MHz). `fn_80148580` computes `lbl_8068B5F4 = fn_804004D0() / 60`
(CPU cycles per 1/60 s; `fn_804004D0` returns the CPU clock read from 0x800000FC).
`fn_801484A8(i)` / `fn_801484E4(i)` store a cycle stamp as "start" / "end" slot `i`;
`fn_80148520(i)` returns `(end[i] - start[i]) / lbl_8068B5F4` as a float (unsigned 32-bit
difference), i.e. elapsed time in **60 Hz frames**. **V**

Pseudo-code of the first third of `fn_80148AB4` (80148AD0..80148E20):

```
fn_802E3A44(lbl_8061F688);
fn_801484E4(0);                          // end[0] = now
A = fn_80148520(0);                      // time since the previous frame start (60 Hz frames)
B = fn_80148520(1);                      // previous frame: start to end of its tick loop
accum += (A - B) + B;                    // = A; lbl_8061B918, float
n = (int)accum;  accum -= (float)n;  if (accum < 0) accum = 0;
if (n <= 0) { accum = 0; n = 1; }        // always at least one tick
fn_801484A8(0); fn_801484A8(1);          // new start stamps
if (n > 3) n = 3;                        // never more than 3 ticks per frame
if (lbl_8061B91C) { lbl_8061B91C = 0; n = 3; }   // first frame after init
lbl_8061B884 = n;
for each object i in [0, lbl_806202EC): obj_i->m55C = n;   // per-object pending ticks
do {
    fn_80400CE4(0);                      // pump EA timers (§3.5)
    if (n > 0) { lbl_8061B8F8 = 1; fn_80148698(self); lbl_8061B8F8 = 0; n--; more = n > 0; }
    ... per-object loops (fn_80148790, fn_8014887C, catch-up while fn_801A0744(o) < o->m4C / 60) ...
} while (!exit && more);
```
Facts that follow (all **V** unless marked):

- Remaining fractions are kept (`accum`), but ticks beyond 3 are dropped: if a frame
  took 5/60 s, 3 ticks run and the extra 2 are forgotten (`accum -= (float)n` uses the
  uncapped `n`).
- A frame that takes exactly one 59.94 Hz retrace (1.001 sixtieths) gives `n = 1`;
  the 0.1% surplus accumulates, so about every 1000 frames (16.7 s) one frame runs
  `n = 2`. This is the game running 0.1% faster than the display and catching up.
- `fn_80148698` is **one logic tick**: lock; `lbl_8061B8C4++`; if the pending-tick
  counter `lbl_8061B8F4` is 0, loop until `fn_802D7A84(&x)` returns nonzero with
  `x > 0` and store `x` there (`fn_802D7A84` calls `Unk802D73B4_Mgr::s5(int*)`, address
  802D8B5C, which in retail is `return 1` and leaves `x = 1`); decrement
  `lbl_8061B8F4`; call `fn_802D75D4` with the first player's sub-object field (pad
  manager per-tick update); unlock; `lbl_8061B8F0++`; **`fn_803289C0(0)`** (the process
  manager's update command 0x11); then `if (self->m2C > 0) self->m2C--`. So `s5` is a
  hook where a network layer could say "you may run `x` ticks now"; retail always
  grants one.
- `fn_80320878` runs a virtual method up to `lbl_8061B884` times (`n`, capped at 3), so
  those objects (I: effects) tick `n` times per frame to stay at 60 Hz.
- The tail of the frame (80149788..8014987C) is the **render phase**:
  `fn_80329498`, `fn_8031CECC`, `fn_8031D7E8`. It runs only when `self->m28 == 0`
  and not `(self->m24 == 0 && self->m2C > 0)`, and then only if `self->m24 == 1` or the
  process-1 clock `fn_801A0744(obj)` is not ahead of `obj->m4C / 60`. So **render is
  skipped on some frames** (`m2C` is a render-skip countdown, set by loaders). The
  exact meaning of `m24`/`m28` is **I**.
- The middle of the function updates debug HUD numbers (FPS, ticks per second, VM
  statistics, `"Timer Ticks"`), using `fn_80400DA4` / `fn_80400E84` (tick counter and
  tick rate, §3.5). None of that feeds the sim. **I** (read, not traced to the end).

### 2.5 Display pacing (EA graphics device, `ealib/tu_803D8234`)

- **Setup** (`fn_803D8918`, 2040 B): `VIGetTvFormat` selects the render mode
  (NTSC `lbl_80634AA0`, PAL `lbl_80634E9C`, MPAL `lbl_80634CF8`, strings `"VI_NTSC\n"`,
  `"VI_PAL\n"`, `"VI_MPAL\n"`), allocates **one** XFB (`w * h * 2`, filled with
  0x10801080 = black), `VIConfigure`, `GXInit`, then `GXSetCopyClear`,
  `VISetPostRetraceCallback(fn_803D9378)`, `GXSetDrawSyncCallback(fn_803D9454)`,
  `GXSetBreakPtCallback(fn_803D95C4)`, creates the vsync queue `lbl_80627390`
  (`OSInitMessageQueue(.., 1)`, **capacity 1**) and primes it with one message.
  It ends with `GXCopyDisp`, `VISetNextFrameBuffer(xfb)`, `VIFlush` and **two**
  `VIWaitForRetrace` calls. After this, the XFB pointer never changes. **V**
- **Present** (`fn_803D8640`, virtual method at vtable `lbl_80686740`+0x14):
  `fn_803E2D18` (profiler stamp), fade overlay if any (`fn_803D83BC`),
  `GXSetZMode`, `GXSetColorUpdate`, then **`OSReceiveMessage(lbl_80627390, .., BLOCK)`**
  at 803D8770, `GXSetDrawSync(0x8000)`, `fn_803D95FC(0, fn_803D8828)` (queue a
  callback to run when the GP reaches this point), **`GXCopyDisp(xfb, clear)`**,
  `GXFlush`, then bookkeeping. **V**
- **Retrace callback** `fn_803D9378` (post-retrace): if the "GP finished" flag
  `lbl_806272E4` is set and no message is outstanding (`lbl_806272D4 == 0`, or vsync is
  off; the present code clears `lbl_806272D4` after each receive), clear it, run the
  deferred-callback ring (`fn_803D9664`), count down the fade-in black (`VISetBlack(0)`
  + `VIFlush` when it hits 0), store `mftb * 12` deltas in `lbl_806272F0/F4` (stats),
  set `lbl_806272D4 = 1` and **`OSSendMessage(vsync queue, .., NOBLOCK)`**. **V**
- **GP-frame-done callback** `fn_803D8828` (queued through `fn_803D95FC`): sets
  `lbl_806272E4 = 1`; when vsync is off (`lbl_806272E8 == 0`) it does the retrace
  callback's work itself, so the queue is filled without waiting for a retrace. **V**
- `lbl_806272E8` is `dev->mDC`, written by `fn_803D982C` (SetVSync). Callers pass
  `!lbl_8061FEAC` (`fn_80313E00`) or 0 if `lbl_8061FEAC` is set (`fn_803134A0`):
  `lbl_8061FEAC` is a debug "unthrottled" flag that is 0 in retail data, so **vsync is
  on**. **V** for the code, **I** for the flag's retail value.
- **Net effect**: the CPU blocks for the retrace *before* it starts the copy of
  frame N. A one-slot queue means a frame that took longer than a retrace finds the
  message already waiting and does not block, so the game falls back to "as fast as it
  can, at most one frame per retrace". There is no `VIWaitForRetrace` in steady state
  (its callers are the mode-switch code `fn_803D8918`, `fn_80313634`, `fn_803136A0`,
  `fn_8031370C`, and the reset path `fn_800043AC`). `GXDrawDone` is called only by
  `fn_803D987C` (GPU idle wait, used by `ealib/tu_803E3C08` resource code) and by the
  reset path. **V**
- GX FIFO pacing: `fn_803D9774` / `fn_803D96FC` enable and disable a GX breakpoint on
  the FIFO read pointer; `fn_803D95C4` (breakpoint callback) runs the next queued
  callback from the ring `lbl_806273B4` (128 records of 12 bytes, index `lbl_8062734C`
  / `lbl_80627348`). **V**
- Who calls `Present` each frame: one verified caller is the loading-screen state code
  `fn_80313E00(self, 3)`; the gameplay path goes through the render phase's virtual
  calls and was not traced. **I**

### 2.6 Where time and frames come from, in one table

| Clock | Source | Rate | Used for | Tag |
|---|---|---|---|---|
| CPU cycle stamps | `mftb * 12` (`fn_8014849C`) | 486 MHz | frame catch-up, profiler | V |
| EA system tick | OS periodic alarm, 60 Hz, via a thread (§3.5) | 60.000 Hz of virtual time | waits, UI timers, loading screens | V |
| VI retrace | `VISetPostRetraceCallback` -> message queue | 59.94 Hz (480i) / 60 Hz (480p) | frame wait | V |
| Sim tick | integer count, `n` per frame | 60 per game-second | everything in the sim | V |

The EA tick runs at exactly 60 Hz and the retrace at 59.94 Hz, so they drift by one
tick per 1000 frames (about 16.7 s). That drift is real retail behaviour and is fully
deterministic if the virtual clock is.

### 2.7 K4 function table

K4 is `main` plus two levels (81 functions, `tools/goals/targets.py --list K4`). Only the
ones whose role is established are named here; the rest are leaf helpers I did not
analyze (screen/fade helpers `fn_80316xxx`, the process-manager accessors
`fn_803285A0` and friends, allocator wrappers).

| Address | Placeholder | Role | Tag |
|---|---|---|---|
| 8014D63C | `fn_8014D63C` | `main` | V |
| 8014BEB8 | `fn_8014BEB8` | boot init | V |
| 8014C5A4 | `fn_8014C5A4` | boot, ends with `"Entering game loop."` | V |
| 8014C8BC | `fn_8014C8BC` | begin session, `srand` | V |
| 8014D1EC | `fn_8014D1EC` | start game | V |
| 8014CBE8 | `fn_8014CBE8` | create game by type, seeds RNG | V |
| 801498C0 | `fn_801498C0` | LoadGameAssets | V |
| 8014D488 | `fn_8014D488` | end game | V |
| 8014D5C0 | `fn_8014D5C0` | restart / next game round | V |
| 80148930 | `fn_80148930` | RunGameLoop (frame loop) | V |
| 80148AB4 | `fn_80148AB4` | one frame | V |
| 80148698 | `fn_80148698` | one logic tick | V |
| 8014865C | `fn_8014865C` | pre-frame lock + controller poll | V |
| 80148A40 | `fn_80148A40` | frame-timer init | V |
| 80148580 | `fn_80148580` | cycles per 1/60 s | V |
| 8014849C | `fn_8014849C` | cycle counter (`mftb * 12`) | V |
| 801484A8 / 801484E4 / 80148520 | | start stamp / end stamp / elapsed in 60 Hz frames | V |
| 802D7588 | `fn_802D7588` | poll pads (gated by `lbl_8061EE10`) | V |
| 802D75D4 | `fn_802D75D4` | per-tick pad manager update | V |
| 80316614 | `fn_80316614` | DrawLoadingProgress(obj, stage, fraction) (string `"DrawLoadingProgress: %d, %.3f @ %.3f seconds"`) | V |
| 803D8640 | | Present / EndFrame (gfx device, virtual) | V |

## 3. Determinism inventory (K3)

Columns: **Where** = address(es). **State** = what game state it can reach. **Control** =
how the engine makes it deterministic (requirement ids refer to §6).

### 3.1 Time-base reads

| ID | Where | Name | What it does | State it can affect | Control | Tag |
|---|---|---|---|---|---|---|
| TB1 | 80438628 | `OSGetTime`* | 64-bit time base (`mftbu`/`mftb` loop). 22 callers: SDK internals (AI, AX, CARD, EXI, GX abort, exception and interrupt dispatch, `PADInit`, VM) plus game-side `fn_803FF888` (EA tick init), `fn_8038DB54`/`fn_8038DF3C`/`fn_8038E468` (video player sync, called from the render phase `fn_8031D7E8`) | EA tick phase; video pacing, which sets when intros end and so when later seeds are read | R1 | V |
| TB2 | 80438640 | `OSGetTick`* | low 32 bits. 5 callers, all SDK (CARD unlock, audio system init/stop, VM page replacement) | none in game code | R1 | V |
| TB3 | 80438648 | `__OSGetSystemTime` | 15 SDK callers (DVD interrupt handler and read, alarms, SI, reset) | completion times inside SDK code | R1; moot if the SDK is replaced by name | V |
| TB4 | 8014849C | cycle counter | `mftb * 12`; callers `fn_801484A8` / `fn_801484E4` (frame stamps) | **frame catch-up `n`** (§2.4) and the profiler bars | R1, R2 | V |
| TB5 | 8014C91C | in `fn_8014C8BC` | `srand(mftb * 12)` once per session | the libc `rand` stream (RN1) | R1, R3 | V |
| TB6 | 8014CC8C, 8014CCF8, 8014CD84, 8014CE0C, 8014CE94, 8014CEDC, 8014CF24, 8014D00C | in `fn_8014CBE8` | `seed = mftb * 12`; `game->m24 = seed`; `fn_8014D860(game + 0x28, seed)`. The last site seeds a second object with `~seed` | **the simulation RNG** (RN2), one per game mode | R1, R3 | V |
| TB7 | 8017ACAC | in `fn_8017AC7C` (called from `fn_801A41D0`) | same: `m24 = mftb * 12`, reseed | simulation RNG on (re)start of a game | R1, R3 | V |
| TB8 | 8017AF10 | in `fn_8017AD8C` (no direct caller) | `obj->m408 = mftb * 12`, after `fn_8037893C(.., 0x2b, ..)` | a timestamp whose reader I did not find | R1; check readers in E0b | V write, reader unknown |
| TB9 | 80047724, 8004774C | in `fn_8004763C` (ctor helper of a class in `tu_80046964`, via `fn_800475EC`) | `r = rand() & 0x3FFFFFFF`; stores `r + (mftb*12) % 1000` if `r < 1000`, else `r - (mftb*12) % 1000`, at `obj+0xC04` | a per-object random value (looks like an 8-channel manager) | R1, R3 | V |
| TB10 | 8005B658, 8005B6A4 | `fn_8005B658` (called from ctor `fn_8005ABA0`) | shuffles the 8-entry array at `obj+0x80` using `(mftb * 12) % 9` as swap index | order of 8 items (a class that also constructs `Unk800401E4`, the crowd-audio object) | R1 | V |
| TB11 | 804EB544 | in `fn_804EB2C8` (frontend start / legal screens, strings `"LEGAL"`, `"POPUP"`) | `19 + (abs(mftb * 12) % 38)` | a random value in 19..56 in the frontend | R1 | V; purpose I |
| TB12 | 803D9400, 803D88B4 | `fn_803D9378`, `fn_803D8828` | `mftb * 12` delta between retraces into `lbl_806272F0/F4` | stats only | none needed | V |
| TB13 | `fn_803E2D18`, `fn_803E2E18`, `fn_803E30A4`, `fn_803E30D0`, `fn_803E3178` (`ealib/tu_803E2248`) | profiler (strings `"PROFILER"`, `"UNPROFILED"`) | stamps for the on-screen profiler | display only | none needed | V |
| TB14 | 804360E4 | `Reset`* (SDK) | reset-button debounce | none (button never pressed) | R12 | V |
| TB15 | 0x800000F8, 0x800000FC | low memory | bus clock (162 MHz) and CPU clock (486 MHz). Read by `fn_803FF888`, `fn_803FFFA8`, `fn_8038DB54`, `fn_8038E080`, `fn_804004D0` | alarm period, sleep length, frame-time unit | R1 (fixed values) | V |

Notes: game and EA code read only the low half, `mftb rD, 268` (TBL), 27 instructions
in 15 functions; the upper half (`mftb rD, 269`) is read only by `OSGetTime` (a retry
loop). So game code sees a 32-bit value that wraps every 106 s (every 8.9 s after
`* 12`). `fn_80148520` subtracts as unsigned 32-bit, which tolerates the wrap. **V**

### 3.2 Random number generators

| ID | Where | Name | Algorithm / state | Seeded from | Users | State it can affect | Control | Tag |
|---|---|---|---|---|---|---|---|---|
| RN1 | 8040AB94 `srand`, 8040ABA0 `rand` (libc/rand) | MSL LCG | `s = s * 0x41C64E6D + 0x3039; return s & 0x7FFFFFFF`; state at `_impure_ptr->0x58`, shared by every caller | `srand(mftb * 12)` once per session (TB5). The strings `"use_ini_seed"` and `"random_seed_to_use"` sit in rodata at `lbl_805C571C` but **nothing in the DOL references them**: the INI-seed path is compiled out | **153 direct callers**: game 103, db 42, fe 7, ealib 1; plus 4 SDK CARD functions (RN4). Buckets: 0x80030000..0x80060000 (50 fns: unnamed classes; includes audio `fn_80040E9C`), 0x80060000 (9), 0x800D0000 (13: strings `"SOAKING %s MINIGAME GAME"`, `"BATTING"`, `"PITCHING"`), 0x80300000..0x80320000 (11: strings `"Parameters"`, `"Colours"`, I: effects), db overlay (42: franchise / season / roster code), frontend (7, e.g. create-a-player `script_createaplayer.txt`) | presentation and menus, and (I) possibly some gameplay variety. Whether any gameplay result reads it is not established | R3, R10 | V |
| RN2 | next: 8014D8AC; seed: 8014D860; reset: 8014D81C | EA PRNG object (6-word add-with-carry chain + index) | `struct { int idx; unsigned w[6]; }`. `fn_8014D8AC(rng, range)` advances the chain and returns `value % range` (stores it in `idx`; `range == 0` returns 0). `fn_8014D860(rng, seed)` fills `w[]` from `seed` with fixed offsets. `fn_8014D81C` copies the constant table `lbl_805C5F90` (6 words) | `mftb * 12` at game creation (TB6, TB7); re-seeded from the **stored** `game->m24` by `fn_801A0DE0` and `fn_801A41D0` (a replay / restart path); reset to seed 0 by `fn_8014C8BC`. Objects are default-constructed from the constant table (`fn_80069F0C`, `fn_805A07C4`) | **192 direct callers**: game 182, animation 8, db 2, concentrated at 0x80150000..0x80290000 | **the gameplay simulation** (pitching, batting, fielding, AI, animation) | R3 | V |
| RN3 | 80693C7C (db RNG object) | season-sim RNG | an RN2 object seeded in `fn_8056FE8C` from `rand() % 0x7FFFFFFF` | RN1 | db overlay (2 callers of RN2 there) | franchise / season simulation | R3 (indirectly) | V |
| RN4 | 8041C488 `VerifyID`*, 8041DE88 `__CARDFormatRegionAsync`*, 80420604 `DummyLen`, 804206C8 `__CARDUnlock` | SDK CARD | call libc `rand` (format IDs, unlock timing) | RN1 | card mount/format/unlock | shifts RN1 when the card code runs | R9: the engine's card must not consume `rand`, or must consume identically on both peers | V |
| RN5 | 8043DF38 `__VMPageReplacementRandom`* | SDK VM | page choice from `OSGetTick` | time | ARAM virtual memory | timing only | R10 (HLE VM) | V |

The two streams are used by disjoint code except one function (the intersection of
callers is 1). The simulation RNG is deterministic given one 32-bit seed per game, and
the game stores that seed (`m24`), which is what a replay or a handshake-agreed seed
can reuse. The libc stream is global and ordered by call order across the whole
program, which is why differences in presentation code are dangerous (§1.2 risk 5).

### 3.3 Threads

| ID | Where | Name | Priority / stack | What it does | State it can affect | Control | Tag |
|---|---|---|---|---|---|---|---|
| TH1 | (main) | main thread | 16 (SDK default, **I**) | runs `main`; handle saved by `fn_803FFE14` in `lbl_806EF47C` | everything | R5 | V |
| TH2 | `fn_8040015C` creates it, entry `fn_8040024C`, thread object `lbl_806B83A8`, queue `lbl_806B8308` (32 msgs) | EA tick thread | 4 / 0x1000 | loop: receive message; 0 -> `fn_803FF9D4` (tick), 1 -> `fn_80400EF8` (runs 8 callbacks `lbl_8062EE20`), 2 -> exit. Started by `fn_803FF888` | **tick counter** `lbl_806EEAE0` / `AE4`, 8 tick callbacks `lbl_8062EE40` | R5, R6 | V |
| TH3 | `fn_803FEE70` creates it (from `fn_803F9544`, file system init), entry `fn_803FF618`, object `lbl_806B5F20`, queue `lbl_806B8238` (32 msgs) | RealCore file I/O thread | 2 | receives requests (`fn_803FF714`), issues `DVDReadAsyncPrio`, waits for the completion callback message, polls `DVDGetDriveStatus` every 100 ms (`fn_803FFFA8(100)`), signals the requester (`fn_803FA888`). Also calls `PCinit`/`PCread`/`PClseek` (libsn host files, a dev-only path: mode 3 vs DVD mode 2 in `lbl_806ADEC0.m0`) | all asset loading | R7 | V |
| TH4 | `fn_803AF61C` (SND init) creates it, entry `fn_803AF238`, stack `lbl_806A7FE8`, size 0x8000 | EA SND audio thread | 1 | sleeps (`OSSleepThread`) until the AX AuxA callback wakes it; flushes callback data, runs the sound system | audio; **I**: no game-state feedback, but the game calls SND under `SNDSYS_entercritical` / `leavecritical` (16 game callers) | R8 | V |
| TH5 | `fn_8035A028` -> `fn_803FFE3C` (wrapper over `OSCreateThread` + `OSResumeThread`), entry `fn_8035A198`, name `"MC_ThreadInterfaceStack"` | worker thread class (memory card interface) | via the table in `fn_803FFD9C` (class 3..-3 -> OS 0, 5, 10, 16, 20, 25, 31) | body: `while (!work()) sleep(1 ms); run()` | memory-card operations, which run in EA's mcard layer (`ealib/tu_803F00FC`, `tu_803F2000`, `tu_803F73B0`) with the **synchronous** SDK CARD calls | R9, R5 | V creation; priority value not decoded |

`OSCreateThread` has exactly four call sites (`fn_803AF61C`, `fn_803FEE70`, `fn_803FFE3C`,
`fn_8040015C`). Other thread calls found game-side: `OSResumeThread` (4), `OSSleepThread`
(1, snd), `OSWakeupThread` (1, snd), `OSYieldThread` (2), `OSCancelThread` (2),
`OSSetThreadPriority` (1), `OSGetCurrentThread` (5), `OSIsThreadTerminated` (1),
`OSInitMutex` (4), `OSLockMutex` / `OSUnlockMutex` (10 each), `OSDisableInterrupts` (6 sites,
`snd/` and the GX breakpoint code), `OSRestoreInterrupts` (7), `OSEnableInterrupts` (2). **V**

### 3.4 Message queues

| ID | Object | Capacity | Producer | Consumer | Meaning | Tag |
|---|---|---|---|---|---|---|
| MQ1 | `lbl_80627390` | **1** (no-block send, so extra sends are dropped) | VI retrace callback `fn_803D9378`, GP-done callback `fn_803D8828` (vsync off) | main thread, `fn_803D8640` (blocking) | vsync | V |
| MQ2 | `lbl_806B8308` | 32 | alarm handler `fn_803FFA3C` via `fn_8040021C` | tick thread | 60 Hz tick | V |
| MQ3 | `lbl_806B8238` | 32 | requester `fn_803FF714`; DVD callback `fn_803FF5E8` | file thread | I/O requests and completions | V |
| MQ4 | per call, on the stack (`fn_803FFF30`) | 1 | one-shot alarm handler `fn_803FFF0C` | the sleeping thread | `SleepTicks` | V |

### 3.5 OS alarms, timers and the EA system tick

| ID | Where | What | State it can affect | Control | Tag |
|---|---|---|---|---|---|
| AL1 | `fn_803FF888(60)` (called at 8014C0A0); `OSSetPeriodicAlarm(lbl_806B82E0, OSGetTime(), period, fn_803FFA3C)` with `period = (busClock / 4) * (1 / rate)` = 675000 ticks | **60 Hz periodic alarm**. Handler `fn_803FFA3C` -> `fn_8040021C(0)` -> `OSSendMessage` -> TH2 -> `fn_803FF9D4`: `lbl_806EEAE0++`, `lbl_806EEAE4++`, run `lbl_8062EE40[0..7]`. Rate stored in `lbl_806EEADC` (default 100 if the argument is 0; boot passes 60). `fn_803FF994` cancels it (registered as shutdown callback via `fn_80400690`) | The tick counter read by: `fn_8014C8BC` (240-tick wait), `fn_8014BEB8`/`fn_80400E18` (timed boot wait), `fn_802D77F0` (waits 5 ticks in the pad code), UI timers (`fn_801A9584` blink timer = rate/10 ticks, `fn_801A7D3C`), loading screens (`fn_80315FFC`, `fn_80316310`, `fn_80316614`), frontend (`fn_804E1020`, `fn_804EB2C8`), db (`fn_80545BB8`), profiler | R5, R6 | V |
| AL2 | `fn_803FFF30` (SleepTicks), `fn_803FFFA8` (SleepMs = ms * busClock/4000) | stack-allocated alarm + queue per sleep; `ticks == 0` -> `OSYieldThread`. Users: file thread (100 ms drive poll), `fn_803F99E8` (10 ms when not on the main thread), MC worker (1 ms), memory-card UI (`game/tu_800057D8`), `fn_800C31FC` | when sleeping threads wake | R5, R6 | V |
| AL3 | `fn_80400C1C(cb, period, delay)` registers into a 16-entry table `lbl_806B96D8`; `fn_80400CE4(x)` runs every due entry | EA timer table, **run by whichever thread pumps it** (main thread once per frame at 80148C68, and inside wait loops). Registered: `fn_8000446C` by `fn_80316B90` (period 1 tick: reset switch, DVD cover/drive status, rumble stop), `fn_803AD994` by `fn_803AD9BC` (SND main-thread update) | reset / cover handling (inputs that never change in a clean run), sound-system housekeeping | R4, R12 | V |
| AL4 | SDK alarms: DVD (`stateReady`, `stateCoverClosed`, `cbForStateMotorStopped`, `Read`, `DVDLow*`), CARD (`Retry`, `__CARDStart`), SI (`SITransfer`), `OSInitAlarm` / decrementer | internal to the SDK | completion times of DVD, CARD, SI | R7, R9; moot if replaced by name | V |

### 3.6 Interrupt-context callbacks

| ID | Registered at | Callback | Context | What it does | State it can affect | Control | Tag |
|---|---|---|---|---|---|---|---|
| CB1 | `fn_803D8918` -> `VISetPostRetraceCallback` (8043BDE0); `fn_803D9AD0` (device destructor) calls `VISetPostRetraceCallback(0)`. `VISetPreRetraceCallback` is **not linked into the DOL** (no such function in the VI unit), so there is no pre-retrace callback | `fn_803D9378` | VI retrace interrupt | §2.5 | frame wait, fade-in black | R2 | V |
| CB2 | `fn_803D8918` -> `GXSetDrawSyncCallback` | `fn_803D9454` | GX CP interrupt | token 0x8000: clear `lbl_80627304`; tokens `0x4xxx`: `GXReadPixMetric` into `lbl_806E9938/EB938/ED938`, then `fn_803D9664` | GPU pixel counts; **no reader found** (I: occlusion / visibility queries) | R11 | V; reader I |
| CB3 | `fn_803D8918` -> `GXSetBreakPtCallback` | `fn_803D95C4` | GX CP interrupt | runs the next record of the deferred ring | FIFO pacing | R11 | V |
| CB4 | `fn_803D95FC(token, fn)` enqueues, e.g. `fn_803D8828` | ring `lbl_806273B4` | CP breakpoint | "GP reached this point" notifications | frame sync | R11 | V |
| CB5 | `fn_803AF61C` -> `AXRegisterAuxACallback` (8041818C) | `fn_803AED08` | AX frame callback (DSP / AI DMA interrupt; the AX frame is 5 ms, SDK knowledge) | copies sound data, `OSWakeupThread` on the audio thread | audio only (I) | R8 | V |
| CB6 | `AXInit` -> `__AXOutInit` -> `AIRegisterDMACallback` (80414F30) | `__AXOutAiCallback`* | AI DMA interrupt | SDK internal: AX output frame | audio | R8 | V |
| CB7 | `AXInit` -> `__AXOutInitDSP` -> `DSPAddTask` (80421D88) | AX DSP task | DSP interrupt | SDK internal | audio | R8 | V |
| CB8 | `fn_803AEB50` -> `ARQPostRequest` (80417554) | `fn_803AEB04` | ARAM DMA interrupt | sample transfers MEM1 <-> ARAM for the sound system (also `ARQRemoveRequest` in `fn_803AEC70`) | audio | R8 | V |
| CB9 | `fn_803FF424`, `fn_803FF244` -> `DVDReadAsyncPrio` (80422EE4), priority 2 | `fn_803FF5E8` | DVD interrupt | `OSSendMessage(MQ3, 1, BLOCK)` | load completion | R7 | V |
| CB10 | `lbl_8062EE40[8]` | EA tick callbacks | TH2 | run on every tick; the register function was not found (only `fn_803FF9D4` runs them and `fn_80400498` removes) | unknown | R5 | V run; registrations unknown |
| CB11 | `lbl_8062EB20[64]` | EA shutdown callbacks (`fn_80400690` add, `fn_804006FC` remove, `fn_8040061C` run in reverse) | at exit | e.g. `fn_803FF994` (cancel the alarm) | none | none | V |
| CB12 | `fn_8014E9E0` -> `OSSetErrorHandler` | crash dumper | exception | writes `"crash.txt"`, prints `"Exception:"`; the string path `C:/mvp2004/source/gc/exception/ddtrap.cpp` names the source file | none (fatal) | R13 | V |
| CB13 | `fn_8000E624` -> `VMSetLogStatsCallback` | `fn_8000E718` | VM | statistics for the HUD (`lbl_806164C4..D4`) | display | R10 | V |
| CB14 | SDK internal: `PADSetSamplingCallback`, `SIRegisterPollingHandler`, `EXISetExiCallback`, `GXSetDrawDoneCallback` (no game-side callers) | | | | | none | V |

### 3.7 Async completions and external inputs

| ID | Where | What | State it can affect | Control | Tag |
|---|---|---|---|---|---|
| AC1 | RealCore file layer: `fn_803FEB34` (request wrapper), `fn_803F99E8` (wait), `fn_803FF424`/`fn_803FF244` (chunked `DVDReadAsyncPrio`: 32-byte aligned head, body, tail; tail chunk up to 0x4000; request clamp 0x100000), `fn_803FF058` (`DVDConvertPathToEntrynum`, `DVDFastOpen`; also host-file paths) | every asset read: blocks the requester in a **polling loop** (`fn_803F99E8`: on the main thread it pumps `fn_80400CE4`, elsewhere sleeps 10 ms) | load duration, hence virtual time at the next `mftb` | R4, R7 | V |
| AC2 | `fn_803FF618` | after each request the file thread polls `DVDGetDriveStatus` until 0, 8 or 1, sleeping 100 ms between polls | load duration | R7 | V |
| AC3 | `fn_8014E614` (crash dump writer: `DVDReadAsyncPrio` + `OSYieldThread` + `DVDGetCommandBlockStatus`), `fn_8014E588` (`DVDOpen`/`DVDClose`) | crash-dump path only | none | none | V |
| AC4 | EA memory-card layer (`ealib/tu_803F00FC`, `tu_803F2000`, `tu_803F73B0`) | memory card: calls `CARDProbeEx`, `CARDMount`, `CARDCheck`, `CARDOpen`/`FastOpen`, `CARDCreate`, `CARDRead`, `CARDWrite`, `CARDDelete`/`FastDelete`, `CARDRename`, `CARDSetStatus`/`GetStatus`, `CARDFormat`, `CARDFreeBlocks`, `CARDGetSectorSize`, `CARDSetAttributes`, `CARDClose`, `CARDUnmount`, `CARDInit` (called from `fn_803F713C`). All **synchronous** variants; the `*Async` functions have no game-side callers | save files and card-dependent UI flows; consumes `rand` inside the SDK (RN4) | R9 | V |
| AC5 | `PADRead` (`fn_8000A44C`, boot `fn_8000D6D4`), `PADReset`, `PADRecalibrate`, `PADClampCircle`, `PADControlMotor` (rumble: `fn_8000AEB4`, `fn_8000446C`), `SISetSamplingRate(1)` (in `fn_80008540`) | the controller path; analog input is clamped by SDK `PADClampCircle`, which uses `frsqrte` | inputs, the netplay boundary | R14, R15 | V |
| AC6 | the pad manager `Unk802D73B4_Mgr` (`vtable lbl_806647E8`, ctor `fn_802D73E8` in `__13Unk802D73B4_W`) | each pad object keeps a **ring buffer of input records** (`m0` read index, `m4` write index, length `lbl_8061F674`). `s13` / `fn_802D8624` store the current pad state, `s14` / `s12` / `fn_802D890C` play it back. `s5(int*)` is a "ready to advance" query that always returns 1 in retail (called in a spin loop in `fn_80148698`) | inputs and replays; the shape a lockstep layer needs already exists | R14 | V code, I purpose |
| AC7 | libsn / ProDG host I/O: `PCinit` (called unconditionally by `fn_803FEE70`), `PCcreat`, `PCopen`, `PCread`, `PCwrite`, `PClseek`, `PCclose`, plus `EXI2*` stubs in `dolphin/amcstubs` | dev-kit file access over EXI2 | none in a clean run | R13: stub to fail | V |
| AC8 | GP: `GXDrawDone` (`fn_803D987C`), `GXAbortFrame`, `GXFlush`, `GXCopyDisp`, `GXCopyTex` (2 game callers), `GXReadPixMetric` | CPU waits for GPU idle; EFB/XFB copies write MEM1 | XFB and copied textures are GPU output | R11, R16 | V |
| AC9 | VM: `fn_8000E624` -> `VMInit(0x100000, 0x800000, 0x800000)` (I: 1 MB resident pages, 8 MB backing); `fn_8000E65C` -> `VMAlloc(0x7E000000, 0x800000)` then creates an EA heap in that region; `fn_8000E6F8` -> `VMStoreAllPages`. The address check `fn_80400E8C` accepts 0x7E000000..0x7FFFFFFF. SDK VM swaps pages with DSI exceptions and `ARStartDMA` | an 8 MB virtual region backed by ARAM | memory layout only (I): contents are the same, timing differs | R10 | V |
| AC10 | `fn_803D8918`, `fn_8000D6D4`, `fn_803134A0`, `fn_804EB2C8`, `fn_8000446C`, `fn_803F713C`, `fn_803EE81C`, `fn_803EE878`, `fn_803E0F9C` | environment queries: `VIGetTvFormat` (`fn_803D8918`), `VIGetDTVStatus` (3 sites), `OSGetProgressiveMode` / `OSSetProgressiveMode` (3 each), `OSGetResetCode` (3), `OSGetResetSwitchState` (`fn_8000446C`), `DVDGetDriveStatus` (2), `DVDGetCurrentDiskID` (`fn_803F713C`), `OSGetArenaLo` / `Hi`, `OSSetArenaLo` (`fn_803EE81C`, `fn_803EE878`: heap setup), `VIGetNextField` (`fn_803E0F9C`, viewport jitter for interlacing: display only) | menu options (progressive scan), video mode, memory layout | R12 | V |

### 3.8 Other sources and things to watch

| ID | Source | Why it matters | Control | Tag |
|---|---|---|---|---|
| OT1 | **Frame catch-up** (`n` ticks, render skip, forced `n = 3`) | §2.4. Pure function of the cycle stamps | R2, R4 | V |
| OT2 | **Spin waits that need an event from another thread**: `fn_803F99E8` (file wait), `fn_80400E18` (`WaitTicks`, called from boot), the 240-tick loop in `fn_8014C8BC` (8014C970), `fn_802D77F0` (waits 5 ticks while calling `fn_80400CE4`), `fn_80148698` (spins on `fn_802D7A84`, which returns 1), `fn_80400114` (polls `fn_804000C8` with 10 ms sleeps) | no instruction-count-free progress; need event delivery at loop back-edges | R4, R5 | V |
| OT3 | **Floating point**: 469 functions use `fctiw`; `frsp` in 1352 (almost all game/EA); 673 game/EA functions use `fdiv`; guest libm (`sinf` 60 callers, `cosf` 59, `atan2f` 120, `sqrtf` 33, `acosf`, `asinf`, `tanf`, `logf`, `exp`, `log`, `floor` 101) is retail code in `libm/`; 12 game/EA functions use paired-single quantized load/store (`psq_l`/`psq_st`); `PPCSetFpNonIEEEMode` (`mtfsb1`) is in the SDK | results must be bit-exact with the Gekko | R15 | V |
| OT4 | **Uninitialized memory and allocators**: `fn_80003100` clears `.bss`/`.sbss` only; EA heaps (guest code) and `OSAllocFromHeap` hand out memory whose old contents depend on history; heap addresses depend on `OSGetArenaHi` / `Lo` and load order | pointer values and reused memory must be a function of history, never of the host | R16 | I (nothing found that reads garbage) |
| OT5 | **PAD analog clamp** (`PADClampCircle`, `frsqrte`) | SDK function in the input path | R15 | V |
| OT6 | `lbl_8061B994` (seed copy), `lbl_8061B8E8` / `8EC` (late-tick counters), HUD counters | written, not read by sim code (**I**: only reads found are HUD) | none | I |
| OT7 | The reset switch and disc cover (`fn_8000446C` every tick) | an input the engine must keep at "not pressed" | R12 | V |
| OT8 | `PCinit`-style dev paths, `crash.txt` | never in a clean run | R13 | V |

### 3.9 Count of sources by kind

| Kind | Rows |
|---|---:|
| time-base read (TB1-TB15) | 15 |
| RNG (RN1-RN5) | 5 |
| thread (TH1-TH5) | 5 |
| callback / alarm / interrupt (AL1-AL4, CB1-CB14) | 18 |
| async completion / external input / message queue (AC1-AC10, MQ1-MQ4) | 14 |
| other (OT1-OT8) | 8 |
| **Total** | **65** |

## 4. SDK machinery behind the callbacks (for the HLE)

If the engine replaces SDK functions by name (ARCHITECTURE §5), none of this runs, but
the HLE must keep its observable behaviour. SDK functions that game or EA code calls:
293 by my count of direct calls (`targets.py --list K1` counts 299 by its own rule). The
determinism-relevant subset and what the game depends on:

| Area | Game-side use | Observable behaviour the HLE must keep |
|---|---|---|
| OS time, alarms | `OSGetTime`, `OSCreateAlarm`, `OSSetAlarm`, `OSSetPeriodicAlarm`, `OSCancelAlarm` | alarms fire in virtual time, handler runs in interrupt context, 64-bit period |
| OS threads / queues / mutexes | see §3.3 | strict priority, run until block, preemption by a higher-priority wakeup at the next safe point |
| DVD | `DVDReadAsyncPrio`, `DVDOpen`, `DVDFastOpen`, `DVDClose`, `DVDConvertPathToEntrynum`, `DVDChangeDir`, `DVDGetDriveStatus`, `DVDGetCommandBlockStatus`, `DVDGetCurrentDiskID`, `DVDInit` | completion after a deterministic delay, callback in interrupt context, status codes 0/1/8 |
| PAD / SI | `PADInit`, `PADRead`, `PADReset`, `PADRecalibrate`, `PADClampCircle`, `PADControlMotor`, `SISetSamplingRate` | status array per read |
| VI | `VIInit`, `VIConfigure`, `VIFlush`, `VISetNextFrameBuffer`, `VISetBlack`, `VIWaitForRetrace`, `VISetPostRetraceCallback`, `VIGetTvFormat`, `VIGetDTVStatus`, `VIGetNextField` | retrace cadence and callback |
| GX | 99 functions; sync / callback set: `GXSetDrawSync`, `GXSetDrawSyncCallback`, `GXSetBreakPtCallback`, `GXEnableBreakPt`, `GXDisableBreakPt`, `GXDrawDone`, `GXFlush`, `GXCopyDisp`, `GXCopyTex`, `GXAbortFrame`, `GXInit`, `GXReadPixMetric`, `GXRedirectWriteGatherPipe` | break points and draw-sync tokens fire when the command stream reaches them |
| AX / AI / ARQ / DSP | `AIInit`, `AXInit`, `AXQuit`, voice calls, `AXRegisterAuxACallback`, `ARInit`, `ARQInit`, `ARQPostRequest`, `ARQRemoveRequest`, `ARGetBaseAddress` | 5 ms AX frame callback, ARQ completion callback |
| CARD | 21 synchronous functions (AC4) | results independent of the host |
| VM | `VMInit`, `VMAlloc`, `VMStoreAllPages`, `VMSetLogStatsCallback` | none beyond returning the region |
| libc / libm | `rand`, `srand`, `qsort` (32 callers), `bsearch` (2), `strtok`, `sprintf` (379), math | `qsort` is guest code (deterministic if the comparator is) |

## 5. Frame and lockstep model implied by the code

Putting §2 together for the engine:

1. The sync unit is **one pass of `fn_80148930`'s loop**: one `PADRead` per pad, then
   `n` logic ticks (1..3), then maybe a render, then the vsync wait.
2. `n` is a function of the cycle stamps. With a deterministic virtual clock it is
   deterministic, and it will be 1 on almost every pass with an occasional 2 (the
   0.1% drift). The engine should log `(frame index, n, accum)` per pass and put them in
   the desync check, so a divergence is found at the frame where it starts.
3. Everything between two `PADRead` calls must be a function of the previous state,
   the inputs and the virtual clock only. That includes the timing of the DVD, alarm,
   VI, GX and AX events (R2..R8).
4. Lockstep from power-on (ARCHITECTURE §8) covers the boot, the first-session path
   (4 s of EA ticks, splash screens, video) and all menus. The only things that
   differ per run in retail are the time-seeded values above, so the engine's virtual
   time makes a run reproducible, and a handshake seed is optional (R3).

## 6. Engine requirements

Numbered for E0b (boot spike) and E4 (determinism). "Verify" says what to check.

- **R1. Virtual time base.** `mftb`, `OSGetTime`, `OSGetTick`, `__OSGetSystemTime`
  return a value derived only from emulated state, never from the host clock.
  Frequencies: time base 40,500,000 Hz (bus 162,000,000 / 4), CPU 486,000,000 Hz
  (`mftb * 12` equals CPU cycles). Write 162,000,000 and 486,000,000 to 0x800000F8 and
  0x800000FC, as the OS does. Advance time by (a) a fixed charge per guest instruction
  (any constant, the same on all peers) and (b) jumps to the next scheduled event
  whenever no thread can run. Verify: log every `mftb` with its caller (the 18 readers
  and the call sites in §3.1 must be the only ones).
- **R2. Retrace schedule.** VI retraces fire at fixed virtual times: every 675,675
  ticks for 480i NTSC (59.94 Hz), 675,000 for 480p, with the mode taken from the game's
  `VIConfigure` call. The VI post-retrace callback runs as an interrupt at the next
  safe point after the retrace time. Choose the per-instruction charge so a normal
  gameplay frame finishes well inside one retrace; otherwise `n = 2` appears (still
  deterministic, but it changes the sim step and the RNG call pattern).
- **R3. Seed policy.** Because the two RNG seeds come from `mftb * 12` (TB5..TB7),
  reproducibility needs R1. Provide a second, explicit control: the engine may force
  `mftb` at the exact instructions listed in TB5/TB6/TB7 (8014C91C, 8014CC8C, 8014CCF8,
  8014CD84, 8014CE0C, 8014CE94, 8014CEDC, 8014CF24, 8014D00C, 8017ACAC) to a seed agreed
  in the handshake. That makes netplay independent of load-time differences
  between peers (including mod-pack file sizes) and lets a replay start from a recorded
  seed. Record the seed in the replay file. The same applies to TB9..TB11 (80047724,
  8004774C, 8005B658, 8005B6A4, 804EB544): they must also see the same value on both
  peers.
- **R4. Safe points.** Deliver interrupts, alarms and thread switches at function
  entries **and loop back-edges**, counted in guest instructions (never wall time). The
  game has spin loops that make no progress without another thread or an alarm (OT2).
  Honour `OSDisableInterrupts` / `OSRestoreInterrupts`: defer delivery while
  disabled. Optional speed-up: replace the known wait functions by name (`fn_80400E18`
  WaitTicks, `fn_803F99E8` FileWait, `fn_803FFFA8` SleepMs) with "block the thread
  and advance virtual time to its wake-up", which is equivalent and avoids
  burning instructions.
- **R5. Deterministic scheduler.** Dolphin OS semantics: strict priority (lower number
  runs first), a thread runs until it blocks, yields or a higher-priority thread
  becomes ready, and a thread made ready by an interrupt preempts at that interrupt's
  return. Implement `OSCreateThread`, `OSResumeThread`, `OSSleepThread`, `OSWakeupThread`,
  `OSYieldThread`, `OSCancelThread`, `OSSetThreadPriority`, `OSIsThreadTerminated`,
  `OSGetCurrentThread`, message queues (`OSInitMessageQueue`, `OSSendMessage`,
  `OSReceiveMessage` with the BLOCK flag) and mutexes. There are five guest threads
  (§3.3): main 16, tick 4, file I/O 2, audio 1, memory-card worker. Verify that the
  tick thread preempts the main thread inside the spin waits of OT2.
- **R6. EA 60 Hz tick.** Honour `OSSetPeriodicAlarm`: first fire at the start time
  passed in, then every `period` ticks (675,000 for 60 Hz), handler in interrupt
  context. `fn_803FFA3C` posts to queue MQ2; the tick thread increments the counter.
  The counter value the main thread sees at a given instruction must be the same on
  both peers. Verify: record `lbl_806EEAE0` at each `PADRead`.
- **R7. DVD.** `DVDReadAsyncPrio` completes after a deterministic virtual delay (a
  function of start offset, length and the previous read position, for example a
  fixed latency plus bytes per tick), then runs the callback as an interrupt;
  `DVDGetDriveStatus` returns 0 (idle) when no command is pending and 1 (busy)
  while one is (the file thread stops polling on 0, 1 or 8 and sleeps 100 ms on any
  other state, so only error states make it loop). Both peers must
  read identical bytes (disc hash plus ordered mod overlay hash in the handshake).
  The unaligned head/tail chunking in `fn_803FF424` / `fn_803FF244` must see the same
  completion order as retail. `PC*` (libsn) functions fail immediately.
- **R8. Audio.** AX frame callbacks (`AXRegisterAuxACallback`, `AIRegisterDMACallback`,
  `DSPAddTask`) run every 5 ms of virtual time, the ARQ completion callback after a
  deterministic delay, and the audio thread (priority 1) wakes through `OSWakeupThread`.
  Output may be dropped or time-stretched but must not change the virtual-time events
  the game sees. Verify (E4): count `rand` / ADC calls per frame with audio on and off;
  they must match.
- **R9. Memory card.** In netplay present an empty card or a card synchronized at
  handshake. Synchronous `CARD*` calls return results that are a pure function of that
  card image. The HLE must not consume `rand` (RN4); document that this changes the
  libc stream relative to retail, which is fine as long as both peers use the engine.
  The card worker thread (TH5) and its 1 ms polling sleeps must be scheduled by R5.
  (2026-10-20: the engine emulates the card itself, not CARD HLE, so the SDK's
  `rand` calls (RN4) happen as on retail when a card is in. Online games have no
  card, the "empty" case. Offline, a run with a card differs from one without from
  the first mount on; test runs have none unless asked: `docs/engine/WEB.md`.)
- **R10. VM.** HLE `VMInit`, `VMAlloc`, `VMStoreAllPages`, `VMSetLogStatsCallback`:
  map 0x7E000000..0x7E7FFFFF (8 MB) as plain RAM; no DSI handling. Keep
  `fn_80400E8C`'s accepted ranges (0x7E000000..0x7FFFFFFF) valid.
- **R11. GX synchronization.** Process the command stream deterministically relative
  to the CPU: `GXSetDrawSync` tokens, FIFO breakpoints (`GXEnableBreakPt`) and
  `GXDrawDone` complete at defined points, and the callbacks `fn_803D9454`,
  `fn_803D95C4`, `fn_803D8828` (via the ring) run as interrupts at the next safe point
  after that. `GXReadPixMetric` returns fixed values (zeros) unless a consumer is
  found. Exclude the XFB (`dev+0xB8`), `GXCopyTex` destinations and
  `lbl_806E9938`/`EB938`/`ED938` from the memory hash (R16).
- **R12. Environment is configuration, fixed per match.** `VIGetTvFormat`,
  `VIGetDTVStatus`, `OSGetProgressiveMode` / `OSSetProgressiveMode`, `OSGetResetCode`,
  `OSGetResetSwitchState` (never pressed), `DVDGetDriveStatus` (never "cover open"),
  `DVDGetCurrentDiskID`, `OSGetArenaLo` / `OSGetArenaHi`, console type and memory size
  are constants chosen at the handshake. Boot-info pointer 0x800000F4 stays null (no
  `argc` / `argv`), as on a retail disc boot. Progressive scan changes the retrace period
  (R2), so it is part of the handshake.
- **R13. Dev-only and fatal paths.** `PCinit` and friends fail; `EXI2*` do nothing;
  an `OSSetErrorHandler` exception ends the match with a diagnostic.
- **R14. Input.** Inject controller state at `PADRead`, keyed by the frame index (the
  pass number of `fn_80148930`'s loop). Player 1 drives port 1, player 2 port 2, both
  peers use the same delay. `PADRead` is called once per pass and its result is shared
  by all `n` ticks in that pass. Alternative hook: patch `Unk802D73B4_Mgr::s5`
  (802D8B5C, 8 bytes) so it returns 0 until the inputs for the next frame have arrived
  and writes the number of ticks to run into `*x`; `fn_80148698` already loops on it.
  The pad manager's own input ring (AC6) can be reused for input delay on the guest
  side if wanted. Provide `PADStatus` error codes, and make `PADClampCircle`
  bit-exact (R15).
- **R15. Exact Gekko arithmetic.** Single-precision rounding after every single
  op (`frsp`, `fadds` and friends), fused multiply-add for `fmadd`/`fmsub`, `fctiw` /
  `fctiwz` saturation and out-of-range results, NaN and denormal behaviour with
  `FPSCR[NI]` as set by `PPCSetFpNonIEEEMode`, `psq_l` / `psq_st` with GQR scale and
  type, and the **`frsqrte` estimate table** (needed by `PADClampCircle` and
  `GXInitFogAdjTable`). Never call the host's `sin`/`cos`/`atan2`: the game's own `libm/`
  code must run on exact arithmetic. Verify (E4): a replay hash is identical across
  Chrome, Firefox and the native build.
- **R16. Memory image.** Initialize MEM1, ARAM and stacks to a fixed pattern (zero) at
  power-on, keep the arena addresses fixed (R12), and keep the allocator state a
  function of history. The memory hash of E4 should skip regions the GPU or the audio
  thread write (R8, R11).
- **R17. Diagnostics for E0b / E4.** Log for each frame: index, `n`, `accum`, EA tick
  value, `rand` call count, ADC call count and the time base value at `PADRead`. Also
  log (caller address, phase) for every `rand` and ADC call, to see whether any is
  reached from the render phase or an interrupt (static analysis cannot, because of
  virtual dispatch).

## 7. Proposed names

Evidence, not renames: the project rule is to keep `fn_` until evidence supports a name,
and the last column is that evidence. The rows above the "Applied" block are proposals and were
not applied to `symbols.txt`; the rows marked **(named)** at the end of the table were applied by
names pass 01 (2026-10-14, `tools/goals/renames/names01.tsv`) and supersede the proposal for the
same address. Where the two differ the applied name is EA's own (tier E), the string's (S) or the
C runtime's (K).

| Address | Placeholder | Proposed name | Evidence |
|---|---|---|---|
| 8014D63C | `fn_8014D63C` | `main` | called from `__start` (8000324C); calls `__main` |
| 8014BEB8 | `fn_8014BEB8` | `InitSystems` | `OSInit`/`DVDInit`/`VIInit`/`PADInit`; prints `"Init complete!"` |
| 8014C5A4 | `fn_8014C5A4` | `InitGame` | prints `"Entering game loop."` |
| 8014C8BC | `fn_8014C8BC` | `BeginSession` | prints `"Initialize new game"`; `srand` |
| 80148930 | `fn_80148930` | `RunGameLoop` | prints `"top of game loop!"`, `"after init!"` |
| 80148AB4 | `fn_80148AB4` | `GameLoopFrame` | §2.4 |
| 80148698 | `fn_80148698` | `GameLoopTick` | §2.4 |
| 8014D1EC | `fn_8014D1EC` | `StartGame` | calls LoadGameAssets and `fn_8014CBE8` |
| 8014CBE8 | `fn_8014CBE8` | `CreateGameByType` | the 8 game-mode strings |
| 801498C0 | `fn_801498C0` | `LoadGameAssets` | its body prints `"end of LoadGameAssets"` |
| 8014D488 | `fn_8014D488` | `EndGame` | deletes the objects `StartGame` created, then cleanup calls |
| 8014D5C0 | `fn_8014D5C0` | `RestartGame` | per-player reset, loop result 2/3 |
| 8014849C | `fn_8014849C` | `GetCpuCycles` | `mftb * 12` = 486 MHz cycles |
| 801484A8 / 801484E4 / 80148520 | | `StampFrameStart` / `StampFrameEnd` / `FrameTimeInSixtieths` | §2.4 |
| 80148580 | `fn_80148580` | `InitFrameTimer` | writes cycles per 1/60 s |
| 8014D8AC | `fn_8014D8AC` | `RngNext` | 192 callers, 6-word add-with-carry chain, `% range` |
| 8014D860 | `fn_8014D860` | `RngSeed` | fills the 6 words from one seed |
| 8014D81C | `fn_8014D81C` | `RngReset` | copies `lbl_805C5F90` |
| 803FF888 | `fn_803FF888` | `EaTimerInit` | prints `"UPDATEhz = %d"`; `OSSetPeriodicAlarm` |
| 803FF9D4 | `fn_803FF9D4` | `EaTickStep` | increments `lbl_806EEAE0` / `AE4`, runs `lbl_8062EE40[]` |
| 803FFA3C | `fn_803FFA3C` | `EaTickAlarmHandler` | posts message 0 |
| 8040015C | `fn_8040015C` | `EaTickThreadStart` | `OSCreateThread` prio 4, stack 0x1000, queue 32 |
| 8040024C | `fn_8040024C` | `EaTickThreadMain` | message loop |
| 80400DA4 / 80400E84 | | `EaGetTick` / `EaGetTickRate` | return `lbl_806EEAE0` / `lbl_806EEADC` |
| 80400C1C / 80400CE4 | | `EaAddTimer` / `EaPumpTimers` | 16-entry table `lbl_806B96D8` |
| 803FFFA8 / 803FFF30 / 80400E18 | | `EaSleepMs` / `EaSleepTicks` / `EaWaitTicks` | alarm + queue; tick spin |
| 803FF618 | `fn_803FF618` | `FileThreadMain` | request loop, `DVDReadAsyncPrio` |
| 803FF5E8 | `fn_803FF5E8` | `FileDvdCallback` | `OSSendMessage(queue, 1, BLOCK)` |
| 803FEE70 | `fn_803FEE70` | `FileThreadStart` | `OSCreateThread` prio 2 |
| 803F99E8 | `fn_803F99E8` | `FileWait` | polling loop with `EaPumpTimers` |
| 803D8918 | `fn_803D8918` | `GfxInit` | `VIConfigure`, `GXInit`, strings `"VI_NTSC\n"` etc. |
| 803D8640 | `fn_803D8640` | `GfxEndFrame` | vsync `OSReceiveMessage`, `GXCopyDisp` |
| 803D9378 | `fn_803D9378` | `GfxRetraceCallback` | `VISetPostRetraceCallback` |
| 803D8828 | `fn_803D8828` | `GfxGpDoneCallback` | queued through `fn_803D95FC` |
| 803D9454 / 803D95C4 | | `GfxDrawSyncCallback` / `GfxBreakPtCallback` | registered with the GX callback setters |
| 803D982C | `fn_803D982C` | `GfxSetVSync` | writes `dev->mDC` and `lbl_806272E8` |
| 803AF61C | `fn_803AF61C` | `SndInit` | `AIInit`, `AXInit`, `ARInit`, `ARQInit`, thread, AuxA callback |
| 803AF238 | `fn_803AF238` | `SndThreadMain` | `OSSleepThread` loop |
| 803AED08 | `fn_803AED08` | `SndAuxACallback` | `OSWakeupThread` |
| 8000446C | `fn_8000446C` | `PollResetAndCover` | `OSGetResetSwitchState`, `DVDGetDriveStatus`, `PADControlMotor`; registered as an EA timer by `fn_80316B90` |
| 8035A028 / 8035A198 | | `McThreadCreate` / `McThreadMain` | string `"MC_ThreadInterfaceStack"` |
| 8000E624 / 8000E65C | | `VmSetup` / `GetVmHeap` | `VMInit`, `VMAlloc(0x7E000000, ..)` |
| 800B6D64 | `fn_800B6D64` | `cFEManager::Init` | the `"cFEManager::Init ..."` strings |
| 8014F05C | `fn_8014F05C` | `CrashHandler` | `"crash.txt"`, `"Exception:"`, `ddtrap.cpp` path |
| 80316614 | `fn_80316614` | `DrawLoadingProgress` | its format string |
| 802D7588 / 802D75D4 | | `PadPoll` / `PadTickUpdate` | pad manager (`"Init cPad object"`) |
| 80438628 / 80438640 / 80438648 | | `OSGetTime` / `OSGetTick` / `__OSGetSystemTime` | already named in the SDK (ours) |
| **Applied** | | names pass 01 | |
| 8014D63C | `fn_8014D63C` | `main` **(named)** | K: crt0 `__start` calls it with argc/argv; the ProDG g++ inserts `__main()` only into `main`, and with that name the function matches 100% |
| 801498C0 | `fn_801498C0` | `LoadGameAssets` **(named)** | S: its own trace `"end of LoadGameAssets"` |
| 80316614 | `fn_80316614` | `DrawLoadingProgress` **(named)** | S: its own format string |
| 803FF888 | `fn_803FF888` | `TIMER_init` **(named)**, replaces `EaTimerInit` | E: EA `TIMER_init` in 8 builds |
| 803FF850 / 803FF994 | | `InitAlarm` / `TIMER_restore` **(named)** | E: EA names (inittmr.cpp) |
| 803FF9D4 | `fn_803FF9D4` | `ttDoTimerMsg` **(named)**, replaces `EaTickStep` | E: EA name (hp, nfsmw) |
| 8040015C | `fn_8040015C` | `ttInit` **(named)**, replaces `EaTickThreadStart` | E: EA `ttInit` in 8 builds (timerthread.cpp) |
| 8040024C | `fn_8040024C` | `TimerThreadFunc` **(named)**, replaces `EaTickThreadMain` | E: EA name in 6 builds |
| 80400DAC | `fn_80400DAC` | `TIMER_elapsedticks` **(named)** | E: EA name (timer.cpp); the caller divides it by the rate to get ms |
| 803FFFA8 / 803FFF30 | | `THREAD_yield` / `THREAD_yield64` **(named)**, replace `EaSleepMs` / `EaSleepTicks` | E: EA names (threads.cpp) |
| 803FFE3C | `fn_803FFE3C` | `THREAD_createparam` **(named)** | E: EA name in the fifa05/uefa maps |
| 80400690 | `fn_80400690` | `SYSTEM_addexit` **(named)** | E: EA name (hp, nfsmw) |
| 803FF618 | `fn_803FF618` | `ReadFileThreadFunc` **(named)**, replaces `FileThreadMain` | E: EA name (ge, moh3, moh4) |
| 803FEE70 | `fn_803FEE70` | `initfiledev` **(named)**, replaces `FileThreadStart` | E: EA name (filedev.cpp) |
| 803F99E8 | `fn_803F99E8` | `FILESYS_waitop` **(named)**, replaces `FileWait` | E: EA name (filesys.cpp) |
| 803F9494 / 803FACFC / 803FAD60 | | `FILESYS_init` / `FILESYS_getopts` / `FILESYS_setopts` **(named)** | E: EA names |
| 803FF244 / 803FF424 | | `AyncDVDRead` / `StartNonAlignedAyncRead` **(named)** | E: EA names (sic) |
| 803AF61C | `fn_803AF61C` | `SNDPLATFORM_init` **(named)**, replaces `SndInit` | E: EA name (5 builds) |
| 803AF238 | `fn_803AF238` | `SNDDRV_thread` **(named)**, replaces `SndThreadMain` | E: EA name (6 builds) |
| 803AED08 | `fn_803AED08` | `SNDDRV_audiocallback` **(named)**, replaces `SndAuxACallback` | E: EA name (5 builds) |
| 803AEB04 / 803AEB50 | | `SNDDRV_dmcallback` / `SNDDRV_dmapost` **(named)** | E: ARQ DMA callback and post; EA names in 9 builds |
| 80394DE8 / 80394F24 / 80394FD4 | | `iSPCH_EACseedrandom` / `iSPCH_InitRandom` / `iSPCH_Rand` **(named)** | E: the speech pick RNG (K3); EA names (nascar, nfsmw, fifa05, uefa) |
| 8039582C | `fn_8039582C` | `SPCHEXT_gettick` **(named)** | E: the speech time source; EA name in fifa05, nascar, nfsmw, uefa |
| 80438724 | `fn_80438724` | `__init_cpp` **(named)** | E and K: EA name, mitsevox names the address too |
| 802BB29C / 802BB334 | | `PrepareForDataUsage__FP15Unk802BB1E4_ObjT0` / `RestoreOverlay__FP15Unk802BB1E4_ObjT0` **(named)** | S: their own assert strings; overlay manager |

Not named, with reasons (names pass 01):

- **EA methods whose class is not in our source.** The reference builds give EA's names, but a
  correct g++ 2.95 name needs the class, which means restructuring: `fn_803D8918` is
  `EAGL::RenderContext::SetupFrameBuffers` (E, and confirmed by its own strings
  `"RenderContext::SetupFrameBuffers() - Could not allocate ..."`, S), so the table's `GfxInit` is
  that function; `fn_803D8640` is `EAGL::RenderContext::EndFrame` (`GfxEndFrame`); `fn_803D9378`
  is `EAGLInternal::RenderContextPrivate::VSyncCallback(unsigned long)` (`GfxRetraceCallback`);
  `fn_803D8828` is `...::BreakPtCallback(unsigned)` (`GfxGpDoneCallback`); `fn_803D9454` is
  `...::ReceiveToken`; `fn_803D95C4` / `fn_803D95FC` are `ProcessGPBreakPoint` / `SetGPBreakPoint`;
  `fn_803E2D18` etc. are `EAGL::Profiler::*` and `EAGL::ProfilerRegion::*`; `fn_8038EA9C` is
  `RCMP::DECODER::GetChunk`; `fn_8038DF3C` and `fn_8038E468` are `RCMP::AV_PLAYER::SetSpeed` and
  `RCMP::AV_MS_TIMER::Update`; `fn_800B6D64` is `cFEManager::Init` (S, its strings).
- **Linkage clash.** `fn_803AD2F0` (`SNDI_parsetimbre`) and `fn_803AFD44`
  (`SNDPLATFORM_getvoicerange`): `src/snd/cmn/sndcmn.h` already declares those names with C++
  linkage while the definitions here are `extern "C"`.
- **No reference name.** `fn_80400DA4` / `fn_80400E84` / `fn_80400E18` / `fn_80400DE4` (tick, rate,
  wait, set deadline), `fn_80400C1C` / `fn_80400CE4`, `fn_803774A8` (`operator new[]`), and the
  tier I rows above (`InitSystems`, `BeginSession`, `RunGameLoop`, `StartGame`, `RngNext`, ...).
- **Held by K1b.** `fn_8043188C` (`OSInit`), `fn_80422FD8` (`DVDInit`), `fn_80438704`
  (`__init_user`): mitsevox names them, but dtk then renames about 150 more symbols (checked again
  in pass 01 with `rename_symbols.py --from-dtk --dry-run`).

Names pass 02 (`work/names-02`, `tools/goals/renames/names02.tsv`): 278 more tier E names, 150 in
`snd/` (the AEMS, stream, system, mixer, filter and utility code) and 128 in `ealib/` (the RealGraph
SHAPE/FONT library, the RealMem manager, the async file and STREAM code, small math, locale and
EAGL helpers). Each one is a free function whose reference name is the same in every build that has
it, or the clear majority (six ties are recorded in the tsv). Not named, with reasons (names pass 02):

- **Methods and namespace functions whose class is not in our source (486): 437 in `ealib/`, 8
  in `ealibs/`, 40 in `snd/`, 1 in `gamelib/`.** By family: `EAGL::*` 196 (Transform, DynamicLoader,
  ViewPort, Device, RenderContext*, TAR*, Model, DrawArray, ...), `EAGLAnim::*` 112 (the animation
  channels and blenders in `tu_803BA9DC` to `tu_803D2B20`), `Realmc::*` 92 (the RealCore GameCube
  layer: `GCInterface` 38, `GCDriver` 31, ...), `Snd::*` 31 (`getbits`, `discardbits`,
  `readsamples`, `interpolate`, `reftolpc`, `filter`, `decodexac`, `decodemut`,
  `process_raw_block` and the `CMTBLKDecf` / `CEAXABLKDecf` decoders in `snd/tu_803B7DA0`, plus
  `Snd::System`, `Snd::Memory` and `Snd::Util`, which `include/snd/sndo.h` and `src/snd/cmn/sndcmn.h` declare, but our
  definitions are `extern "C"` free functions with guessed prototypes), `RCMP::*` 21, `Csis::*` 9
  and 25 more (GCHW_VD, VP6_/MAD_CODEC_INTERNAL, MyAllocator, ...). A correct g++ 2.95 name needs
  the class or namespace and the real parameter list.
- **Header clashes (19), all in `snd/`.** The name is already declared in `src/snd/cmn/sndcmn.h`
  or `include/snd/sndo.h` with a different prototype (void vs int return, `void *` vs a struct
  pointer, `int` vs `Snd::Channel`) or with C++ linkage, so the compiler rejects the rename: `SNDbankadd`
  (`fn_803A82A8`), `SNDAEMS_addmodulebank` (`fn_803A6B64`), `SNDstop` (`fn_803AB480`),
  `SNDSTRM_destroy` (`fn_803AA95C`), `SNDplaysetdef` (`fn_803A9204`), `SNDSTRM_pitchmult`
  (`fn_803AB56C`), `SNDSTRM_setvol` (`fn_803AB9F0`), `SNDSTRM_setazimuth` (`fn_803AAB7C`),
  `SNDI_parsetimbre` (`fn_803AD2F0`), `SNDPLATFORM_getvoicerange` (`fn_803AFD44`),
  `SNDPLATFORM_memalloc` / `memfree` / `download` / `downloadcomplete` / `packetplaycreate` /
  `packetplay` / `packetplaydestroy` (`fn_803B1118`, `fn_803B115C`, `fn_803B11A8`, `fn_803B11FC`,
  `fn_803B0368`, `fn_803B042C`, `fn_803B0714`), `SNDfxlevel` (`fn_803B3EA8`) and
  `SNDI_validrendermode` (`fn_803B58E8`). Fixing them means changing prototypes or linkage in
  every caller (a contract commit), not renaming.
- **Hand-written static-init emulations (9).** `fn_803BD800`, `fn_803DD4E4`, `fn_803DE0F0`,
  `fn_803E1B54`, `fn_803E2E18`, `fn_803E3A74`, `fn_803E9594`, `fn_803EF23C`, `fn_803FADD0` are
  `__static_initialization_and_destruction_0` in the reference builds. That is a compiler-generated
  name (36 `scope:local` copies of it already exist in `symbols.txt`), and in our sources these
  are ordinary `extern "C"` functions that a TU with its own static constructors would collide with.
- **Ambiguous names, left alone (5).** `fn_803BD358` (`Multiply` in ge and moh4, `MtxMult` in moh3
  and nfsu: a 2-2 tie), `fn_80402FCC` (`RealmcUtils::Unicode2Ascii` in hp, `USTR_strcpyW` in nfsu),
  `fn_803F6754` (`Realmc::GetVersionNumber` in ge, hp and nfsmw, `GetVersionNumber` in moh4),
  `fn_803EFE60` (`cGroperBindGroup::Init` in hp, `Realmc::GCMessage::Clear` in moh3, moh4 and nfsu),
  `fn_803E6FB8` (`GaugesMgr_BeginShareDraft` in nascar, `EAGLInternal::DefaultMalloc` in nfsu). The
  Freedom Fighters, Goldeneye and Medal of Honor game-class names that show up as a lone rival
  (`ZCOUNTER::GetNrCounters`, `ZDllSound::SetMasterVolume`, `hud::HudTimer`, ...) are look-alikes
  and were ignored when the other builds agree.
- **Overload and single-build outliers (2).** `fn_803EC568` is EA's overload
  `SHAPE_size(int, int, int, unsigned int, bool)` (`SHAPE_size__FiiiUib`); `fn_803EC6F8`, the
  unmangled one, took the plain name, and C linkage cannot hold two, so the overload keeps its
  placeholder until its prototype is settled. `fn_803DF7A0` (`mload44`, Freedom Fighters only) has
  no family around it in the EAGL transform code. Two statics with the same base name in different
  units (`locaterequest` in asyncfile and in stream) took their mangled names
  (`locaterequest__Fi`, `locaterequest__FP15STREAMHEADERtagi`).
- **Game code (107 hits, none applied).** Every hit outside the library units is a look-alike
  (`GuiGetChampionsLeagueMatchType`, `RangedRandInt`, `LUAShutdown`, `bServiceFileSystem`,
  `DVDValidErrorState`, ...); no second reason found for any.
- **SDK units (273 hits: `dolphin/*` 237, `sdk/*` 24, `libgcc` 7, `libsn` 5).** Not in scope for
  this pass; they belong to K1b because dtk renames more symbols once these are named.

## 8. Open points for E0b / E4 to settle

1. Who calls `Present` (`fn_803D8640`) in gameplay, and whether the render phase or any
   interrupt-reachable code calls `rand` or the ADC generator (R17 logging answers it).
2. The readers of `fn_8017AD8C`'s timestamp (`obj+0x408`) and of the GP pixel-metric
   arrays (CB2). If gameplay reads either, R11 / R1 get stricter.
3. The meaning of `self->m24`, `m28`, `m2C` in `fn_80148AB4`'s render condition. They
   are set outside the files read for this document.
4. Whether game logic reads audio state (SND is called under critical sections from 16
   game functions; the entry points that return state were not decoded). R8 assumes it
   might.
5. The registrations of the 8 tick callbacks (`lbl_8062EE40`; `fn_8014865C` is removed from
   it at the end of `fn_80148930` but nothing adds it) and of the 8 callbacks run by message 1
   (`lbl_8062EE20`).
6. The real priority of the memory-card worker (`fn_803FFD9C` table) and whether more
   than one instance is created.
7. `TB11` (random value 19..56 in the frontend): what it selects.
