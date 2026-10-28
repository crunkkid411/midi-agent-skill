# Classical / general theory (archive, NOT the defaults)

These seven files came with the original midi-agent-skill fork. They hold
textbook rules: classical voice leading, species counterpoint, orchestra
ranges, and jazz and pop progressions.

**Don't apply them by default.** Several of them contradict the music this
project is built for. For example, "never move two voices in parallel fifths"
rules out the power chord. The defaults now live one folder up in
`resources/`, and they're built from the reference albums listed in
`resources/reference-albums.md`.

Read a file here only when:
- the user asks for classical, orchestral, choral or jazz writing, or
- a genre profile in `genres/` says to use one of these rules.

A genre profile in `genres/` always beats both this folder and `resources/`.
