# Why not just OpenClaw / Hermes / Qwen? (grounded notes)

The user already runs all three and none is "it." Here's the honest read, and
what Intern borrows from each. (Researched June 2026 — verify before relying.)

## OpenClaw (openclaw, formerly Clawdbot/Moltbot)
- **What it is:** MIT-licensed, local-first, self-hosted *gateway* that bridges
  chat apps (Telegram/Signal/Discord/WhatsApp/…) to AI coding agents. Model-agnostic
  (Claude, GPT, Ollama). Local markdown memory. Can write its own skills. Runs shell,
  browses web, reads/writes files.
- **Why it's not it:** it's a generic bridge, not an opinionated personal chief of
  staff. There's no built-in "Claude makes the important calls, cheap models do the
  rest" routing, and no real cost discipline — safety is a bolt-on (NVIDIA shipped
  "NemoClaw" specifically to make OpenClaw agents safer). The user finds it not-it.
- **Keep:** local-first markdown memory; the chat-app gateway as the cockpit.

## Hermes Agent (Nous Research, Feb 2026)
- **What it is:** open-source autonomous agent with **persistent memory that grows
  with you**, NL scheduling of recurring automations, runs entirely on your hardware,
  zero per-message cost, no telemetry. This is the "it understands me" the user loves.
- **Why it's not it:** **requires WSL2 — native Windows is explicitly unsupported by
  the installer.** The user hates WSL2. That single dependency is the dealbreaker.
- **Keep:** the persistent, adaptive memory model and NL scheduling — but run it
  **native Windows**.

## Qwen CLI (yolo mode)
- **What it is:** wrote its own win32 toolset and tested it by literally dragging the
  mouse and putting itself in yolo mode. Native Windows. Real initiative.
- **Why it's not it:** dumber than Claude at judgment; sometimes outputs Chinese;
  self-respawns every ~30 min and acts without sanction. Gumption without judgment.
- **Keep:** the **self-tooling on native Windows** — but every self-made tool gets
  registered, English-documented, and risky ones gated behind approval.

## The synthesis = Intern
**Hermes' adaptive memory + Qwen's native-Windows self-tooling gumption + Claude as
the decision layer + OpenClaw's local-first chat cockpit — minus WSL2, minus rogue
behavior, minus token waste.** Cheap models do the grunt work; Claude makes the calls
that matter; guardrails keep the gumption from becoming a liability.

## Sources
- [openclaw/openclaw (GitHub)](https://github.com/openclaw/openclaw)
- [OpenClaw — Complete Guide (Milvus)](https://milvus.io/blog/openclaw-formerly-clawdbot-moltbot-explained-a-complete-guide-to-the-autonomous-ai-agent.md)
- [Secure always-on local agent with OpenClaw + NVIDIA NemoClaw](https://developer.nvidia.com/blog/build-a-secure-always-on-local-ai-agent-with-nvidia-nemoclaw-and-openclaw/)
- [Hermes Agent (Nous Research)](https://hermes-agent.nousresearch.com/docs/getting-started/installation)
- [NousResearch/hermes-agent (GitHub)](https://github.com/NousResearch/hermes-agent)
- [Hermes Agent Windows (WSL2) guide](https://hermes-agent.nousresearch.com/docs/user-guide/windows-wsl-quickstart)
