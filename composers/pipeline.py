"""
Lego Pipeline orchestrator.

Runs composers in the canonical order — each layer 'hears' all previous layers:

    structure → chords → drums → bass (locks to drums) → melody (reads drums + chords)

Returns a Composition. Callers can override any layer by passing a
pre-built Track in `overrides` — the pipeline will skip that composer and
use the supplied track instead. This is how user-provided ideas get
incorporated into the song.
"""

from typing import Dict, Optional, List
import random

from midi_types.music import Composition, Track, Section
from midi_types.genre_profile import GenreProfile

from .structure import plan_sections
from .chords import generate_chords
from .drums import generate_drums
from .bass import generate_bass
from .melody import generate_melody


LAYER_ORDER = ["chords", "drums", "bass", "melody", "lead"]


def compose(
    profile: GenreProfile,
    title: str = "untitled",
    bars: int = 8,
    bpm: Optional[int] = None,
    key: Optional[str] = None,
    seed: Optional[int] = None,
    overrides: Optional[Dict[str, Track]] = None,
    layers: Optional[List[str]] = None,
) -> Composition:
    """Build a full Composition by running each layer in order.

    Args:
        profile: GenreProfile to compose from (already blended if multi-genre).
        title: composition title.
        bars: total bar count. Default 8 — the typical loop length.
        bpm: override the profile's tempo (otherwise picks midpoint of range).
        key: override the profile's default_key.
        seed: deterministic randomization seed.
        overrides: pre-built tracks keyed by role ('chords', 'drums', 'bass',
            'melody', 'lead'). The pipeline skips that composer and uses the
            override. This is how user ideas get incorporated.
        layers: which roles to render (default: all roles defined on the profile).
    """
    rng = random.Random(seed)
    overrides = overrides or {}

    chosen_bpm = bpm if bpm is not None else (profile.tempo_range[0] + profile.tempo_range[1]) // 2
    chosen_key = key or profile.default_key

    sections = plan_sections(profile, bars)

    available_roles = {i.role for i in profile.instruments}
    if layers is None:
        # Default order: only render what the profile defines
        layers = [r for r in LAYER_ORDER if r in available_roles]

    tracks: List[Track] = []
    chord_plan = []

    if "chords" in layers:
        if "chords" in overrides:
            chord_track = overrides["chords"]
            # We don't have the original plan; downstream composers will fall back
        else:
            chord_track, chord_plan = generate_chords(profile, sections, key=chosen_key, rng=rng)
        if chord_track.notes or "chords" in overrides:
            tracks.append(chord_track)

    drum_track: Optional[Track] = None
    if "drums" in layers:
        if "drums" in overrides:
            drum_track = overrides["drums"]
        else:
            drum_track = generate_drums(profile, sections, rng=rng)
        if drum_track.notes:
            tracks.append(drum_track)

    if "bass" in layers:
        if "bass" in overrides:
            tracks.append(overrides["bass"])
        else:
            bass_track = generate_bass(
                profile, sections, chord_plan, drum_track=drum_track,
                key=chosen_key, rng=rng,
            )
            if bass_track.notes:
                tracks.append(bass_track)

    if "melody" in layers:
        if "melody" in overrides:
            tracks.append(overrides["melody"])
        else:
            mel = generate_melody(profile, sections, chord_plan, drum_track=drum_track,
                                  key=chosen_key, role="melody", rng=rng)
            if mel.notes:
                tracks.append(mel)

    if "lead" in layers and "lead" in available_roles:
        if "lead" in overrides:
            tracks.append(overrides["lead"])
        else:
            lead = generate_melody(profile, sections, chord_plan, drum_track=drum_track,
                                   key=chosen_key, role="lead", rng=rng)
            if lead.notes:
                tracks.append(lead)

    return Composition(
        title=title,
        bpm=chosen_bpm,
        tracks=tracks,
        bars=bars,
        sections=sections,
        key=chosen_key,
    )


def regenerate_section(
    composition: Composition,
    profile: GenreProfile,
    section_name: str,
    layers: Optional[List[str]] = None,
    seed: Optional[int] = None,
) -> Composition:
    """Re-render only the notes inside `section_name` (e.g. 'verse2.B') for the
    requested layers. Notes outside the section are preserved.

    This is what makes 'add a drum break in verse 2 B' a one-line operation.
    """
    target = next((s for s in composition.sections if s.name == section_name), None)
    if target is None:
        raise ValueError(f"Section '{section_name}' not found in composition")

    bars_in = target.end_bar - target.start_bar + 1
    sub_sections = [Section(name=target.name, start_bar=1, end_bar=bars_in)]
    # Include any micro children of the target, rebased to start at bar 1
    for s in composition.sections:
        if s.name.startswith(target.name + "."):
            sub_sections.append(Section(
                name=s.name,
                start_bar=s.start_bar - target.start_bar + 1,
                end_bar=s.end_bar - target.start_bar + 1,
            ))

    sub = compose(
        profile, title=composition.title, bars=bars_in, bpm=composition.bpm,
        key=composition.key, seed=seed, layers=layers,
    )

    # Time-shift sub.notes back to absolute song position
    shift = (target.start_bar - 1) * 4  # beats
    layer_set = set(layers) if layers else None

    new_tracks: List[Track] = []
    for original in composition.tracks:
        role = _track_role(original)
        if layer_set is not None and role not in layer_set:
            new_tracks.append(original)
            continue
        # Drop any notes inside [target.start_bar, target.end_bar] from this track
        kept = [
            n for n in original.notes
            if n.start is None or not (shift <= n.start < shift + bars_in * 4)
        ]
        # Append shifted notes from the sub-render's matching track
        sub_track = next((t for t in sub.tracks if _track_role(t) == role), None)
        if sub_track:
            for n in sub_track.notes:
                if n.start is not None:
                    kept.append(type(n)(
                        pitch=n.pitch, duration=n.duration,
                        velocity=n.velocity, start=round(n.start + shift, 4),
                    ))
        new_tracks.append(type(original)(
            notes=kept, instrument=original.instrument,
            is_drum=original.is_drum, name=original.name,
        ))

    return Composition(
        title=composition.title, bpm=composition.bpm, tracks=new_tracks,
        bars=composition.bars, sections=composition.sections,
        time_signature=composition.time_signature, key=composition.key,
    )


def _track_role(track: Track) -> str:
    if track.is_drum:
        return "drums"
    name = (track.name or "").lower()
    for role in ("chords", "bass", "melody", "lead", "pad"):
        if role in name:
            return role
    return "melody"
