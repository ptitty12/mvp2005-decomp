#!/usr/bin/env python3
"""Update the engine's pinned copy of the decomp's metadata (engine/decomp_pin/).

The engine reads three files the decompilation keeps in config/GV4E69/, and only
through its own copy of them (DECISIONS 2026-10-27, option (b)):

- symbols.txt: function boundaries for the translator, and the names of the SDK
  functions the runtime replaces by name (runtime/hle.cpp);
- splits.txt: which unit each address belongs to (native development tools);
- dol_layout.txt: the DOL's section layout (the local disc check).

A decomp commit never changes the engine by itself. This tool copies the decomp's
current files on purpose, records the decomp commit they came from and their
SHA-256 in engine/decomp_pin/pin.json, and prints what changed in the function
boundaries and names. Then build and run ctest: test_decomp_pin checks every name
and address the engine binds against the new copy, and decomp_pin_hashes checks
the copy against pin.json.

    python -I engine/tools/sync_decomp_pin.py            copy from this checkout's config/GV4E69
    python -I engine/tools/sync_decomp_pin.py --check    only report what would change
    python -I engine/tools/sync_decomp_pin.py --from DIR another checkout of the decomp

The files are copied byte for byte, so the browser's translation cache key (the
SHA-256 of symbols.txt in build-id.json) changes only when the content does.
Python standard library only. Exit status: 0 done (or, with --check, nothing to
change), 1 an error (or, with --check, the decomp's files differ), 2 bad usage.
"""
import argparse
import hashlib
import json
import os
import re
import subprocess
import sys

FILES = ("symbols.txt", "splits.txt", "dol_layout.txt")
SOURCE = "config/GV4E69"
HERE = os.path.dirname(os.path.abspath(__file__))
PIN = os.path.join(HERE, "..", "decomp_pin")

SYMBOL = re.compile(r"^(\S+) = (\S+):0x([0-9A-Fa-f]+); // (.*)$")


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def read(path):
    with open(path, "rb") as f:
        return f.read()


def functions(text):
    """Functions in a symbols.txt: start address -> (name, size)."""
    out = {}
    for line in text.decode("utf-8").splitlines():
        m = SYMBOL.match(line)
        if not m or "type:function" not in m.group(4).split():
            continue
        size = re.search(r"\bsize:0x([0-9A-Fa-f]+)", m.group(4))
        out[int(m.group(3), 16)] = (m.group(1), int(size.group(1), 16) if size else 0)
    return out


def git(repo, *args):
    return subprocess.run(["git", "-C", repo, *args], check=True, capture_output=True, text=True).stdout.strip()


def describe(old, new, limit=8):
    """Lines describing how the functions of two symbols.txt files differ."""
    a, b = functions(old), functions(new)
    added = sorted(set(b) - set(a))
    removed = sorted(set(a) - set(b))
    renamed = sorted(k for k in set(a) & set(b) if a[k][0] != b[k][0])
    resized = sorted(k for k in set(a) & set(b) if a[k][1] != b[k][1])
    lines = [f"  functions: {len(a)} -> {len(b)}"]
    for title, keys, show in (
        ("added", added, lambda k: f"{b[k][0]} at 0x{k:08X}, size 0x{b[k][1]:X}"),
        ("removed", removed, lambda k: f"{a[k][0]} at 0x{k:08X}"),
        ("renamed", renamed, lambda k: f"0x{k:08X}: {a[k][0]} -> {b[k][0]}"),
        ("resized", resized, lambda k: f"{b[k][0]} at 0x{k:08X}: 0x{a[k][1]:X} -> 0x{b[k][1]:X}"),
    ):
        if not keys:
            continue
        lines.append(f"  {title}: {len(keys)}")
        lines += [f"    {show(k)}" for k in keys[:limit]]
        if len(keys) > limit:
            lines.append(f"    ... and {len(keys) - limit} more")
    if added or removed or resized:
        lines.append("  Function boundaries changed: the translation changes, so check it in the browser")
        lines.append("  (browser_check.mjs) as well as with ctest before this goes out.")
    return lines


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--from", dest="src", default=os.path.join(HERE, "..", ".."),
                    help="a checkout of the decomp (default: the one this file is in)")
    ap.add_argument("--check", action="store_true", help="only report what would change; write nothing")
    args = ap.parse_args()

    src = os.path.abspath(args.src)
    pin = os.path.abspath(PIN)
    new = {}
    for name in FILES:
        path = os.path.join(src, SOURCE, name)
        if not os.path.isfile(path):
            print(f"sync_decomp_pin: {path} is missing; --from must name a checkout of the decomp", file=sys.stderr)
            return 1
        new[name] = read(path)
        if b"\r" in new[name]:
            print(f"sync_decomp_pin: {path} has CR line endings; the decomp keeps config/**/*.txt as LF "
                  "(.gitattributes), so check that checkout", file=sys.stderr)
            return 1
    old = {name: read(os.path.join(pin, name)) for name in FILES if os.path.isfile(os.path.join(pin, name))}

    changed = [name for name in FILES if old.get(name) != new[name]]
    print(f"decomp: {os.path.join(src, SOURCE)}")
    print(f"pin:    {pin}")
    for name in FILES:
        print(f"  {name}: {'changed' if name in changed else 'unchanged'}")
    if "symbols.txt" in changed and "symbols.txt" in old:
        print("symbols.txt:")
        for line in describe(old["symbols.txt"], new["symbols.txt"]):
            print(line)
    if args.check:
        return 1 if changed else 0
    sys.stdout.flush()

    # The commit to record: the last one that changed these files, with nothing uncommitted on top.
    paths = [f"{SOURCE}/{name}" for name in FILES]
    try:
        dirty = git(src, "status", "--porcelain", "--", *paths)
        commit = git(src, "log", "-1", "--format=%H", "--", *paths)
    except (OSError, subprocess.CalledProcessError) as e:
        print(f"sync_decomp_pin: git failed in {src}: {e}", file=sys.stderr)
        return 1
    if dirty:
        print(f"sync_decomp_pin: {SOURCE} has uncommitted changes; commit them first, so that the pin names "
              f"the decomp commit it matches:\n{dirty}", file=sys.stderr)
        return 1

    os.makedirs(pin, exist_ok=True)
    for name in FILES:
        with open(os.path.join(pin, name), "wb") as f:
            f.write(new[name])
    record = {
        "format": 1,
        "from": f"{SOURCE} of the decompilation",
        "commit": commit,
        "files": {name: sha256(new[name]) for name in sorted(FILES)},
    }
    with open(os.path.join(pin, "pin.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(record, f, indent=2)
        f.write("\n")
    print(f"pinned at decomp commit {commit}")
    print("next: build the engine and run ctest (test_decomp_pin, decomp_pin_hashes), then commit as")
    print(f"  engine(decomp_pin): sync with the decomp at {commit[:10]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
