# Tool Registry — a shared map for all your AI agents

You have 100+ tools and forget what they do. Gemini, Claude, and Hermes each
re-guess your toolbox every time, so they only work *sometimes*. This fixes that
with **one map every agent reads** instead of each one trying to remember.

## The mental model

- **Gemini Flash realtime = the cockpit.** Your driver. Talk to it, it delegates.
- **`tools.json` = the map.** Every tool: what it is, when to use it, how to run it.
- **Claude Code = the mechanic.** Builds & maintains the map and new tools. Not your daily driver.

The intelligence moves *out of the model's memory* and *into a file*. Lookup, not recall.

## Quick start (on your Windows PC)

```bash
# 1. one-time: create a config and point it at your script folders
python scan_tools.py --init
#    then edit scan_config.json -> add your script folders

# 2. scan your machine (also auto-finds Ollama + LM Studio models)
python scan_tools.py

# 3. open INDEX.md, fill in "when_to_use" for the tools you actually use,
#    and set "reviewed": true on them in tools.json to lock your notes

# 4. point your agents at tools.json using DISPATCH_PROTOCOL.md
```

No dependencies — just Python. Re-run `scan_tools.py` anytime; it **never**
overwrites notes you've marked `reviewed`.

## Files

| File | What it is |
|------|------------|
| `scan_tools.py` | Scanner that builds/refreshes the registry |
| `scan_config.json` | Where to scan (created by `--init`) |
| `tools.json` | The registry — the map agents read |
| `INDEX.md` | Human-readable view, auto-generated |
| `DISPATCH_PROTOCOL.md` | The checks-and-balances contract for your agents |

## The one rule

Cheapest tier that can do the job wins:
**local-script → local-model → api-cheap → heavy-agent.**
A task only climbs when the rung below genuinely can't do it — and the agent has
to say so out loud. That's how you stop burning tokens on stuff a 4B model or a
plain script could've handled.
