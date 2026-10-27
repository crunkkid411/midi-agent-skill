"""
Controller automation — filter sweeps, sidechain pumping, wobbles, pitch dives.

Sections bind automation lanes:

    "automation": [
      {"param": "filter", "shape": "ramp_up", "roles": ["pad", "lead"], "from": 0.15, "to": 1.0},
      {"param": "volume", "shape": "pump", "roles": ["pad"], "depth": 0.35, "rate": "4"},
      {"param": "filter", "shape": "wobble", "roles": ["synth_bass"], "rate": "8"},
      {"param": "pitch",  "shape": "dive", "roles": ["rhythm_guitar"], "beats": 2}
    ]

Params are abstract — filter, volume, expression, mod_wheel, pitch,
macro1..macro8 — and the track's instrument map turns them into CC numbers
or pitch bend at render time (GM defaults: filter CC74, volume CC7,
expression CC11, mod wheel CC1).

Shapes:
  ramp_up / ramp_down — linear from → to across the section (builds, filtered intros/outros)
  hold                — constant `value`
  pump                — sidechain duck: drops to `depth` on every `rate` hit, recovers (four-on-the-floor feel)
  wobble              — LFO between `from` and `to` at `rate` ("4", "8", "8t", "16")
  dive                — pitch: falls from 0 to `to` (default -1 = full bend down) over the last `beats` (tape stop)
  rise                — pitch: climbs from `from` (default -1) to 0 over the first `beats`
Values are 0..1 (pitch: -1..1, 0 = centre). Events are written at 16th-note resolution.
Pitch lanes may give "semitones" (e.g. 12 for an octave tape-stop dive): the
renderer converts it with the instrument's pitch_bend_range.
"""

from typing import Dict, List
import math

from midi_types.music import Section, Track

RATES = {"1": 4.0, "2": 2.0, "4": 1.0, "8": 0.5, "8t": 1 / 3, "16": 0.25, "16t": 1 / 6}
STEP = 0.25


def apply_automation(tracks: List[Track], sections: List[Section]) -> None:
    """Write section-bound automation lanes onto the tracks they target.
    Top-level sections and micro-sections both count; a micro-section's
    lanes come from its own binding (bindings are inherited, so only the
    top-level copy is used unless the micro adds or changes it)."""
    seen = set()
    for sec in sections:
        lanes = sec.bindings.get("automation") or []
        parent = next((s for s in sections if sec.is_micro and s.name == sec.name.partition(".")[0]), None)
        if parent is not None and (parent.bindings.get("automation") or []) == lanes:
            continue
        for lane in lanes:
            key = (sec.name, repr(sorted(lane.items())))
            if key in seen:
                continue
            seen.add(key)
            roles = set(lane.get("roles") or [])
            for t in tracks:
                if roles and t.role not in roles:
                    continue
                t.automation.extend(render_lane(lane, sec.start_beat(), sec.end_beat()))
    for t in tracks:
        t.automation.sort(key=lambda e: (e["param"], e["time"]))


def render_lane(lane: Dict, start: float, end: float) -> List[Dict]:
    param = lane.get("param", "filter")
    shape = lane.get("shape", "hold")
    lo = float(lane.get("from", 0.0 if shape != "rise" else -1.0))
    hi = float(lane.get("to", 1.0 if shape != "dive" else -1.0))
    length = max(STEP, end - start)
    out: List[Dict] = []

    semis = lane.get("semitones") if param == "pitch" else None

    def ev(t: float, v: float) -> None:
        e = {"param": param, "time": round(t, 4), "value": round(v, 4)}
        if semis is not None:
            e["semitones"] = round(v * float(semis), 3)
        out.append(e)

    if shape in ("ramp_up", "ramp_down"):
        a, b = (lo, hi) if shape == "ramp_up" else (hi, lo)
        n = int(length / STEP)
        for i in range(n + 1):
            ev(start + i * STEP, a + (b - a) * i / max(1, n))
        ev(end, b if lane.get("reset") is None else float(lane["reset"]))
    elif shape == "hold":
        ev(start, float(lane.get("value", hi)))
        if lane.get("reset") is not None:
            ev(end, float(lane["reset"]))
    elif shape == "pump":
        period = RATES.get(str(lane.get("rate", "4")), 1.0)
        depth = float(lane.get("depth", 0.35))
        t = start
        while t < end - 1e-6:
            for frac, v in ((0.0, depth), (0.25, depth + (1 - depth) * 0.55),
                            (0.5, depth + (1 - depth) * 0.85), (0.75, 1.0)):
                ev(t + period * frac, v)
            t += period
        ev(end, 1.0)
    elif shape == "wobble":
        period = RATES.get(str(lane.get("rate", "8")), 0.5)
        n = int(length / (period / 4))
        for i in range(n):
            t = start + i * period / 4
            phase = (i % 4) / 4
            ev(t, lo + (hi - lo) * (0.5 - 0.5 * math.cos(2 * math.pi * phase)))
        ev(end, lo)
    elif shape == "dive":
        beats = min(length, float(lane.get("beats", 2.0)))
        t0 = end - beats
        ev(start, 0.0)
        n = int(beats / STEP)
        for i in range(1, n + 1):
            ev(t0 + i * STEP, hi * (i / n) ** 2)
        ev(end, 0.0)
    elif shape == "rise":
        beats = min(length, float(lane.get("beats", 2.0)))
        n = int(beats / STEP)
        for i in range(n + 1):
            ev(start + i * STEP, lo * (1 - i / n))
    return out
