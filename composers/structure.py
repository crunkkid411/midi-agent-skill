"""
Structure planner — turns a genre profile + requested bar count into a
concrete list of Section objects with bar offsets.

Default behavior is loop-oriented: 8 bars produces a single addressable
loop section with A/B micro-sections. Longer requests get expanded by
chaining the genre's default song sections; even longer ones repeat the
loop with numbered suffixes (verse1, verse2, ...).
"""

from typing import List
from dataclasses import dataclass

from midi_types.music import Section
from midi_types.genre_profile import GenreProfile, SongSection


def plan_sections(profile: GenreProfile, total_bars: int) -> List[Section]:
    """Map total_bars to a flat list of Sections using the profile's structure.

    For total_bars <= 8: emit a single 'loop' section, expanded with
    micro-sections from the profile's first non-intro section.

    For larger requests: chain profile.song_sections in order, looping the
    middle sections (verse/chorus/loop) with numeric suffixes until the bar
    budget is filled. Intro/outro are added once at the boundaries.
    """
    if total_bars <= 0:
        return []

    sections = profile.song_sections
    if not sections:
        return [Section(name="loop", start_bar=1, end_bar=total_bars)]

    # Short loop case: just one section, with micro
    if total_bars <= 8:
        loop_template = next(
            (s for s in sections if s.name in ("loop", "verse", "drop", "chorus")),
            sections[0],
        )
        result = [Section(name=loop_template.name, start_bar=1, end_bar=total_bars)]
        result.extend(_expand_micro(loop_template, start_bar=1, total_bars=total_bars))
        return result

    # Multi-section case
    intros = [s for s in sections if s.name == "intro"]
    outros = [s for s in sections if s.name == "outro"]
    middles = [s for s in sections if s.name not in ("intro", "outro")]
    if not middles:
        middles = sections

    result: List[Section] = []
    cursor = 1
    remaining = total_bars

    # Intro
    if intros and remaining >= intros[0].bars + 4:
        intro = intros[0]
        result.append(Section(name="intro", start_bar=cursor, end_bar=cursor + intro.bars - 1))
        result.extend(_expand_micro(intro, cursor, intro.bars))
        cursor += intro.bars
        remaining -= intro.bars

    # Reserve outro
    outro_bars = outros[0].bars if outros else 0
    middle_budget = remaining - outro_bars
    if middle_budget < 1:
        middle_budget = remaining
        outro_bars = 0

    # Fill middle by cycling through middle sections with suffix numbering
    counts = {s.name: 0 for s in middles}
    i = 0
    while middle_budget > 0:
        s = middles[i % len(middles)]
        counts[s.name] += 1
        bars_to_use = min(s.bars, middle_budget)
        suffix = str(counts[s.name]) if sum(1 for m in middles if m.name == s.name) == 1 and counts[s.name] > 1 else (str(counts[s.name]) if counts[s.name] > 1 else "")
        section_name = f"{s.name}{suffix}" if suffix else s.name
        result.append(Section(name=section_name, start_bar=cursor, end_bar=cursor + bars_to_use - 1))
        result.extend(_expand_micro(s, cursor, bars_to_use, name_override=section_name))
        cursor += bars_to_use
        middle_budget -= bars_to_use
        i += 1

    # Outro
    if outro_bars > 0 and outros:
        outro = outros[0]
        result.append(Section(name="outro", start_bar=cursor, end_bar=cursor + outro_bars - 1))
        result.extend(_expand_micro(outro, cursor, outro_bars))

    return result


def _expand_micro(template: SongSection, start_bar: int, total_bars: int,
                  name_override: str = None) -> List[Section]:
    """Emit Section entries for each micro-section, scaled if necessary."""
    if not template.micro:
        return []
    parent_name = name_override or template.name
    declared = sum(m.bars for m in template.micro)
    if declared == 0:
        return []
    scale = total_bars / declared if declared != total_bars else 1.0

    out = []
    cursor = start_bar
    for m in template.micro:
        bars = max(1, round(m.bars * scale))
        end = min(start_bar + total_bars - 1, cursor + bars - 1)
        if cursor > end:
            break
        out.append(Section(
            name=f"{parent_name}.{m.name}",
            start_bar=cursor,
            end_bar=end,
        ))
        cursor = end + 1
    return out


def section_density(profile: GenreProfile, section: Section) -> float:
    """Look up the density value for a Section by matching against the profile's
    micro-section catalog. Falls back to parent section density, then 0.7."""
    parent_name, _, micro_name = section.name.partition(".")
    # Strip numeric suffix from parent ("verse2" → "verse")
    parent_base = parent_name.rstrip("0123456789") or parent_name

    for s in profile.song_sections:
        if s.name == parent_base:
            if micro_name:
                for m in s.micro:
                    if m.name == micro_name:
                        return m.density
            return s.density
    return 0.7
