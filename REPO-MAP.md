# REPO-MAP — what you already built (stop starting #34)

Honest inventory of all 33 repos under `crunkkid411`, grouped by what they
actually are. Built from repo names + descriptions only — **I could not read
inside the private repos this session** (access is locked to midi-agent-skill),
so treat the "survivor" picks as triage-by-description. Confirm before archiving.

The pattern this map exists to fix: you keep rebuilding the same personal agent
instead of finishing one. Pick a survivor per cluster, fold the good parts in,
archive the rest. Consolidate — don't spawn #34.

---

## 1. The personal agent (you built this ~10 times)

| Repo | What it is | Call |
|------|-----------|------|
| **Whoretana-X** (py, private) | Voice AI that hears/sees/controls Windows. **The one you actually use.** | ★ **SURVIVOR — cockpit** |
| whoretana (rust, private) | Older predecessor of the above | Harvest ideas → archive |
| becky2 (ts, private) | "Pattern-based personal AI assistant" | Harvest patterns → archive |
| becky-go (go, public) | "ADHD-friendly PC automation" CLI toolbelt | Harvest → archive |
| Super-Agent-Local (ts, private) | "local super agent for windows" | Archive |
| badass-windows (fork) | GUI computer-using-agent framework | Reference only |
| nanobot (fork) | "ultra-lightweight personal AI agent" | Reference only |
| paperclip (fork) | orchestration for "zero-human companies" | Reference only |
| gemini-desktop-buddy (ts, private) | "realtime gemini windows" | Likely folded into Whoretana-X → archive |
| 7-day-kilo-claw (private) | OpenClaw experiment logs | Keep as a journal, not active code |

## 2. Source of truth (revive this — it's the index)

| Repo | What it is | Call |
|------|-----------|------|
| **hairs-toolbelt** (private) | "Central source of truth — architecture, roadmap, specs" | ★ **SURVIVOR — the index. Point it at Whoretana-X.** |
| pc-automation-kb (private) | Windows automation knowledge base | Fold into hairs-toolbelt |

## 3. Video pipeline (your actual business — pick ONE path)

| Repo | What it is | Call |
|------|-----------|------|
| **CLI-Cut** (py, private) | AI video editing with kdenlive | ★ candidate survivor |
| video-use (py, fork) | "edit videos with coding agents" | ★ candidate survivor (pick one of these two) |
| Gemini-Video-Editor (ts, private) | edit videos with Gemini | Harvest → archive |
| clipsai / auto-editor (forks) | long video → clips | Keep as libraries |
| kdenlive (fork) | the editor itself | Keep (upstream) |
| kevs-obsidian-ingestion-engine (fork) | Dropbox→Whisper→Claude→Obsidian | Keep — this is a real pipeline |
| social_video_factory ×2, storytime-editor | older content factories | Archive the dupes |
| D-ORCA / DiCk-ORA | audio-visual captioning research | Keep if active research |

## 4. Memory / knowledge

| Repo | What it is | Call |
|------|-----------|------|
| **maw** (fork) | crawl a site into one searchable offline file | ★ survivor |
| maw-menoria (ts) | memory variant | Merge into maw or archive |

## 5. Misc forks (reference, not yours to maintain)
system-prompts-and-models-of-ai-tools · witsy-kokoro-onnx · CODEX-Game-Studios ·
Pake · replicate-python · scira · midi-agent-skill (this repo)

---

## The recommended survivor stack

- **Cockpit:** `Whoretana-X` — the multimodal voice agent you already use daily.
- **Index/source of truth:** `hairs-toolbelt` — revive it; make it point at Whoretana-X.
- **Video:** one of `CLI-Cut` / `video-use` + `kevs-obsidian-ingestion-engine`.
- **Memory:** `maw`.
- **Everything in `midi-agent-skill/tool-registry/` and `/intern/`** (the stuff I
  just made): these are NOT a new repo. They're notes to **fold into Whoretana-X** —
  the tool-registry = its tool map; guardrails.py = its approval gate. Then this
  midi repo goes back to being just a midi skill.

## For hermes-agent
You can read inside these repos; I can't. Verify each "harvest → archive" by
diffing for anything unique before archiving. The goal is ONE living agent
(Whoretana-X) with ONE index (hairs-toolbelt), not eleven half-finished ones.
