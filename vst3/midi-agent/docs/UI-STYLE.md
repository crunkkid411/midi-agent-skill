# UI style: Y2K chrome, scenecore neon

The look is defined in one place, `source/ui/theme.h` / `theme.cpp`. Views
never hard code a colour or a font.

## Tokens

| Token | Value | Used for |
|---|---|---|
| `kVoid` | #07050D | window background |
| `kPanel`, `kPanelLight` | #120A1F, #1F1234 | panels, rows, tiles |
| `kGrid`, `kGridStrong` | #2E1C4C, #543080 | piano roll beat and bar lines |
| `kMagenta`, `kMagentaDim` | #FF2BD6, #8C1876 | the primary neon: selection, hot buttons, wordmark glow |
| `kCyan`, `kCyanDim` | #39F0FF, #1A6E78 | secondary neon: drag chips, hover, headings |
| `kLime` | #B6FF3B | playhead, solo, success lines in the log |
| `kHotPink`, `kViolet` | #FF6EC7, #965CFF | accents, section labels |
| `kChromeHi / Mid / Lo` | #F6F6FF, #AAACBE, #484A5C | chrome bevels and readouts |
| `kText`, `kTextDim` | #ECE6FF, #968CB4 | text |
| `kDanger` | #FF4848 | mute, errors |

Fonts: Verdana bold 24 (wordmark), Verdana bold 10 (labels), Verdana 11
(body), Courier New bold 12 (readouts), Verdana 8 (tiny). Verdana ships with
Windows; on Linux fontconfig substitutes DejaVu.

## Motifs

- **Chrome panels**: `drawChromePanel` (light-to-dark vertical gradient, a
  highlight band on the top 45 percent, a white hairline).
- **Neon pills**: `drawNeonButton` (full-radius rounded rect, gradient body,
  glow halo when hot or hovered, pressed state shifts by 1 px).
- **Sparkles**: `drawSparkle` four-point stars in the header and headings.
- **Scanlines**: `drawScanlines` one dark line every 3 px over the piano roll.
- **Checker strip**: `drawCheckerStrip` as a divider under the header.
- **Horizon lines**: fading magenta lines at the bottom of the header.
- **Glow text**: `drawGlowText` draws the text four times offset by 1 px in
  the glow colour, then once in the text colour.

## Rules

- Chrome for frames and readouts, neon only for state (selected, hot, hover,
  playing). A screen with everything glowing reads as nothing glowing.
- The selected segment tile and the selected track row are the only large
  magenta surfaces.
- Playback position is always lime.
- Keep text on `kText`; `kTextDim` for metadata; never neon for body text.
- Each track has a colour from `trackColor (index)`; the piano roll uses it
  for notes, the track list for the swatch. Ghost notes use the same colour
  at alpha 55.
