"""
Structure planner — turns a genre profile + requested bar count into a
concrete list of Section objects with bar offsets, densities and resolved
bindings (energy, drum_pattern, progression, scale, mute, stop, ...).

Three modes:
  - total_bars <= 8 (no form requested): a single addressable loop section
    with the profile's A/B micro-sections.
  - profile.form present (or use_form=True): walk the form template in
    order. Optional slots (probability < 1) are rolled with the seed.
    Repeated sections are numbered (verse, verse2, chorus3). When a bar
    budget is given the form is truncated to fit; use_form renders it whole.
  - no form: chain profile.song_sections, cycling the middle sections with
    numeric suffixes until the bar budget is filled (legacy behaviour).

Every Section's `bindings` is the merge of song section → form-slot
overrides → micro-section, so composers only ever read `section.bindings`.
"""

from typing import Any, Dict, List, Optional
import random

from midi_types.music import Section
from midi_types.genre_profile import (
    GenreProfile, SongSection, SECTION_BINDING_KEYS,
)


def plan_sections(profile: GenreProfile, total_bars: Optional[int],
                  rng: Optional[random.Random] = None, use_form: bool = False) -> List[Section]:
    """Map total_bars to a flat list of Sections using the profile's structure.

    total_bars=None with use_form=True renders the full form template.
    """
    return _apply_play_in(profile, _plan(profile, total_bars, rng or random.Random(), use_form))


def _apply_play_in(profile: GenreProfile, sections: List[Section]) -> List[Section]:
    """Roles with instruments[].play_in are muted in every other section."""
    limits = {i.role: set(i.play_in) for i in profile.instruments if i.play_in}
    for sec in sections:
        extra = [r for r, allowed in limits.items() if sec.base not in allowed]
        if extra:
            mute = list(sec.bindings.get("mute") or [])
            sec.bindings["mute"] = mute + [r for r in extra if r not in mute]
    return sections


def _plan(profile: GenreProfile, total_bars: Optional[int], rng: random.Random,
          use_form: bool) -> List[Section]:
    sections = profile.song_sections
    if not sections:
        bars = total_bars or 8
        return [Section(name="loop", start_bar=1, end_bar=bars, density=0.7)]

    if use_form or (profile.form and total_bars and total_bars > 8):
        return _plan_from_form(profile, None if use_form else total_bars, rng)

    total_bars = total_bars or 8
    if total_bars <= 0:
        return []

    # Short loop case: just one section, with micro
    if total_bars <= 8:
        loop_template = next(
            (s for s in sections if s.name in ("loop", "verse", "drop", "chorus")),
            sections[0],
        )
        return _emit(loop_template, loop_template.name, 1, total_bars, {})

    # Multi-section case
    intros = [s for s in sections if s.name == "intro"]
    outros = [s for s in sections if s.name == "outro"]
    middles = [s for s in sections if s.name not in ("intro", "outro")]
    if not middles:
        middles = sections

    result: List[Section] = []
    cursor = 1
    remaining = total_bars

    if intros and remaining >= intros[0].bars + 4:
        intro = intros[0]
        result.extend(_emit(intro, "intro", cursor, intro.bars, {}))
        cursor += intro.bars
        remaining -= intro.bars

    # Reserve outro
    outro_bars = outros[0].bars if outros else 0
    middle_budget = remaining - outro_bars
    if middle_budget < 1:
        middle_budget = remaining
        outro_bars = 0

    # Fill middle by cycling through middle sections with suffix numbering
    counts: Dict[str, int] = {}
    i = 0
    while middle_budget > 0:
        s = middles[i % len(middles)]
        counts[s.name] = counts.get(s.name, 0) + 1
        bars_to_use = min(s.bars, middle_budget)
        section_name = s.name if counts[s.name] == 1 else f"{s.name}{counts[s.name]}"
        result.extend(_emit(s, section_name, cursor, bars_to_use, {}))
        cursor += bars_to_use
        middle_budget -= bars_to_use
        i += 1

    if outro_bars > 0 and outros:
        result.extend(_emit(outros[0], "outro", cursor, outro_bars, {}))

    return result


