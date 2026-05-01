stack(
  stack(
    s("bd [~ bd] bd [~ bd]").bank("RolandTR808").velocity("0.9 0.4 0.85 0.4"),
    s("~ sd ~ sd").bank("RolandTR808").velocity("0.75 0.7 0.75 0.7"),
    s("hh [hh ho] hh [hh ho]").bank("RolandTR808").velocity("0.5 0.3 0.5 0.3")
  ),
  note("a1 ~ a1 ~").s("808").lpf(250).velocity("0.85 0.3 0.8 0.3"),
  note("[a3,c4,e3]@2 [f3,a3,c3]@2").s("supersaw").lpf(1800).release(0.15).velocity("0.6 0.55"),
  note("a3 c4 e4 f3 a3 c4 e4 f3").s("bright_lead").lpf(3000).velocity("0.7 0.65 0.7 0.6 0.7 0.65 0.7 0.6")
)