#!/usr/bin/env python3
"""
guardrails.py - the line between a helpful intern and a tool that respawns itself
every 30 minutes and acts without permission.

Three jobs:
  1. classify_action  - how risky is this action?
  2. needs_approval   - should it pause for a one-tap yes/no?
  3. budget + STOP    - daily spend cap, and a hard kill switch.

Dependency-light: stdlib only. hermes-agent: wire needs_approval() into the
orchestrator so risky actions get queued and pinged to the phone, and call
record_spend() after every paid API call.
"""
from __future__ import annotations

import json
from datetime import date
from pathlib import Path

HERE = Path(__file__).resolve().parent
STOP_FILE = HERE / "STOP"           # create this file to halt ALL autonomy instantly
LEDGER = HERE / "spend_ledger.json"
CONFIG = HERE / "config.json"

# Actions that can never quietly auto-run, no matter how cheap.
IRREVERSIBLE = ("delete", "rm ", "drop ", "overwrite", "format", "uninstall", "kill")
OUTWARD = ("post", "publish", "send", "email", "tweet", "upload", "commit", "push", "pay", "buy", "purchase")


def killed() -> bool:
    """Hard kill switch: presence of a STOP file halts everything."""
    return STOP_FILE.exists()


def classify_action(action: str) -> str:
    """Return 'block-until-approved' | 'approve-if-spendy' | 'auto'."""
    a = action.lower()
    if any(k in a for k in IRREVERSIBLE):
        return "block-until-approved"
    if any(k in a for k in OUTWARD):
        return "block-until-approved"
    return "auto"


def _daily_cap() -> float:
    cfg = json.loads(CONFIG.read_text(encoding="utf-8")) if CONFIG.exists() else {}
    return float(cfg.get("daily_spend_cap_usd", 5.0))


def _ledger() -> dict:
    if not LEDGER.exists():
        return {}
    return json.loads(LEDGER.read_text(encoding="utf-8"))


def spent_today() -> float:
    return float(_ledger().get(str(date.today()), 0.0))


def record_spend(usd: float) -> None:
    led = _ledger()
    today = str(date.today())
    led[today] = round(led.get(today, 0.0) + usd, 4)
    LEDGER.write_text(json.dumps(led, indent=2), encoding="utf-8")


def needs_approval(action: str, est_cost_usd: float = 0.0) -> tuple[bool, str]:
    """The gate the orchestrator must call before doing anything.
    Returns (must_ask, reason)."""
    if killed():
        return True, "STOP file present - all autonomy halted"
    klass = classify_action(action)
    if klass == "block-until-approved":
        return True, "irreversible or outward-facing - needs one-tap yes/no"
    if spent_today() + est_cost_usd > _daily_cap():
        return True, f"would exceed daily spend cap (${_daily_cap():.2f})"
    return False, "reversible, cheap, within budget - safe to auto-run"


if __name__ == "__main__":
    # quick self-check
    for a in ["summarize my notes", "delete old_stems folder", "post to instagram",
              "rename file"]:
        must, why = needs_approval(a, est_cost_usd=0.01)
        print(f"{'ASK ' if must else 'AUTO'}  {a:32}  -> {why}")
