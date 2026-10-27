"""
Pipeline tests — run with:  python -m unittest discover -s tests -v

Covers the riff-first features (power chords, riff composer, layer order,
kick/bass locking, breakdown/stop sections, drum lanes + fills, motif
repetition, forms, tuning, per-section scale), instrument-map rendering
(Superior Drummer 3, Odin III) and backwards compatibility.
"""

import bisect
import copy
import json
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).parent.parent
sys.path.insert(0, str(ROOT))

from composers.pipeline import compose, regenerate_section, layer_order
from composers.profile import load_profile
from composers.theory import voice_chord, parse_key, lowest_string
from composers.harmony import build_timeline
from composers.structure import plan_sections
from midi_types.genre_profile import Humanize
from midi_types.instrument_map import load_instrument_map, InstrumentMap
from midi_types.music import Composition
from midi_types.gm_instruments import resolve_instrument
from skills.generate_midi import (generate_midi, generate_midi_from_dict, parse_pitch, _keyswitch_events,
                                  _untangle, _phrase_events)

GENRES = sorted(p.stem for p in (ROOT / "genres").glob("*.json") if not p.name.startswith("_"))


def dry(profile):
    """Profile copy with humanization off, so timing assertions are exact."""
    p = copy.deepcopy(profile)
    p.humanize = Humanize()
    for i in p.instruments:
        i.double_track = False
    return p


def near(xs, ys, tol=0.03):
    ys = sorted(ys)
    hits = 0
    for x in xs:
        i = bisect.bisect_left(ys, x - tol)
        hits += i < len(ys) and ys[i] <= x + tol
    return hits / max(1, len(xs))


def track(comp, role):
    return next(t for t in comp.tracks if t.role == role)


def notes_in(t, section, beats_per_bar=4, slack=0.03):
    """Notes starting in the section. `slack` absorbs humanize jitter that
    nudges a downbeat a few ms before its section starts."""
    a, b = section.start_beat(beats_per_bar) - slack, section.end_beat(beats_per_bar) - slack
    return [n for n in t.notes if a <= n.start < b]


class TestProfiles(unittest.TestCase):
    def test_schema(self):
        try:
            import jsonschema
        except ImportError:
            self.skipTest("jsonschema not installed")
        schema = json.loads((ROOT / "genres/_schema.json").read_text())
        for g in GENRES:
            with self.subTest(genre=g):
                jsonschema.validate(json.loads((ROOT / f"genres/{g}.json").read_text()), schema)

    def test_every_genre_uses_the_granular_features(self):
        for g in GENRES:
            p = load_profile(g)
            with self.subTest(genre=g):
                self.assertTrue(p.form, "form template")
                self.assertTrue(p.fills, "fills")
                self.assertTrue(p.motif, "motif")
                self.assertGreaterEqual(len(p.drum_patterns), 4)
                self.assertTrue(any(s.drum_pattern for s in p.song_sections) or
                                any(m.drum_pattern for s in p.song_sections for m in s.micro))
                self.assertTrue(any(s.progression for s in p.song_sections))
                self.assertTrue(all(cp.name for cp in p.chord_progressions))
                self.assertTrue(any(s.automation for s in p.song_sections) or
                                any(m.automation for s in p.song_sections for m in s.micro),
                                "automation (filter sweeps / pumps / risers)")

    def test_every_genre_renders_loop_and_form_deterministically(self):
        for g in GENRES:
            p = load_profile(g)
            with self.subTest(genre=g):
                a = compose(p, bars=8, seed=3).to_dict()
                b = compose(p, bars=8, seed=3).to_dict()
                self.assertEqual(a, b)
                full = compose(p, use_form=True, seed=3)
                self.assertGreater(full.bars, 16)
                self.assertTrue(all(t.notes for t in full.tracks))

    def test_lofi_bass_is_a_bass(self):
        p = load_profile("lo-fi-hip-hop")
        bass = next(i for i in p.instruments if i.role == "bass")
        self.assertEqual(resolve_instrument(bass.gm), 33)