def _plan_from_form(profile: GenreProfile, budget: Optional[int],
                    rng: random.Random) -> List[Section]:
    by_name = {s.name: s for s in profile.song_sections}
    result: List[Section] = []
    counts: Dict[str, int] = {}
    cursor = 1
    for entry in profile.form:
        template = by_name.get(entry.section)
        if template is None:
            raise ValueError(f"form references unknown section '{entry.section}'")
        if entry.probability < 1.0 and rng.random() >= entry.probability:
            continue
        bars = entry.bars or template.bars
        if budget is not None:
            left = budget - cursor + 1
            if left <= 0:
                break
            bars = min(bars, left)
        counts[template.name] = counts.get(template.name, 0) + 1
        n = counts[template.name]
        name = template.name if n == 1 else f"{template.name}{n}"
        result.extend(_emit(template, name, cursor, bars, entry.overrides))
        cursor += bars
    return result


def _emit(template: SongSection, name: str, start_bar: int, bars: int,
          overrides: Dict[str, Any]) -> List[Section]:
    parent_bindings = _merge(_bindings_of(template), overrides)
    top = Section(name=name, start_bar=start_bar, end_bar=start_bar + bars - 1,
                  density=template.density, bindings=parent_bindings)
    return [top] + _expand_micro(template, start_bar, bars, name, parent_bindings)


def _expand_micro(template: SongSection, start_bar: int, total_bars: int,
                  parent_name: str, parent_bindings: Dict[str, Any]) -> List[Section]:
    """Emit Section entries for each micro-section, scaled if necessary.

    Micro-sections are dropped (the parent plays through) when the parent
    was truncated below the number of micro blocks it declares — 1-bar
    slivers of an A/B split are worse than no split."""
    if not template.micro:
        return []
    declared = sum(m.bars for m in template.micro)
    if declared == 0 or total_bars < len(template.micro):
        return []
    scale = total_bars / declared

    out = []
    cursor = start_bar
    last = start_bar + total_bars - 1
    for idx, m in enumerate(template.micro):
        bars = max(1, round(m.bars * scale))
        end = last if idx == len(template.micro) - 1 else min(last, cursor + bars - 1)
        if cursor > end:
            break
        out.append(Section(
            name=f"{parent_name}.{m.name}", start_bar=cursor, end_bar=end,
            density=m.density, bindings=_merge(parent_bindings, _bindings_of(m)),
        ))
        cursor = end + 1
    return out


def _bindings_of(obj) -> Dict[str, Any]:
    out = {}
    for k in SECTION_BINDING_KEYS:
        v = getattr(obj, k, None)
        if v is None or v == []:
            continue
        out[k] = v
    return out


def _merge(parent: Dict[str, Any], child: Dict[str, Any]) -> Dict[str, Any]:
    out = {k: (list(v) if isinstance(v, list) else v) for k, v in parent.items()}
    out.update({k: v for k, v in child.items() if v is not None and v != []})
    return out


# -------- Queries used by every composer --------

def top_sections(sections: List[Section]) -> List[Section]:
    return [s for s in sections if not s.is_micro]


def leaf_sections(sections: List[Section]) -> List[Section]:
    """Micro-sections where they exist, otherwise the top-level section.
    This is the granularity composers render at, so verse.B can differ
    from verse.A."""
    out: List[Section] = []
    for top in top_sections(sections):
        kids = [s for s in sections if s.name.startswith(top.name + ".")]
        out.extend(kids or [top])
    return out


def section_density(profile: GenreProfile, section: Section) -> float:
    """Density for a Section: the planner's resolved value when present,
    otherwise looked up in the profile's micro-section catalog. Falls back
    to parent section density, then 0.7."""
    if section.density is not None:
        return section.density
    parent_name, _, micro_name = section.name.partition(".")
    parent_base = parent_name.rstrip("0123456789") or parent_name

    for s in profile.song_sections:
        if s.name == parent_base:
            if micro_name:
                for m in s.micro:
                    if m.name == micro_name:
                        return m.density
            return s.density
    return 0.7


def section_energy(profile: GenreProfile, section: Section) -> float:
    """Energy (how hard) defaults to density (how busy) when unset."""
    e = section.bindings.get("energy")
    return float(e) if e is not None else section_density(profile, section)


def is_muted(section: Section, role: str) -> bool:
    return role in (section.bindings.get("mute") or [])


def is_stop(section: Section) -> bool:
    return bool(section.bindings.get("stop"))
