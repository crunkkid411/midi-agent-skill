# Architecture — Intern

Five layers, bottom to top. Each is independently testable.

```
                 ┌─────────────────────────────────────────┐
   PHONE  ─────▶ │  COCKPIT: chat app + voice (Gemini RT)   │  one-tap approvals,
                 │           morning brief                  │  talk to it
                 └───────────────────┬─────────────────────┘
                                     │ Tailscale (private mesh)
   VPS (24/7) ──▶ ┌──────────────────┴─────────────────────┐
                  │ 5. GUARDRAILS  approve / budget / STOP   │
                  │ 4. TOOLS       tool-registry/tools.json  │  ← already built
                  │ 3. ROUTER      cheapest-capable model    │
                  │ 2. MEMORY      markdown/JSON on disk      │
                  │ 1. SENSING     reads user's OWN activity  │
                  └──────────────────┬─────────────────────┘
                                     │ Tailscale
   WINDOWS PC ──▶ ┌──────────────────┴─────────────────────┐
   (native win32)  │ HANDS: files, app launch, GUI/mouse,    │  no WSL2 required
                   │ local models (Ollama / LM Studio)        │
                   └─────────────────────────────────────────┘
```

## 1. Sensing (consent-gated)
Reads the user's own signals to anticipate: recently modified files, foreground
app, calendar, clipboard, recent downloads. **Local only.** This is what makes it
*understand* the user (the thing they liked about Hermes/Qwen reading their logs).
Off by default per-source; user flips sources on. Nothing leaves their machines
except what a chosen task sends to a chosen API.

## 2. Memory
Local-first, like OpenClaw/Hermes: plain markdown + JSON on disk, synced across
nodes over Tailscale. Holds: durable facts about the user, project state, learned
preferences (e.g. "user works in bursts at night", "stems go to D:\Exports"), and
a rolling activity log. No hosted DB, no lock-in.

## 3. Router  (`router.py`)
Given a task, pick the cheapest tier that can do it, using the registry's tiers:
`local-script → local-model → api-cheap → heavy-agent`. Climb only when forced;
log the reason. Providers: Ollama, LM Studio (OpenAI-compatible), Gemini, OpenAI,
Anthropic. Claude = the decision layer for ambiguous/important calls only.

## 4. Tools  (`../tool-registry/`)
The shared map of all 100+ tools — what exists, when to use it, how to run it.
Already built. Intern reads it before acting (the Dispatch Protocol) and can add
to it (self-tooling, M5), English-documented and reviewed.

## 5. Guardrails  (`guardrails.py`)
The line between "helpful intern" and "Qwen respawning itself every 30 minutes":
- **Reversible + cheap → act autonomously.**
- **Irreversible / outward-facing / spend > daily cap → queue for one-tap yes/no.**
- **Daily budget cap** across all paid APIs, enforced by a ledger.
- **Rate cap** on scheduled/self-initiated actions (no runaway loops).
- **`STOP` file = instant halt** of all autonomy.

## The morning brief
Every morning Intern writes a short, plain-English brief: what it did overnight,
what it spent, what's waiting for a yes/no, and one suggestion. This is the "wake
up to results" the user asked for — results they can read in 20 seconds, not a log.

## Why three nodes
The VPS is the always-on brain so things happen while the PC sleeps. The PC is the
hands (native win32 — GUI, mouse, local GPU models). Tailscale makes them one private
network with zero port-forwarding. The phone is just a window in.