class TestTheory(unittest.TestCase):
    def test_power_chords_and_root_position(self):
        pc, scale = parse_key("C minor")
        self.assertEqual(voice_chord("i", pc, scale, (36, 60), "power_chords", low_root=36), [36, 43, 48])
        v = voice_chord("VI", pc, scale, (55, 76), "triads")
        self.assertEqual(v[0] % 12, 8)               # Ab is the lowest note
        self.assertEqual(v, sorted(v))

    def test_tunings(self):
        self.assertEqual(lowest_string("drop_c"), 36)
        self.assertEqual(lowest_string("Drop B"), 35)
        self.assertEqual(lowest_string([30, 35, 40]), 30)


class TestMetalcore(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.profile = dry(load_profile("metalcore"))
        cls.comp = compose(cls.profile, use_form=True, seed=11)
        cls.sections = {s.name: s for s in cls.comp.sections}

    def test_layer_order_is_riff_first(self):
        self.assertEqual(layer_order(self.profile)[0], "rhythm_guitar")
        self.assertEqual(self.comp.tracks[0].role, "rhythm_guitar")

    def test_form_has_stop_breakdown_and_final_chorus(self):
        names = [s.name for s in self.comp.sections if "." not in s.name]
        self.assertIn("breakdown", names)
        self.assertIn("stop", names)
        self.assertLess(names.index("stop"), names.index("breakdown"))
        self.assertTrue(names[names.index("breakdown") + 1].startswith("chorus"))

    def test_rhythm_guitar_plays_power_chords_not_triads(self):
        g = track(self.comp, "rhythm_guitar")
        by_start = {}
        for n in g.notes:
            by_start.setdefault(n.start, []).append(parse_pitch(n.pitch))
        for start, ps in by_start.items():
            if len(ps) > 1:
                intervals = {p - min(ps) for p in ps}
                self.assertTrue(intervals <= {0, 7, 12, 13, 18, 19}, (start, ps))

    def test_riff_sits_on_the_tuning(self):
        g = track(self.comp, "rhythm_guitar")
        self.assertGreaterEqual(min(parse_pitch(n.pitch) for n in g.notes), 36)
        bd = self.sections["breakdown"]
        lows = [parse_pitch(n.pitch) for n in notes_in(g, bd) if n.articulation == "palm_mute"]
        self.assertTrue(lows and all(p in (36, 37) for p in lows))   # C2 pedal (or the b2)

    def test_breakdown_kick_follows_riff_and_bass_doubles(self):
        g, d, b = track(self.comp, "rhythm_guitar"), track(self.comp, "drums"), track(self.comp, "bass")
        bd = self.sections["breakdown"]
        riff = [n.start for n in notes_in(g, bd)]
        kicks = [n.start for n in notes_in(d, bd) if n.drum == "kick"]
        chugs = [n.start for n in notes_in(g, bd) if n.articulation == "palm_mute"]
        self.assertGreater(near(kicks, riff), 0.85)
        self.assertGreater(near(chugs, kicks), 0.85)
        self.assertEqual(near([n.start for n in b.notes], [n.start for n in g.notes]), 1.0)

    def test_breakdown_drums_and_mutes(self):
        d = track(self.comp, "drums")
        bd = self.sections["breakdown.chug"]
        lanes = {n.drum for n in notes_in(d, bd)}
        self.assertIn("china", lanes)
        snares = [n.start - bd.start_beat() for n in notes_in(d, bd) if n.drum == "snare"]
        self.assertTrue(any(abs(s % 4 - 2.0) < 1e-6 for s in snares))   # snare on beat 3
        lead = track(self.comp, "lead")
        self.assertFalse(notes_in(lead, self.sections["breakdown"]))

    def test_stop_is_one_hit_then_silence(self):
        stop = self.sections["stop"]
        for t in self.comp.tracks:
            for n in notes_in(t, stop):
                self.assertAlmostEqual(n.start, stop.start_beat(), places=3, msg=t.name)

    def test_micro_sections_get_their_own_grooves(self):
        d = track(self.comp, "drums")
        va, vb = self.sections["verse.A"], self.sections["verse.B"]
        self.assertNotIn("china", {n.drum for n in notes_in(d, va)})
        self.assertIn("china", {n.drum for n in notes_in(d, vb)})     # gallop-verse groove

    def test_crash_on_section_downbeats_and_fills(self):
        d = track(self.comp, "drums")
        crash_starts = {n.start for n in d.notes if n.drum == "crash"}
        for name in ("verse", "chorus", "breakdown"):
            self.assertIn(self.sections[name].start_beat(), crash_starts, name)
        self.assertTrue({"tom_high", "tom_mid", "floor_tom"} & {n.drum for n in d.notes})

    def test_riff_motif_repeats_and_verse2_reuses_verse(self):
        g = track(self.comp, "rhythm_guitar")

        def sig(bar):
            a = (bar - 1) * 4
            return sorted((round(n.start - a, 3), n.articulation) for n in g.notes if a <= n.start < a + 4)

        v = self.sections["verse"].start_bar
        self.assertEqual(sig(v), sig(v + 4))
        v2 = self.sections["verse2"].start_bar
        self.assertEqual(sig(v), sig(v2))

    def test_per_section_scale(self):
        timeline, _ = build_timeline(self.profile, self.comp.sections, "C minor",
                                     __import__("random").Random(1))
        scales = {s.section.name.partition(".")[0].rstrip("0123456789"): s.scale for s in timeline}
        self.assertEqual(scales["verse"], "phrygian")
        self.assertEqual(scales["chorus"], "minor")

    def test_double_tracking(self):
        comp = compose(load_profile("metalcore"), bars=8, seed=2)
        guitars = [t for t in comp.tracks if t.role == "rhythm_guitar"]
        self.assertEqual([t.pan for t in guitars], [-1.0, 1.0])
        self.assertNotEqual([n.start for n in guitars[0].notes], [n.start for n in guitars[1].notes])

    def test_regenerate_keeps_everything_outside_the_section(self):
        target = "breakdown.drop"
        new = regenerate_section(self.comp, self.profile, target, layers=["drums"], seed=5)
        sec = self.sections[target]
        old_d, new_d = track(self.comp, "drums"), track(new, "drums")
        outside = lambda t: [(n.start, n.drum) for n in t.notes
                             if not (sec.start_beat() <= n.start < sec.end_beat())]
        self.assertEqual(outside(old_d), outside(new_d))
        self.assertEqual(track(self.comp, "rhythm_guitar").notes, track(new, "rhythm_guitar").notes)
        # regenerated breakdown drums still lock to the existing riff (ignoring the end fill)
        kicks = [n.start for n in notes_in(new_d, sec)
                 if n.drum == "kick" and n.start < sec.end_beat() - 2]
        riff = [n.start for n in notes_in(track(new, "rhythm_guitar"), sec)]
        self.assertGreater(near(kicks, riff), 0.95)


class TestCrabcore(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.profile = load_profile("crabcore")
        cls.comp = compose(cls.profile, use_form=True, seed=4)
        cls.sections = {s.name: s for s in cls.comp.sections}

    def take(self, side):
        return next(t for t in self.comp.tracks if t.name == f"Rhythm Guitar {side}")

    def test_dance_break_is_drum_machine_and_synths(self):
        db = self.sections["dance-break"]
        roles = {t.role for t in self.comp.tracks if notes_in(t, db)}
        self.assertIn("electronic_drums", roles)
        self.assertTrue({"pad", "chords", "synth_bass"} <= roles)
        self.assertFalse({"drums", "rhythm_guitar", "bass", "melody"} & roles)
        kicks = sorted(n.start - db.start_beat() for n in notes_in(track(self.comp, "electronic_drums"), db)
                       if n.drum == "kick")
        self.assertTrue(all(abs(k % 1.0) < 0.02 or abs(k % 1.0 - 1) < 0.02 for k in kicks))  # four on the floor

    def test_tempo_shifts_and_returns(self):
        tm = dict(self.comp.tempo_map)
        db = self.sections["dance-break"]
        self.assertAlmostEqual(tm[db.start_beat()], round(142 * 0.9, 2))
        self.assertEqual(self.comp.tempo_map[-1][1], 142)

    def test_electronic_layer_only_where_bound(self):
        e = track(self.comp, "electronic_drums")
        self.assertFalse(notes_in(e, self.sections["verse.A"]))
        self.assertTrue(notes_in(e, self.sections["verse.B"]))

    def test_play_in_limits_parts(self):
        melody = track(self.comp, "melody")
        for name, sec in self.sections.items():
            if not sec.is_micro and not name.startswith("chorus"):
                self.assertFalse(notes_in(melody, sec), name)

    def test_double_track_modes_per_section(self):
        L, R = self.take("L"), self.take("R")

        def pairs(sec, art=None):
            ln = sorted((round(n.start, 1), parse_pitch(n.pitch)) for n in notes_in(L, sec)
                        if art is None or n.articulation == art)
            rn = sorted((round(n.start, 1), parse_pitch(n.pitch)) for n in notes_in(R, sec)
                        if art is None or n.articulation == art)
            return ln, rn
        ln, rn = pairs(self.sections["breakdown"])
        self.assertEqual([p for _, p in ln], [p for _, p in rn])           # identical
        lo, ro = pairs(self.sections["chorus"], "open")
        self.assertTrue(lo and all(b - a == 12 for (_, a), (_, b) in zip(lo, ro)))  # octave split
        lp, rp = pairs(self.sections["chorus"], "palm_mute")
        self.assertEqual(lp, rp)                                           # chugs stay identical
        lv, rv = pairs(self.sections["verse"])
        self.assertNotEqual(lv, rv)                                        # variation

    def test_synth_stabs_follow_the_breakdown_riff(self):
        bd = self.sections["breakdown"]
        stabs = sorted({n.start for n in notes_in(track(self.comp, "chords"), bd)})
        riff = [n.start for n in notes_in(self.take("L"), bd)]
        self.assertGreater(near(stabs, riff, tol=0.05), 0.9)

    def test_automation_lanes(self):
        pad, sb = track(self.comp, "pad"), track(self.comp, "synth_bass")
        ch = self.sections["chorus"]
        pumps = [e for e in pad.automation if e["param"] == "volume" and ch.start_beat() <= e["time"] < ch.end_beat()]
        self.assertTrue(pumps and min(e["value"] for e in pumps) < 0.5)
        bd = self.sections["breakdown"]
        wob = [e["value"] for e in sb.automation if e["param"] == "filter" and bd.start_beat() <= e["time"] < bd.end_beat()]
        self.assertGreater(max(wob) - min(wob), 0.5)
        stop = self.sections["stop"]
        dive = [e for e in sb.automation if e["param"] == "pitch" and stop.start_beat() <= e["time"] <= stop.end_beat()]
        self.assertLess(min(e["semitones"] for e in dive), -6)


class TestStructure(unittest.TestCase):
    def test_optional_form_slots_vary_with_seed(self):
        p = load_profile("metalcore")
        shapes = {tuple(s.name for s in plan_sections(p, None, __import__("random").Random(i), use_form=True)
                        if "." not in s.name) for i in range(12)}
        self.assertGreater(len(shapes), 1)

    def test_bars_budget_truncates_form(self):
        p = load_profile("scenecore")
        comp = compose(p, bars=24, seed=1)
        self.assertEqual(comp.bars, 24)


class TestRendering(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp())

    def midi_notes(self, path):
        import mido
        return [m for t in mido.MidiFile(path).tracks for m in t if m.type == "note_on" and m.velocity]

    def test_sd3_and_odin_maps(self):
        try:
            import mido  # noqa
        except ImportError:
            self.skipTest("mido not installed")
        comp = compose(dry(load_profile("metalcore")), use_form=True, seed=11)
        generate_midi(comp, split_tracks=True, out_dir=self.tmp)
        files = sorted((self.tmp / comp.title).glob("*.mid"))
        self.assertEqual(len(files), len(comp.tracks))
        drums = self.midi_notes(next(f for f in files if "Drums" in f.name))
        self.assertIn(27, {m.note for m in drums})           # SD3 china 1
        sd3 = load_instrument_map("sd3")
        self.assertEqual(sd3.drum_note("china", 52), 27)
        self.assertEqual(sd3.drum_note("clap", 39), 40)      # fallback → rimshot
        gtr = self.midi_notes(next(f for f in files if "Rhythm" in f.name))
        odin = load_instrument_map("odin-iii")
        bands = list(odin.velocity_bands.values())
        self.assertTrue(all(any(lo <= m.velocity <= hi for lo, hi in bands) for m in gtr))

    def test_any_vst_on_any_genre(self):
        comp = compose(load_profile("lo-fi-hip-hop"), bars=8, seed=1,
                       vst_overrides={"drums": "superior-drummer-3"})
        self.assertEqual(track(comp, "drums").vst, "superior-drummer-3")

    def test_keyswitches_held_across_notes(self):
        odin = InstrumentMap.from_dict(json.loads((ROOT / "instruments/odin-iii.json").read_text()))
        odin.keyswitches["palm_mute"] = 24
        self.assertEqual(odin.keyswitch_for("palm_mute"), 24)
        self.assertIsNone(odin.velocity_for("palm_mute", 0.5))     # keyswitch wins
        self.assertIsNotNone(odin.velocity_for("open", 0.5))       # no keyswitch → band
        merged = _keyswitch_events([(24, 0.0, 0.5), (24, 0.5, 1.0), (25, 1.0, 2.0)], latching=False)
        self.assertEqual(merged, [(24, 0.0, 1.0), (25, 1.0, 2.0)])
        latched = _keyswitch_events([(24, 0.0, 0.5), (24, 0.5, 1.0), (25, 1.0, 2.0)], latching=True)
        self.assertEqual([e[0] for e in latched], [24, 25])

    def test_automation_tempo_and_glide_render(self):
        try:
            import mido
        except ImportError:
            self.skipTest("mido not installed")
        comp = compose(load_profile("crabcore"), use_form=True, seed=4)
        generate_midi(comp, split_tracks=True, out_dir=self.tmp)
        events = lambda f: [m for t in mido.MidiFile(f).tracks for m in t]
        d = self.tmp / comp.title
        sb = events(next(d.glob("*Synth_Bass.mid")))
        self.assertTrue(any(m.type == "control_change" and m.control == 74 for m in sb))
        self.assertTrue(any(m.type == "pitchwheel" and m.pitch < -4000 for m in sb))
        tempos = [m for m in events(self.tmp / f"{comp.title}.mid") if m.type == "set_tempo"]
        self.assertGreaterEqual(len(tempos), 3)
        # 808 glides overlap in scenecore (Serum 2 legato)
        sc = compose(load_profile("scenecore"), use_form=True, seed=3)
        self.assertTrue(any(n.articulation == "glide" for n in track(sc, "bass").notes))

    def test_beatmaker_maps(self):
        bm = load_instrument_map("beatmaker")
        self.assertEqual((bm.drum_note("kick", 0), bm.drum_note("clap", 0), bm.drum_note("china", 0)), (36, 39, 50))
        style = load_instrument_map("ujam-beatmaker-style")
        comp = compose(load_profile("crabcore"), use_form=True, seed=4,
                       vst_overrides={"electronic_drums": "ujam-beatmaker-style"})
        keys = [k for k, _, _ in _phrase_events(comp, track(comp, "electronic_drums"), style)]
        self.assertTrue(set(keys) <= set(range(60, 84)))
        self.assertIn(69, keys)          # Chorus 1
        self.assertIn(78, keys)          # Breakdown 1
        self.assertIn(77, keys)          # Special 1 (dance break)

    def test_untangle_same_pitch_overlaps(self):
        out = _untangle([(38, 0.0, 0.25, 90), (38, 0.003, 0.25, 110), (40, 0.0, 2.0, 80), (40, 1.0, 1.0, 80)])
        self.assertEqual([(p, t, round(d, 3), v) for p, t, d, v in out],
                         [(38, 0.0, 0.253, 110), (40, 0.0, 1.0, 80), (40, 1.0, 1.0, 80)])

    def test_legacy_composition_dict(self):
        path = generate_midi_from_dict({
            "title": "legacy", "bpm": 100,
            "tracks": [{"instrument": "acoustic-grand-piano",
                        "notes": [{"pitch": "C4", "duration": "4"}, {"pitch": "E4", "duration": "4"}]}],
        })
        self.assertTrue(Path(path).exists())
        round_trip = Composition.from_dict(compose(load_profile("metalcore"), bars=8, seed=1).to_dict())
        self.assertTrue(any(n.articulation for t in round_trip.tracks for n in t.notes))
        self.assertTrue(round_trip.sections[0].bindings)


if __name__ == "__main__":
    unittest.main()
