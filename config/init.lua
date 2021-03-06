config.set_sloppy_focus(true)
config.set_tile_borderpx(2)
config.set_float_borderpx(2)
config.set_inner_gaps(20)
config.set_outer_gaps(20)
config.set_root_color({0.3, 0.3, 0.3, 1.0})
config.set_focus_color({1.0, 0.0, 0.0, 1.0})
config.set_border_color({0.0, 0.0, 1.0, 1.0})
config.set_repeat_rate(25)
config.set_repeat_delay(600)

config.create_workspaces({"0:1", "1:2", "2:3", "3:4", "4:5", "5:6", "6:7", "7:8"})

local layouts = {
    {"[M]", "tile"},
    {"[]=", "two_pane"},
    {"||",  "monocle"},
    {"--",  "tmp" },
}

config.set_rules({
    -- {"termite", "termite", function(n) container.set_sticky(n, true) end},
    -- {"termite", "", function(n) container.set_ratio(n, 1) end},
})

config.set_monrules({
    -- {"", function() monitor.set_scale(0.6) end},
    -- {"", function() monitor.set_transform(info.monitor.transform.rotate_90) end},
})

config.create_layout_set("default", layouts)

config.set_default_layout(layouts[1])

local termcmd = "/usr/bin/termite"
config.set_keybinds({
    {"mod-p",         function() action.exec("rofi -show run") end},
    {"mod-e",         function() action.view(info.get_next_empty_workspace(info.get_workspace(), info.direction.right)) end},
    {"mod-period",    function() action.toggle_workspace() end},
    {"mod-S-period",  function() action.toggle_layout() end},
    {"mod-S-Return",  function() action.exec(termcmd) end},
    {"mod-a",         function() action.increase_nmaster() end},
    {"mod-x",         function() action.decrease_nmaster() end},
    {"mod-k",         function() action.focus_on_stack(-1) end},
    {"mod-j",         function() action.focus_on_stack(1) end},
    {"mod-S-j",       function() action.focus_on_hidden_stack(0) end},
    {"mod-S-k",       function() action.focus_on_hidden_stack(-1) end},
    {"mod-S-c",       function() action.kill(info.this_container_position()) end},
    {"mod-S-q",       function() action.quit() end},
    {"mod-space",     function() action.load_next_layout_in_set("default") end},
    {"mod-S-space",   function() action.load_prev_layout_in_set("default") end},
    {"mod-m",         function() action.load_layout_in_set("default", 1) end},
    {"mod-S-t",       function() action.load_layout_in_set("default", 2) end},
    {"mod-w",         function() action.load_layout_in_set("default", 3) end},
    {"mod-S-w",       function() action.load_layout_in_set("default", 4) end},
    {"mod-S-p",       function() action.load_layout({"g", "tmp"}) end},
    {"mod-b",         function() action.toggle_bars() end},
    {"mod-S-h",       function() action.resize_main(-1/10) end},
    {"mod-S-l",       function() action.resize_main(1/10) end},
    {"mod-S-s",       function() action.write_this_overlay("tmp") end},
    {"mod-Return",    function() action.zoom() end},
    -- {"mod-Return",    function() action.repush(2, 0) end},
    {"mod-1",         function() action.view(0) end},
    {"mod-2",         function() action.view(1) end},
    {"mod-3",         function() action.view(2) end},
    {"mod-4",         function() action.view(3) end},
    {"mod-5",         function() action.view(4) end},
    {"mod-6",         function() action.view(5) end},
    {"mod-7",         function() action.view(6) end},
    {"mod-8",         function() action.view(7) end},
    {"mod-9",         function() action.view(8) end},
    {"mod-C-1",       function() action.swap_workspace(info.get_workspace(), 0) end},
    {"mod-C-2",       function() action.swap_workspace(info.get_workspace(), 1) end},
    {"mod-C-3",       function() action.swap_workspace(info.get_workspace(), 2) end},
    {"mod-C-4",       function() action.swap_workspace(info.get_workspace(), 3) end},
    {"mod-C-5",       function() action.swap_workspace(info.get_workspace(), 4) end},
    {"mod-C-6",       function() action.swap_workspace(info.get_workspace(), 5) end},
    {"mod-C-7",       function() action.swap_workspace(info.get_workspace(), 6) end},
    {"mod-C-8",       function() action.swap_workspace(info.get_workspace(), 7) end},
    {"mod-C-9",       function() action.swap_workspace(info.get_workspace(), 8) end},
    {"mod-S-1",       function() action.move_container_to_workspace(0) end},
    {"mod-S-2",       function() action.move_container_to_workspace(1) end},
    {"mod-S-3",       function() action.move_container_to_workspace(2) end},
    {"mod-S-4",       function() action.move_container_to_workspace(3) end},
    {"mod-S-5",       function() action.move_container_to_workspace(4) end},
    {"mod-S-6",       function() action.move_container_to_workspace(5) end},
    {"mod-S-7",       function() action.move_container_to_workspace(6) end},
    {"mod-S-8",       function() action.move_container_to_workspace(7) end},
    {"mod-S-9",       function() action.move_container_to_workspace(8) end},
    {"mod-r",         function() config.reload() end},
    {"mod-t",         function() action.set_floating(false)    end},
    -- {"mod-period",            function() focusmon(1) end},
    -- {"mod-comma",             function() focusmon(-1) end},
    -- {"mod-d",                 function() incnmaster(-1) end},
    {"M1", function() action.focus_container(info.get_container_under_cursor()) end},
    {"mod-M1",  function() action.move_resize(info.cursor.mode.move) end},
    {"mod-M2",  function() action.move_resize(info.cursor.mode.resize) end},
})
