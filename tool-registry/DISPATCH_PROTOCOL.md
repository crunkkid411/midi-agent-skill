# Dispatch Protocol

The checks-and-balances contract. Paste the block below into **Gemini's system
instructions** (your cockpit) and drop it in as a **Claude Code skill / memory**.
It turns "the model might remember to use my tools" into "the model must look
them up." Same rules for every agent.

---

## The contract (paste this into any agent)

```
You have a TOOL REGISTRY at tool-registry/tools.json. It is the single source
of truth for what tools exist on this machine and when to use them.

RESOURCE REQUEST PROTOCOL — when the user asks you to do a task, find a tool,
"use my X", or "get resources":

1. CONSULT — read tools.json before acting. Never invent a tool or assume one
   isn't there. If you haven't read it this session, read it now.
2. MATCH — pick the cheapest tier that can do the job:
      local-script > local-model > api-cheap > heavy-agent (Claude Code)
   Name the matched tool and one line on why it fits.
3. CONFIRM — state the EXACT command you will run (from the tool's "command"
   field). Wait for the user's go-ahead before running anything that writes,
   deletes, spends money, or controls the GUI.
4. RUN — execute, then report the result and which tool produced it.
5. MISS — if nothing in the registry fits, say so explicitly: "No registry
   match." Only then consider a heavy agent or writing a new tool. Do NOT
   silently improvise with a frontier model.

NEVER use a heavy/expensive agent for a task a registered local tool or cheap
API already covers. Escalation is a last resort, stated out loud.
```

---

## Why this works when skills/memory didn't

- **Skills/memory = recall.** The model has to *remember* your 100 tools at the
  right moment. It won't, reliably.
- **This protocol = lookup.** The model is forced to *read a file* every time.
  The knowledge lives outside the model, so it can't be forgotten or hallucinated.

## The tier ladder (cheapest wins)

| Tier | What | Use for |
|------|------|---------|
| `local-script` | Your scripts / AHK macros | Deterministic, repeatable jobs |
| `local-model` | Ollama / LM Studio | Cleanup, summarize, classify, route |
| `api-cheap` | Gemini Flash, Haiku, 4o-mini | Medium reasoning, vision on a few frames |
| `heavy-agent` | Claude Code, big agents | Building tools, hard multi-step engineering |

A task may only climb the ladder when every rung below it genuinely can't do it —
and the agent has to say so.
