include("defaultConfig.jl")
include("layouts/layouts.jl")
import Main.Layouts

sloppyfocus = 1
borderpx = 1
rootcolor = [0.3, 0.3, 0.3, 1.0]
bordercolor = [0.3, 0.3, 0.3, 1.0]
focuscolor = [1.0, 0.0, 0.0, 0.0]

tags = [ "1", "2", "3", "4", "5", "6", "7", "8", "9" ]

# where to put things
rules = [
    [ "Gimp", "title", 1, true, 3]
]

layouts = [
    [ "[]=", ()-> Layouts.tile() ],
    [ "><>", ()-> Layouts.floating() ],
    [ "[M]", ()-> Layouts.monocle() ],
]

monrules = [
    # name mfact nmaster scale layout transform
    [ "rule", 0.55, 1, 1, layouts[1], NORMAL ],
]


xkb_rules = []
repeatRate = 25
repeatDelay = 600
termcmd = "/usr/bin/termite"

mod = mod1
#maps (between 1 and 4)
keys = [
        ["$mod u",           ()->  run(termcmd)        ],
        ["$mod period",      ()->  focusmon(+1)           ],
        ["$mod comma",       ()->  focusmon(-1)           ],
        ["$mod k",           ()->  focusstack(-1)         ],
        ["$mod j",           ()->  focusstack(1)          ],
        ["$mod i",           ()->  incnmaster(+1)         ],
        ["$mod d",           ()->  incnmaster(-1)         ],
        ["$mod C",           ()->  killclient()           ],
        ["$mod Q",           ()->  quit(0)                ],
        ["$mod space",       ()->  setlayout()            ],
        ["$mod t",           ()->  setlayout(layouts[0])  ],
        ["$mod f",           ()->  setlayout(layouts[1])  ],
        ["$mod m",           ()->  setlayout(layouts[2])  ],
        ["$mod l",           ()->  setmfact(+0.05)        ],
        ["$mod h",           ()->  setmfact(-0.05)        ],
        ["$mod parenright",  ()->  tag(~0)                ],
        ["$mod greater",     ()->  tagmon(+1)             ],
        ["$mod less",        ()->  tagmon(-1)             ],
        ["$mod space",       ()->  togglefloating(0)      ],
        ["$mod Tab",         ()->  view()                 ],
        ["$mod 0",           ()->  view(~0)               ],
        ["$mod Return",      ()->  zoom()                 ],
]