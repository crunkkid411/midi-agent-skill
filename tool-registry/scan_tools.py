#!/usr/bin/env python3
"""
scan_tools.py - Build/refresh a registry of your local AI tools.

Why this exists
---------------
You have 100+ tools and forget what they are. Agents (Gemini, Claude, Hermes)
re-guess your toolbox every time, so they work "sometimes". This script walks
your machine ONCE and writes a single map (tools.json) that every agent can read.

It is deliberately dependency-free (Python stdlib only) so you can run it on a
fresh Windows box with nothing but Python installed.

Usage
-----
    python scan_tools.py                # scan using scan_config.json
    python scan_tools.py --init         # write a starter scan_config.json
    python scan_tools.py --dry-run      # show what it found, write nothing

What it preserves
-----------------
Re-running NEVER clobbers your hand-written notes. Once you set a tool's
"description", "when_to_use", "tier", or "command", or mark it "reviewed": true,
the scanner keeps your text and only refreshes auto fields (path, last_seen).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import urllib.request
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent
CONFIG_PATH = HERE / "scan_config.json"
REGISTRY_PATH = HERE / "tools.json"
INDEX_PATH = HERE / "INDEX.md"

# file extension -> (tool kind, how you'd typically run it)
SCRIPT_KINDS = {
    ".py":  ("python-script", "python {path}"),
    ".ps1": ("powershell-script", "powershell -File {path}"),
    ".bat": ("batch-script", "{path}"),
    ".cmd": ("batch-script", "{path}"),
    ".ahk": ("autohotkey-macro", "autohotkey {path}"),
    ".sh":  ("shell-script", "bash {path}"),
    ".js":  ("node-script", "node {path}"),
    ".ts":  ("node-script", "ts-node {path}"),
}

DEFAULT_CONFIG = {
    "script_folders": [
        # Add the folders where your scripts/macros live, e.g.:
        # "C:/Users/you/scripts",
        # "C:/Users/you/Documents/AutoHotkey",
    ],
    "ollama_url": "http://localhost:11434",
    "lmstudio_url": "http://localhost:1234",
    "apis": [
        # Hand-list cloud tools you want agents to know about. Example:
        # {"id": "gemini-flash-realtime", "tier": "api-cheap",
        #  "description": "Realtime voice conversation + delegation + computer control",
        #  "when_to_use": "Default cockpit. Talk, delegate, drive the machine in real time.",
        #  "command": "(handled by your Gemini realtime client)"}
    ],
    "ignore_dirs": [".git", "node_modules", "__pycache__", ".venv", "venv"],
}


def log(msg: str) -> None:
    print(msg, file=sys.stderr)


# --------------------------------------------------------------------------- #
# Config
# --------------------------------------------------------------------------- #
def load_config() -> dict:
    if not CONFIG_PATH.exists():
        log(f"No config found. Run:  python {Path(__file__).name} --init")
        sys.exit(1)
    return json.loads(CONFIG_PATH.read_text(encoding="utf-8"))


def write_starter_config() -> None:
    if CONFIG_PATH.exists():
        log(f"{CONFIG_PATH.name} already exists - leaving it alone.")
        return
    CONFIG_PATH.write_text(json.dumps(DEFAULT_CONFIG, indent=2), encoding="utf-8")
    log(f"Wrote {CONFIG_PATH.name}. Edit it to point at your script folders, then re-run.")


# --------------------------------------------------------------------------- #
# Description extraction
# --------------------------------------------------------------------------- #
def extract_description(path: Path) -> str:
    """Best-effort one-liner from a script's top comment / docstring."""
    try:
        head = path.read_text(encoding="utf-8", errors="ignore")[:2000]
    except Exception:
        return ""
    # Python triple-quoted docstring
    m = re.search(r'"""(.*?)"""', head, re.DOTALL)
    if m:
        first = next((ln.strip() for ln in m.group(1).splitlines() if ln.strip()), "")
        if first:
            return first[:200]
    # Leading comment lines (#, //, ;, REM)
    for raw in head.splitlines():
        line = raw.strip()
        if not line:
            continue
        if line.startswith("#!"):
            continue
        m = re.match(r'^(?:#|//|;|REM\b|::)\s*(.+)', line, re.IGNORECASE)
        if m:
            return m.group(1).strip()[:200]
        break  # first non-blank, non-comment line -> stop
    return ""


