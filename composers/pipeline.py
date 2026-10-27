"""
Lego Pipeline orchestrator.

Runs composers in order — each layer 'hears' all previous layers:

    structure → harmony timeline → [layer_order]

The default layer order is

    chords → pad → rhythm_guitar → drums → electronic_drums → bass → synth_bass → melody → lead

and a profile can reorder it (`layer_order`). Riff-driven genres put
rhythm_guitar first so the kick (kick_follow) and the bass (style
'doubled') lock to the riff, the way a metalcore band actually works.

Callers can override any layer by passing a pre-built Track in
`overrides` — the pipeline skips that composer and uses the supplied
track instead (later layers still hear it). After composing, each track
is tagged with its role and target instrument map (profile
`instruments[].vst`, or `vst_overrides` from the CLI — any VST, any
genre), given its section automation (filter sweeps, sidechain pumps,
wobbles, pitch dives), split into takes where the role is double/quad
tracked (composers/doubling.py), and humanized per take. Section `tempo` /
`tempo_scale` bindings become the composition's tempo map.
"""

from typing import Dict, Optional, List
import random

from midi_types.music import Composition, Track, Note, Section
from midi_types.genre_profile import GenreProfile

from .structure import plan_sections
from .harmony import build_timeline
from .chords import generate_chords
from .riff import generate_riff, Attack
from .drums import generate_drums
from .bass import generate_bass
from .melody import generate_melody
from .automation import apply_automation
from .doubling import make_takes
from .profile import get_instrument


DEFAULT_LAYER_ORDER = ["chords", "pad", "rhythm_guitar", "drums", "electronic_drums",
                       "bass", "synth_bass", "melody", "lead"]
LAYER_ORDER = DEFAULT_LAYER_ORDER  # backwards-compatible name


def layer_order(profile: GenreProfile) -> List[str]:
    order = [r for r in profile.layer_order if r in DEFAULT_LAYER_ORDER]
    return order + [r for r in DEFAULT_LAYER_ORDER if r not in order]


