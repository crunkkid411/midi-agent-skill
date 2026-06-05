#!/usr/bin/env python3
"""
router.py - pick the CHEAPEST model that can do a task, then call it.

This is the seed of Intern's brain. The rule: local first, climb to Claude only
when nothing cheaper can do it, and always log WHY. Claude is the decision layer,
not the daily worker.

Reads the shared tool map at ../tool-registry/tools.json (built by scan_tools.py).
Dependency-light: stdlib only. Provider calls use urllib; fill in keys via config.

    python router.py "summarize these notes" --dry      # show the routing decision
    python router.py "design a release strategy"         # actually run (needs keys)

This is a SEED. hermes-agent: expand the heuristics, add streaming, add retries,
wire it to the orchestrator. Keep it readable.
"""
from __future__ import annotations

import argparse
import json
import os
import sys
import urllib.request
from pathlib import Path

HERE = Path(__file__).resolve().parent
REGISTRY = HERE.parent / "tool-registry" / "tools.json"
CONFIG = HERE / "config.json"

# Tier ladder, cheapest first. The router never climbs past what a task needs.
TIERS = ["local-script", "local-model", "api-cheap", "heavy-agent"]

# Crude complexity signal. Replace with a learned classifier later (M0+).
# The point: most tasks are NOT hard. Don't reach for Claude by reflex.
HARD_SIGNALS = ("design", "architect", "decide", "strategy", "debug", "plan",
                "tradeoff", "negotiat", "important", "refactor", "ambiguous")
CHEAP_SIGNALS = ("summarize", "rewrite", "tag", "classify", "clean", "transcribe",
                 "extract", "rename", "format", "list", "convert")


def load_json(path: Path, default: dict) -> dict:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        return default


def choose_tier(task: str) -> tuple[str, str]:
    """Return (tier, reason). Cheapest tier that plausibly handles the task."""
    t = task.lower()
    if any(s in t for s in CHEAP_SIGNALS):
        return "local-model", "routine text task a small local model handles fine"
    if any(s in t for s in HARD_SIGNALS):
        return "heavy-agent", "needs judgment/multi-step reasoning -> decision layer"
    # Default: try local first. Escalation happens only on failure/low-confidence.
    return "local-model", "no hard signal; default to cheapest and escalate on miss"


def pick_provider(tier: str, cfg: dict) -> dict:
    """Map a tier to a concrete provider from config."""
    providers = cfg.get("providers", {})
    default_for_tier = {
        "local-model": "ollama",
        "api-cheap": "gemini",
        "heavy-agent": "anthropic",
    }.get(tier)
    name = cfg.get("tier_provider", {}).get(tier, default_for_tier)
    prov = providers.get(name, {})
    prov["_name"] = name
    return prov


# --- provider calls (seeds; wire real auth/models in config.json) ------------- #
def _post(url: str, payload: dict, headers: dict, timeout: float = 60.0) -> dict:
    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(url, data=data, headers=headers, method="POST")
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.loads(r.read().decode("utf-8"))


def call_ollama(prov: dict, task: str) -> str:
    url = prov.get("url", "http://localhost:11434") + "/api/generate"
    out = _post(url, {"model": prov.get("model", "qwen2.5:7b"),
                      "prompt": task, "stream": False}, {"Content-Type": "application/json"})
    return out.get("response", "")


def call_openai_compatible(prov: dict, task: str) -> str:
    """Works for LM Studio, OpenAI, and Gemini's OpenAI-compatible endpoint."""
    url = prov["url"].rstrip("/") + "/chat/completions"
    headers = {"Content-Type": "application/json"}
    if prov.get("api_key"):
        headers["Authorization"] = "Bearer " + prov["api_key"]
    out = _post(url, {"model": prov.get("model", "gpt-4o-mini"),
                      "messages": [{"role": "user", "content": task}]}, headers)
    return out["choices"][0]["message"]["content"]


def call_anthropic(prov: dict, task: str) -> str:
    url = "https://api.anthropic.com/v1/messages"
    headers = {"Content-Type": "application/json",
               "x-api-key": prov.get("api_key", os.environ.get("ANTHROPIC_API_KEY", "")),
               "anthropic-version": "2023-06-01"}
    out = _post(url, {"model": prov.get("model", "claude-sonnet-4-6"),
                      "max_tokens": 1024,
                      "messages": [{"role": "user", "content": task}]}, headers)
    return "".join(b.get("text", "") for b in out.get("content", []))


PROVIDER_FN = {
    "ollama": call_ollama,
    "lmstudio": call_openai_compatible,
    "openai": call_openai_compatible,
    "gemini": call_openai_compatible,
    "anthropic": call_anthropic,
}


def route(task: str, dry: bool = False) -> str:
    cfg = load_json(CONFIG, {})
    tier, reason = choose_tier(task)
    prov = pick_provider(tier, cfg)
    decision = f"[route] tier={tier} provider={prov.get('_name')} model={prov.get('model')}\n[why]  {reason}"
    print(decision, file=sys.stderr)
    if dry:
        return decision
    if not prov or "_name" not in prov:
        return "(no provider configured for this tier - copy config.example.json to config.json)"
    fn = PROVIDER_FN.get(prov["_name"])
    if not fn:
        return f"(no caller implemented for provider '{prov['_name']}')"
    try:
        return fn(prov, task)
    except Exception as e:  # escalate-on-miss belongs here (M0+)
        return f"(provider call failed: {e})"


def main() -> None:
    ap = argparse.ArgumentParser(description="Route a task to the cheapest capable model.")
    ap.add_argument("task", help="the task / prompt")
    ap.add_argument("--dry", action="store_true", help="show the routing decision only")
    args = ap.parse_args()
    print(route(args.task, dry=args.dry))


if __name__ == "__main__":
    main()
