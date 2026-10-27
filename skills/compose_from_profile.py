#!/usr/bin/env python3
"""
Compose a song from one or more genre profiles → write composition.json + MIDI.

Usage:
    # Single genre, 8-bar loop (the default)
    python skills/compose_from_profile.py --genres lo-fi-hip-hop --title chillsong

    # Blend three genres with weights
    python skills/compose_from_profile.py \\
        --genres scenecore,witch-house,shoegaze \\
        --weights 0.5,0.3,0.2 \\
        --bars 16 --title hybrid_song

    # Override a layer with a hand-written composition fragment
    python skills/compose_from_profile.py --genres scenecore \\
        --override-bass plans/my-bassline.json --title custom_bass

    # Regenerate just one section
    python skills/compose_from_profile.py --genres scenecore \\
        --resume plans/song.composition.json \\
        --regenerate verse2.B --layers drums

    # Full song from the profile's form template, rendered for specific VSTs
    # (any VST works with any genre), one .mid per track for a DAW template
    python skills/compose_from_profile.py --genres metalcore --form \\
        --vst drums=superior-drummer-3,rhythm_guitar=odin-iii \\
        --split-tracks --title riffs

    # List available instrument maps
    python skills/compose_from_profile.py --list-vsts
"""

import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from midi_types.music import Composition, Track, Note
from composers.profile import load_profile, blend_profiles
from composers.pipeline import compose, regenerate_section
from skills.generate_midi import generate_midi


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--genres", default=None,
                   help="Comma-separated genre slugs from genres/. Aliases resolve.")
    p.add_argument("--weights", default=None,
                   help="Comma-separated weights matching --genres. Defaults to equal weight.")
    p.add_argument("--bars", type=int, default=8,
                   help="Total bars to compose. Default 8 (a single loop). With a profile form, "
                        "bars > 8 walks the form truncated to fit.")
    p.add_argument("--form", action="store_true",
                   help="Render the profile's complete form template (ignores --bars).")
    p.add_argument("--tuning", default=None,
                   help="Override the profile tuning (e.g. drop_b, standard_e, seven_b).")
    p.add_argument("--vst", default=None,
                   help="Comma-separated role=instrument-map pairs, e.g. "
                        "drums=superior-drummer-3,rhythm_guitar=odin-iii (role 'all' targets every track).")
    p.add_argument("--split-tracks", action="store_true",
                   help="Also write one .mid per track to output/<title>/.")
    p.add_argument("--list-vsts", action="store_true",
                   help="List instrument maps in instruments/ and exit.")
    p.add_argument("--bpm", type=int, default=None,
                   help="Override BPM. Defaults to midpoint of (blended) tempo range.")
    p.add_argument("--key", default=None,
                   help="Override key (e.g. 'D Dorian'). Defaults to profile.")
    p.add_argument("--title", default="composition",
                   help="Composition title (also output filename stem).")
    p.add_argument("--seed", type=int, default=None,
                   help="Deterministic seed.")
    p.add_argument("--layers", default=None,
                   help="Comma-separated layer roles to render: chords,pad,rhythm_guitar,drums,bass,melody,lead. "
                        "Default: all available.")
    p.add_argument("--override-rhythm-guitar", default=None, help="...rhythm guitar (kick/bass still lock to it).")
    p.add_argument("--override-pad", default=None, help="...pad.")
    p.add_argument("--override-chords", default=None, help="JSON file with a Track to use instead of generating chords.")
    p.add_argument("--override-drums", default=None, help="...drums.")
    p.add_argument("--override-bass", default=None, help="...bass.")
    p.add_argument("--override-melody", default=None, help="...melody.")
    p.add_argument("--override-lead", default=None, help="...lead.")
    p.add_argument("--resume", default=None,
                   help="Path to an existing composition.json to modify (paired with --regenerate).")
    p.add_argument("--regenerate", default=None,
                   help="Section name to re-render (e.g. 'verse2.B'). Requires --resume.")
    p.add_argument("--out", default=None,
                   help="Where to write composition.json. Defaults to plans/<title>.composition.json")
    p.add_argument("--no-midi", action="store_true",
                   help="Skip MIDI rendering (only emit composition.json).")
    return p.parse_args()