def compose(
    profile: GenreProfile,
    title: str = "untitled",
    bars: Optional[int] = 8,
    bpm: Optional[int] = None,
    key: Optional[str] = None,
    seed: Optional[int] = None,
    overrides: Optional[Dict[str, Track]] = None,
    layers: Optional[List[str]] = None,
    sections: Optional[List[Section]] = None,
    use_form: bool = False,
    vst_overrides: Optional[Dict[str, str]] = None,
    context: Optional[Dict[str, Track]] = None,
) -> Composition:
    """Build a full Composition by running each layer in order.

    Args:
        profile: GenreProfile to compose from (already blended if multi-genre).
        title: composition title.
        bars: total bar count. Default 8 — the typical loop length. With a
            profile `form`, bars > 8 walks the form (truncated to fit).
        bpm: override the profile's tempo (otherwise picks midpoint of range).
        key: override the profile's default_key.
        seed: deterministic randomization seed. Each layer gets its own
            stream, so regenerating one layer never reshuffles another.
        overrides: pre-built tracks keyed by role. The pipeline skips that
            composer and uses the override. This is how user ideas get in.
        layers: which roles to render (default: all roles defined on the profile).
        sections: pre-planned sections (used by regenerate_section).
        use_form: render the profile's complete form template.
        vst_overrides: {role: instrument-map slug} — target any VST in any genre.
        context: existing tracks keyed by role that later layers should hear
            but that are not re-rendered or returned (regenerate_section uses
            this so regenerated drums still follow the existing riff).
    """
    overrides = overrides or {}
    context = context or {}
    vst_overrides = vst_overrides or {}

    def stream(name: str) -> random.Random:
        return random.Random(f"{seed}:{name}") if seed is not None else random.Random()

    chosen_bpm = bpm if bpm is not None else (profile.tempo_range[0] + profile.tempo_range[1]) // 2
    chosen_key = key or profile.default_key

    if sections is None:
        sections = plan_sections(profile, None if use_form else bars, rng=stream("structure"),
                                 use_form=use_form)
    timeline, _ = build_timeline(profile, sections, chosen_key, stream("harmony"))

    available_roles = {i.role for i in profile.instruments} | set(overrides)
    if layers is None:
        layers = [r for r in layer_order(profile) if r in available_roles]

    built: Dict[str, Track] = {}
    attacks: Optional[List[Attack]] = None

    for role in layer_order(profile):
        if role not in layers and role in context:
            built[role] = context[role]
            if role == "rhythm_guitar":
                attacks = _attacks_from_track(context[role])
            continue
        if role not in layers or role not in available_roles:
            continue
        rng = stream(role)
        if role in overrides:
            track = overrides[role]
            if role == "rhythm_guitar":
                attacks = _attacks_from_track(track)
        elif role in ("chords", "pad"):
            track = generate_chords(profile, sections, timeline, key=chosen_key, rng=rng, role=role,
                                    riff_attacks=attacks)
        elif role == "rhythm_guitar":
            track, attacks = generate_riff(profile, sections, timeline, key=chosen_key, rng=rng)
        elif role in ("drums", "electronic_drums"):
            track = generate_drums(profile, sections, rng=rng, riff_attacks=attacks,
                                   layer="electronic" if role == "electronic_drums" else "acoustic")
        elif role in ("bass", "synth_bass"):
            track = generate_bass(profile, sections, timeline, drum_track=built.get("drums"),
                                  key=chosen_key, rng=rng, riff_track=built.get("rhythm_guitar"),
                                  role=role)
        else:  # melody / lead
            track = generate_melody(profile, sections, timeline, drum_track=built.get("drums"),
                                    key=chosen_key, role=role, rng=rng,
                                    riff_track=built.get("rhythm_guitar"))
        track.role = track.role or role
        if track.notes or role in overrides:
            built[role] = track

    rendered = [r for r in layer_order(profile)
                if r in built and not (r in context and r not in layers)]
    apply_automation([built[r] for r in rendered if r not in overrides], sections)

    tracks: List[Track] = []
    for role in rendered:
        track = built[role]
        inst = get_instrument(profile, role)
        track.vst = vst_overrides.get(role) or vst_overrides.get("all") or track.vst or (inst.vst if inst else None)
        if role in overrides:
            tracks.append(track)
            continue
        if inst is not None and inst.double_track:
            takes = make_takes(track, inst.double_track, sections, timeline, chosen_key,
                               inst.register, stream(f"takes:{role}"))
            for take in takes:
                side = take.name.rsplit(" ", 1)[-1]
                tracks.append(_humanize(take, profile, stream(f"humanize:{role}:{side}"),
                                        minimum=(0.012, 0.05)))
        else:
            tracks.append(_humanize(track, profile, stream(f"humanize:{role}")))

    total_bars = max((s.end_bar for s in sections), default=bars or 0)
    return Composition(
        title=title,
        bpm=chosen_bpm,
        tracks=tracks,
        bars=total_bars,
        sections=sections,
        key=chosen_key,
        tuning=_tuning_name(profile.tuning),
        tempo_map=tempo_map(sections, chosen_bpm),
    )


def tempo_map(sections: List[Section], bpm: float) -> List[tuple]:
    """Section `tempo` (absolute) / `tempo_scale` (x song tempo) → [(beat, bpm)].
    A change is written at the section start and the song tempo is restored
    after it, so a slowed breakdown snaps back for the next chorus."""
    out: List[tuple] = []
    current = bpm
    for sec in sorted((s for s in sections if not s.is_micro), key=lambda s: s.start_bar):
        b = sec.bindings
        want = b.get("tempo") or (bpm * b["tempo_scale"] if b.get("tempo_scale") else bpm)
        want = round(float(want), 2)
        if abs(want - current) > 1e-6:
            out.append((sec.start_beat(), want))
            current = want
    return out