def slugify(name: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", name.lower()).strip("-")


# --------------------------------------------------------------------------- #
# Scanners
# --------------------------------------------------------------------------- #
def scan_scripts(folders: list[str], ignore_dirs: set[str]) -> dict[str, dict]:
    found: dict[str, dict] = {}
    for folder in folders:
        root = Path(os.path.expanduser(folder))
        if not root.exists():
            log(f"  ! script folder not found: {root}")
            continue
        for path in root.rglob("*"):
            if any(part in ignore_dirs for part in path.parts):
                continue
            if not path.is_file():
                continue
            kind_run = SCRIPT_KINDS.get(path.suffix.lower())
            if not kind_run:
                continue
            kind, run_tmpl = kind_run
            tool_id = slugify(path.stem)
            base = tool_id
            n = 2
            while tool_id in found:
                tool_id = f"{base}-{n}"
                n += 1
            found[tool_id] = {
                "id": tool_id,
                "name": path.stem,
                "kind": kind,
                "tier": "local-script",
                "description": extract_description(path),
                "when_to_use": "",
                "command": run_tmpl.format(path=str(path)),
                "path": str(path),
                "source": "scanned",
                "reviewed": False,
            }
    return found


def _http_json(url: str, timeout: float = 3.0) -> dict | None:
    try:
        with urllib.request.urlopen(url, timeout=timeout) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except Exception:
        return None


def scan_ollama(base_url: str) -> dict[str, dict]:
    data = _http_json(base_url.rstrip("/") + "/api/tags")
    if not data:
        log("  - Ollama not reachable (skipping)")
        return {}
    found = {}
    for m in data.get("models", []):
        model = m.get("name", "")
        if not model:
            continue
        tid = "ollama-" + slugify(model)
        found[tid] = {
            "id": tid,
            "name": model,
            "kind": "local-model",
            "tier": "local-model",
            "description": f"Local model served by Ollama ({model}).",
            "when_to_use": "",
            "command": f"ollama run {model}",
            "path": base_url,
            "source": "scanned",
            "reviewed": False,
        }
    log(f"  + Ollama: {len(found)} model(s)")
    return found


def scan_lmstudio(base_url: str) -> dict[str, dict]:
    data = _http_json(base_url.rstrip("/") + "/v1/models")
    if not data:
        log("  - LM Studio not reachable (skipping)")
        return {}
    found = {}
    for m in data.get("data", []):
        model = m.get("id", "")
        if not model:
            continue
        tid = "lmstudio-" + slugify(model)
        found[tid] = {
            "id": tid,
            "name": model,
            "kind": "local-model",
            "tier": "local-model",
            "description": f"Local model served by LM Studio ({model}).",
            "when_to_use": "",
            "command": f"(OpenAI-compatible) POST {base_url}/v1/chat/completions model={model}",
            "path": base_url,
            "source": "scanned",
            "reviewed": False,
        }
    log(f"  + LM Studio: {len(found)} model(s)")
    return found


def scan_apis(apis: list[dict]) -> dict[str, dict]:
    found = {}
    for entry in apis:
        tid = entry.get("id") or slugify(entry.get("name", "api"))
        found[tid] = {
            "id": tid,
            "name": entry.get("name", tid),
            "kind": "api",
            "tier": entry.get("tier", "api-cheap"),
            "description": entry.get("description", ""),
            "when_to_use": entry.get("when_to_use", ""),
            "command": entry.get("command", ""),
            "path": entry.get("path", ""),
            "source": "manual",
            "reviewed": True,
        }
    return found


# --------------------------------------------------------------------------- #
# Merge (preserve human edits)
# --------------------------------------------------------------------------- #
HUMAN_FIELDS = ("description", "when_to_use", "tier", "command")


def merge(old: dict, new: dict) -> dict:
    """Keep human-written fields from `old`, refresh auto fields from `new`."""
    merged = dict(new)
    if old.get("reviewed"):
        merged["reviewed"] = True
    for f in HUMAN_FIELDS:
        old_val = old.get(f, "")
        # keep the human value if it was reviewed or differs from a blank/auto default
        if old.get("reviewed") and old_val:
            merged[f] = old_val
        elif old_val and old_val != new.get(f, ""):
            # only keep non-empty manual overrides for description/when_to_use
            if f in ("description", "when_to_use") and old.get("source") != "scanned":
                merged[f] = old_val
            elif f in ("tier", "command") and old.get("reviewed"):
                merged[f] = old_val
            elif old.get("reviewed"):
                merged[f] = old_val
    return merged


def load_registry() -> dict[str, dict]:
    if not REGISTRY_PATH.exists():
        return {}
    data = json.loads(REGISTRY_PATH.read_text(encoding="utf-8"))
    return {t["id"]: t for t in data.get("tools", [])}


# --------------------------------------------------------------------------- #
# Output
# --------------------------------------------------------------------------- #
def write_registry(tools: dict[str, dict]) -> None:
    payload = {
        "version": 1,
        "generated_at": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "tool_count": len(tools),
        "tools": sorted(tools.values(), key=lambda t: (t["tier"], t["id"])),
    }
    REGISTRY_PATH.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    log(f"Wrote {REGISTRY_PATH.name} ({len(tools)} tools)")


def write_index(tools: dict[str, dict]) -> None:
    by_tier: dict[str, list[dict]] = {}
    for t in tools.values():
        by_tier.setdefault(t["tier"], []).append(t)
    lines = [
        "# Tool Registry (human view)",
        "",
        f"_Auto-generated by scan_tools.py — {len(tools)} tools. "
        "Edit descriptions in `tools.json` and set `\"reviewed\": true` to lock them._",
        "",
        "> ⚠️ Entries with an empty **when_to_use** are invisible to your agents in",
        "> practice — fill those in first; that's the whole point of the registry.",
        "",
    ]
    for tier in sorted(by_tier):
        lines.append(f"## {tier}")
        lines.append("")
        lines.append("| Tool | When to use | Run |")
        lines.append("|------|-------------|-----|")
        for t in sorted(by_tier[tier], key=lambda x: x["id"]):
            wtu = t.get("when_to_use") or "_(unset)_"
            desc = t.get("description") or ""
            cell = f"**{t['name']}**<br><sub>{desc}</sub>" if desc else f"**{t['name']}**"
            cmd = (t.get("command") or "").replace("|", "\\|")
            lines.append(f"| {cell} | {wtu} | `{cmd}` |")
        lines.append("")
    INDEX_PATH.write_text("\n".join(lines), encoding="utf-8")
    log(f"Wrote {INDEX_PATH.name}")


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #
def main() -> None:
    ap = argparse.ArgumentParser(description="Scan local AI tools into a registry.")
    ap.add_argument("--init", action="store_true", help="write a starter scan_config.json")
    ap.add_argument("--dry-run", action="store_true", help="scan but write nothing")
    args = ap.parse_args()

    if args.init:
        write_starter_config()
        return

    cfg = load_config()
    ignore = set(cfg.get("ignore_dirs", []))

    log("Scanning...")
    discovered: dict[str, dict] = {}
    discovered.update(scan_scripts(cfg.get("script_folders", []), ignore))
    log(f"  + scripts: {len(discovered)} so far")
    discovered.update(scan_ollama(cfg.get("ollama_url", "")))
    discovered.update(scan_lmstudio(cfg.get("lmstudio_url", "")))
    discovered.update(scan_apis(cfg.get("apis", [])))

    existing = load_registry()
    merged: dict[str, dict] = {}
    for tid, new in discovered.items():
        merged[tid] = merge(existing[tid], new) if tid in existing else new
    # keep manual/api entries that no longer scan (don't silently drop hand-written tools)
    for tid, old in existing.items():
        if tid not in merged and old.get("source") == "manual":
            merged[tid] = old

    unset = sum(1 for t in merged.values() if not t.get("when_to_use"))
    log(f"\nTotal: {len(merged)} tools  ({unset} still missing 'when_to_use')")

    if args.dry_run:
        log("(dry run — nothing written)")
        return

    write_registry(merged)
    write_index(merged)
    log("\nNext: open INDEX.md, fill in 'when_to_use' for the tools you actually use,")
    log("set \"reviewed\": true on them in tools.json, and point your agents at tools.json.")


if __name__ == "__main__":
    main()
