# BUILD SPEC — "Intern": an always-on personal AI that runs on the user's own machines

> **This document is a handoff to an implementing agent (e.g. hermes-agent).**
> The user is non-technical, sleep-deprived, and does NOT want to read code or
> jargon. They want to wake up to a working system. Build it. Make decisions.
> Ask the user ONLY when something is irreversible, costs real money, or is
> genuinely ambiguous — and when you do, ask in one plain-English sentence.

---

## 0. Who this is for (read this first)

The user is a single-parent professional influencer and music producer. They:
- have a Windows 10 PC (8GB VRAM), a VPS they don't use, Tailscale, WSL2,
  100+ AI tools, and APIs for Claude, Gemini, and OpenAI.
- **hate WSL2**, hate jargon, and hate being told to go to bed.
- love that Qwen wrote its own win32 toolset and drove the mouse ("gumption").
- want an AI that acts like an employee/intern: does the stuff they'd delegate,
  while they sleep, using their own resources, and escalates only the big calls.

**The north star:** they should be able to forget everything technical and still
have their digital life handled. Optimize for *their* cognitive load being near
zero, not for cleverness.

---

## 1. What "Intern" is

An always-on personal agent that runs across the user's own machines, routes each
task to the **cheapest model that can do it**, acts autonomously on safe/reversible
work, and queues big decisions for a one-tap yes/no. It learns the user from their
own activity and greets them with a **morning brief** of what it did overnight.

It is NOT a frontier-model-for-everything. Claude is the *decision layer*, not the
*worker*. Most work goes to local/cheap models. (See `../tool-registry/` — the
shared map of tools is already built; Intern is the thing that drives it.)

---

## 2. Architecture (full detail in ARCHITECTURE.md)

Three nodes on one Tailscale network:

| Node | Role | Notes |
|------|------|-------|
| **Windows PC (native)** | Hands. GUI/mouse control, reads local activity, runs local models | **NATIVE win32 — do NOT depend on WSL2.** This is a hard requirement; the user hates WSL2. |
| **VPS** | Brain. Runs the orchestrator 24/7 so it works while the PC sleeps | Long-running tasks, scheduling, the approval queue |
| **Phone / chat app** | Cockpit. One-tap approvals + morning brief | Telegram or Signal gateway (OpenClaw-style); voice via Gemini realtime |

Five layers: **Sensing → Memory → Router → Tools → Guardrails.** See ARCHITECTURE.md.

---

## 3. What to keep from the tools the user already has

| From | Keep | Drop / fix |
|------|------|-----------|
| OpenClaw | Local-first markdown memory; chat-app gateway | Generic "bridge to a coding agent" feel; relying on bolt-on safety |
| Hermes Agent | Persistent memory that adapts to the user; NL scheduling | The **WSL2 requirement** — run native Windows |
| Qwen yolo | **Self-tooling gumption** (writes + tests its own win32 tools) | Unsanctioned 30-min self-respawns; non-English output; recklessness |
| Claude | Quality judgment as the **decision layer** | Using it for high-frequency grunt work (token waste) |

---

## 4. Milestones (build in this order; each is shippable on its own)

- **M0 — Router online.** Wire `router.py` to `../tool-registry/tools.json`.
  Prove it routes a trivial task to a local Ollama/LM Studio model and a hard one
  to Claude. Acceptance: `python router.py "summarize this" --dry` prints the
  chosen tier + provider and why.
- **M1 — Orchestrator on the VPS.** A long-running loop reachable over Tailscale.
  Generates a **morning brief** (markdown) of overnight activity. Acceptance: brief
  file appears daily; viewable from the phone.
- **M2 — Native-Windows executor.** A win32 action runner on the PC (files, app
  launch, GUI/mouse via pyautogui or AHK) callable from the orchestrator over
  Tailscale. **No WSL2.** Acceptance: orchestrator tells PC to open a folder / move
  a file and it happens.
- **M3 — Guardrails + approval queue.** Wire `guardrails.py`. Reversible+cheap =
  auto. Irreversible / outward-facing / spend-over-cap = queued for one-tap yes/no
  from the phone. A `STOP` file anywhere = hard kill. Acceptance: a delete/post/
  purchase action pauses and pings the phone; tapping yes runs it.
- **M4 — Sensing / anticipation (consent-gated).** Read the user's OWN local
  activity (recent files, app focus, calendar) to anticipate needs. Data stays on
  their machines — never exfiltrated beyond the model APIs they already chose.
  Acceptance: morning brief includes a relevant suggestion drawn from yesterday.
- **M5 — Self-tooling.** Intern can write a new tool, test it, and **register it**
  in `tool-registry/tools.json` with an English description + a `reviewed:false`
  flag. Risky tools stay gated until the user approves. Acceptance: Intern adds a
  working tool to the registry on its own and uses it next time.

---

## 5. Hard requirements (do not violate)

1. **Native Windows. No WSL2 dependency.** WSL2 may be used for convenience by the
   implementer but must never be *required* to run Intern.
2. **Cheapest-capable model wins.** Default to local; climb to Claude only when
   nothing cheaper can do it, and log why. Claude is for decisions, not grunt work.
3. **Guardrails are non-negotiable.** Irreversible, outward-facing, or money-spending
   actions require explicit one-tap approval. A daily spend cap (config) is enforced.
   No silent self-respawning loops — every scheduled action is logged and rate-capped.
4. **Privacy/local-first.** The user's data lives on the user's machines. The only
   outbound traffic is to the model APIs the user already pays for (Claude/Gemini/
   OpenAI) and only what's needed for the task.
5. **English only**, plain language, in every user-facing message.
6. **Kill switch.** Presence of a file named `STOP` in the Intern home dir halts all
   autonomous action immediately.

---

## 6. Seed code in this folder

- `router.py` — model router: reads the registry, picks a tier, calls the provider.
- `guardrails.py` — action risk classifier, approval gate, budget ledger, kill switch.
- `config.example.json` — copy to `config.json`, fill in API keys + node addresses.

These are seeds, not the finished product. Expand them. Keep them dependency-light.

---

## 7. The vibe (this matters to the user)

Match the user's energy: capable, a little bold, warm, never condescending, never
nagging. Don't tell them to sleep. Don't lecture. Take initiative — that's the
"gumption" they want — but never at the cost of their money, their files, or their
trust. Be the intern who handled it and left a sticky note, not the one who needed
hand-holding or went rogue.