def regenerate_section(
    composition: Composition,
    profile: GenreProfile,
    section_name: str,
    layers: Optional[List[str]] = None,
    seed: Optional[int] = None,
    vst_overrides: Optional[Dict[str, str]] = None,
) -> Composition:
    """Re-render only the notes inside `section_name` (e.g. 'verse2.B') for the
    requested layers. Notes outside the section are preserved, and the
    section keeps its bindings (drum_pattern, energy, mute, ...).

    This is what makes 'add a drum break in verse 2 B' a one-line operation.
    """
    target = next((s for s in composition.sections if s.name == section_name), None)
    if target is None:
        raise ValueError(f"Section '{section_name}' not found in composition")

    def rebase(s: Section) -> Section:
        return Section(name=s.name, start_bar=s.start_bar - target.start_bar + 1,
                       end_bar=s.end_bar - target.start_bar + 1,
                       density=s.density, bindings=dict(s.bindings))

    bars_in = target.end_bar - target.start_bar + 1
    if target.is_micro:
        sub_sections = [Section(name=target.name.partition(".")[0], start_bar=1, end_bar=bars_in,
                                density=target.density, bindings=dict(target.bindings)),
                        rebase(target)]
    else:
        sub_sections = [rebase(target)] + [rebase(s) for s in composition.sections
                                           if s.name.startswith(target.name + ".")]

    shift = (target.start_bar - 1) * 4  # beats
    layer_set = set(layers) if layers else None

    # Layers we keep still inform the ones we regenerate (kick_follow hears the riff, etc.)
    context: Dict[str, Track] = {}
    if layer_set is not None:
        for t in composition.tracks:
            role = _track_role(t)
            if role in layer_set or role in context:
                continue
            window = [Note(n.pitch, n.duration, n.velocity, round(n.start - shift, 4), n.articulation, n.drum)
                      for n in t.notes if n.start is not None and shift <= n.start < shift + bars_in * 4]
            context[role] = Track(notes=window, instrument=t.instrument, is_drum=t.is_drum,
                                  name=t.name, role=role)

    sub = compose(
        profile, title=composition.title, bars=bars_in, bpm=composition.bpm,
        key=composition.key, seed=seed, layers=layers, sections=sub_sections,
        vst_overrides=vst_overrides, context=context,
    )

    new_tracks: List[Track] = []
    for original in composition.tracks:
        role = _track_role(original)
        if layer_set is not None and role not in layer_set:
            new_tracks.append(original)
            continue
        kept = [
            n for n in original.notes
            if n.start is None or not (shift <= n.start < shift + bars_in * 4)
        ]
        sub_track = (next((t for t in sub.tracks if t.name == original.name), None)
                     or next((t for t in sub.tracks if _track_role(t) == role), None))
        if sub_track:
            for n in sub_track.notes:
                if n.start is not None:
                    kept.append(Note(pitch=n.pitch, duration=n.duration, velocity=n.velocity,
                                     start=round(n.start + shift, 4),
                                     articulation=n.articulation, drum=n.drum))
        kept.sort(key=lambda n: n.start if n.start is not None else -1)
        new_tracks.append(Track(
            notes=kept, instrument=original.instrument, is_drum=original.is_drum,
            name=original.name, role=original.role, vst=original.vst, pan=original.pan,
            automation=original.automation,
        ))

    return Composition(
        title=composition.title, bpm=composition.bpm, tracks=new_tracks,
        bars=composition.bars, sections=composition.sections,
        time_signature=composition.time_signature, key=composition.key,
        tuning=composition.tuning, tempo_map=composition.tempo_map,
    )


# -------- helpers --------

def _track_role(track: Track) -> str:
    if track.role:
        return track.role
    if track.is_drum:
        return "drums"
    name = (track.name or "").lower()
    for role in ("rhythm guitar", "electronic drums", "synth bass", "chords", "bass", "melody",
                 "lead", "pad"):
        if role in name:
            return role.replace(" ", "_")
    return "melody"


def _attacks_from_track(track: Track) -> List[Attack]:
    from skills.generate_midi import parse_pitch
    lowest: Dict[float, int] = {}
    for n in track.notes:
        if n.start is None:
            continue
        p = parse_pitch(n.pitch)
        lowest[n.start] = min(p, lowest.get(n.start, 128))
    floor = min(lowest.values(), default=0)
    return [Attack(t, 0.5, p <= floor + 7, False) for t, p in sorted(lowest.items())]


def _humanize(track: Track, profile: GenreProfile, rng: random.Random,
              minimum=(0.0, 0.0)) -> Track:
    timing = max(profile.humanize.timing, minimum[0])
    vel = max(profile.humanize.velocity, minimum[1])
    if track.is_drum:
        timing *= 0.5
    if timing <= 0 and vel <= 0:
        return track
    for n in track.notes:
        if n.start is not None and timing > 0 and n.start > 0:
            n.start = round(max(0.0, n.start + rng.uniform(-timing, timing)), 4)
        if n.velocity is not None and vel > 0:
            n.velocity = round(max(0.05, min(1.0, n.velocity + rng.uniform(-vel, vel))), 3)
    return track


def _tuning_name(tuning) -> Optional[str]:
    if tuning is None:
        return None
    if isinstance(tuning, dict):
        return tuning.get("name") or "custom"
    if isinstance(tuning, (list, tuple)):
        return "custom"
    return str(tuning)