def load_track_override(path: str) -> Track:
    with open(path) as f:
        data = json.load(f)
    notes = [
        Note(
            pitch=str(n["pitch"]), duration=str(n["duration"]),
            velocity=n.get("velocity"), start=n.get("start"),
            articulation=n.get("articulation"), drum=n.get("drum"),
        )
        for n in data.get("notes", [])
    ]
    return Track(
        notes=notes, instrument=data.get("instrument"),
        is_drum=bool(data.get("is_drum", False)), name=data.get("name"),
        role=data.get("role"), vst=data.get("vst"), pan=data.get("pan"),
    )


def parse_vst(arg: str | None) -> dict:
    out = {}
    for pair in (arg or "").split(","):
        if "=" in pair:
            role, slug = pair.split("=", 1)
            out[role.strip().replace("-", "_")] = slug.strip()
    return out


def main() -> int:
    args = parse_args()

    if args.list_vsts:
        from midi_types.instrument_map import list_instrument_maps
        for m in list_instrument_maps():
            extra = f"  aliases: {', '.join(m.aliases)}" if m.aliases else ""
            print(f"{m.name:24s} {m.type:8s} {m.vendor or ''}{extra}")
        return 0
    if not args.genres:
        print("--genres is required", file=sys.stderr)
        return 2

    slugs = [s.strip() for s in args.genres.split(",") if s.strip()]
    if args.weights:
        weights = [float(w) for w in args.weights.split(",")]
        if len(weights) != len(slugs):
            print("--weights count must match --genres count", file=sys.stderr)
            return 2
    else:
        weights = [1.0] * len(slugs)

    profiles = [load_profile(s) for s in slugs]
    profile = blend_profiles(list(zip(profiles, weights))) if len(profiles) > 1 else profiles[0]
    if args.tuning:
        profile.tuning = args.tuning
    vst_overrides = parse_vst(args.vst)

    overrides = {}
    for role, arg in [("chords", args.override_chords), ("drums", args.override_drums),
                      ("bass", args.override_bass), ("melody", args.override_melody),
                      ("lead", args.override_lead), ("rhythm_guitar", args.override_rhythm_guitar),
                      ("pad", args.override_pad)]:
        if arg:
            overrides[role] = load_track_override(arg)

    layers = [l.strip() for l in args.layers.split(",")] if args.layers else None

    # Resume / regenerate path
    if args.resume:
        with open(args.resume) as f:
            base = Composition.from_dict(json.load(f))
        if args.regenerate:
            composition = regenerate_section(base, profile, args.regenerate, layers=layers,
                                             seed=args.seed, vst_overrides=vst_overrides)
        else:
            composition = base
        # On resume, default to keeping the original title and overwriting in place
        if args.title == "composition":
            args.title = base.title
        composition.title = args.title
        default_out = Path(args.resume)
    else:
        composition = compose(
            profile, title=args.title, bars=args.bars, bpm=args.bpm, key=args.key,
            seed=args.seed, overrides=overrides, layers=layers,
            use_form=args.form, vst_overrides=vst_overrides,
        )
        composition.title = args.title
        default_out = Path(__file__).parent.parent / "plans" / f"{args.title}.composition.json"

    out_path = Path(args.out) if args.out else default_out
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w") as f:
        json.dump(composition.to_dict(), f, indent=2)

    print(f"Wrote composition: {out_path}")
    print(f"  Genre(s): {'+'.join(slugs)}")
    print(f"  Tempo: {composition.bpm} BPM   Key: {composition.key}   Bars: {composition.bars}")
    print(f"  Sections: {', '.join(s.name for s in composition.sections)}")
    print(f"  Tracks: {', '.join((t.name or '?') + (f' [{t.vst}]' if t.vst else '') for t in composition.tracks)} "
          f"({sum(len(t.notes) for t in composition.tracks)} notes)")

    if not args.no_midi:
        midi_path = generate_midi(composition, split_tracks=args.split_tracks)
        print(f"Wrote MIDI: {midi_path}")
        if args.split_tracks:
            print(f"  Per-track files: {Path(midi_path).with_suffix('')}/")

    return 0


if __name__ == "__main__":
    sys.exit(main())
