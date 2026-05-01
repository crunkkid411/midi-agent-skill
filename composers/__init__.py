"""
Deterministic composers — the Lego Pipeline.

Each module reads a GenreProfile + cumulative composition state and writes
one layer of the song. The orchestrator in pipeline.py runs them in order:

    structure → chords → drums → bass → melody

Each layer 'hears' all previous layers (same idea as Ace-Step's context-aware
sequential generation, but with rules instead of a 1.5B-param model).
"""
