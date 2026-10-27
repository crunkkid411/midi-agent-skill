"""
Deterministic composers — the Lego Pipeline.

Each module reads a GenreProfile + cumulative composition state and writes
one layer of the song. The orchestrator in pipeline.py runs them in the
profile's layer_order (default):

    structure → harmony → chords → pad → rhythm_guitar → drums → bass → melody → lead

Each layer 'hears' all previous layers (same idea as Ace-Step's context-aware
sequential generation, but with rules instead of a 1.5B-param model).
"""
