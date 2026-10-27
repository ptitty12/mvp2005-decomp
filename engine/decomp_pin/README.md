# engine/decomp_pin/

The engine's own copy of three files the decompilation keeps in `config/GV4E69/`.
The engine reads them from here and nowhere else (DECISIONS, 2026-10-27): a
decomp commit never changes the engine by itself, and `engine/` could move to a
repository of its own without them.

| File | What the engine uses it for |
|---|---|
| `symbols.txt` | The function boundaries the translator works from, in the browser and natively. The names of the 10 SDK functions `runtime/hle.cpp` replaces or hooks by name. The page serves it as `symbols.txt`, and its SHA-256 is part of the browser's translation cache key (`build-id.json`) |
| `splits.txt` | Which unit each address belongs to, for the native development tools (`translator/units.h`) |
| `dol_layout.txt` | The DOL's section layout, for the local disc check (`vfs/tests/local_disc.cpp`) |
| `pin.json` | The decomp commit the copy came from (the last one that changed these files), and each file's SHA-256 |

They describe the retail binary (names, addresses, sizes), so they are metadata,
not game data (`docs/ARCHITECTURE.md` §4). Game functions, native replacements and
mod hooks bind by retail address, one of the three stable contracts; only the SDK
replacements bind by name.

## Updating the copy

Only with the tool, never by hand:

```
python -I engine/tools/sync_decomp_pin.py --check   # what would change
python -I engine/tools/sync_decomp_pin.py           # copy, and record the commit in pin.json
```

Sync when the engine needs something newer from the decomp, such as a name for a
function it is about to replace. Don't sync for every decomp commit. The tool lists
the functions added, removed, renamed and resized. Then build and run `ctest`.
Two data-free tests guard the copy:

- `decomp_pin_hashes`: the files are the ones `pin.json` records. This catches a
  hand edit or a CRLF checkout.
- `test_decomp_pin`: everything the engine binds resolves in the copy. Each
  name-keyed HLE entry must match exactly one function, and each address-keyed
  entry, mod hook site and SDK alias (`runtime/sdk_names.h`) must be a function
  start. The replaced SDK functions must sit in runtime-library units of
  `splits.txt`.

What a sync costs players: any change to `symbols.txt` makes every browser
translate the game again once, after the next deploy. A change to function
boundaries also changes the translated code itself, so check it in the browser
(`engine/web/tools/browser_check.mjs`) before it goes out, as for any engine
change.

Commit a sync on its own: `engine(decomp_pin): sync with the decomp at <commit>`.
