;GIL-2
(
(
	:type                   :menu
	:name                   menu_file
	:help                   ""
	:columns                1
	:label                  ""
	:label-type             :string
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "File Menu"
	:menu-item-labels       ("Load ..." "Save Frame" "Create Mask" "Quit" )
	:menu-item-label-types  (:string :string :string :string )
	:menu-item-defaults     (t nil nil nil )
	:menu-item-handlers     (menu_load_proc menu_save_frame_proc menu_create_proc menu_quit_proc )
	:menu-item-menus        (nil nil nil nil )
	:menu-item-colors       ("" "" "" "" )
	:pinnable               nil
	:user-data              ()
)
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  451
	:height                 203
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal (SEGmentation AnaLyzer) ver 3.0 beta"
	:label-type             :string
	:mapped                 t
	:show-footer            nil
	:resizable              t
	:icon-file              ""
	:icon-mask-file         ""
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls
	:owner                  win
	:help                   ""
	:x                      0
	:y                      0
	:width                  451
	:height                 203
	:background-color       ""
	:foreground-color       ""
	:show-border            t
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_file
	:owner                  controls
	:help                   ""
	:x                      10
	:y                      8
	:constant-width         nil
	:button-type            :normal
	:width                  78
	:height                 19
	:foreground-color       ""
	:label                  "File I/O"
	:label-type             :string
	:menu                   menu_file
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_image_fname
	:owner                  controls
	:help                   ""
	:x                      99
	:y                      16
	:width                  45
	:height                 13
	:foreground-color       ""
	:label                  "Image:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_image_size
	:owner                  controls
	:help                   ""
	:x                      67
	:y                      36
	:width                  77
	:height                 13
	:foreground-color       ""
	:label                  "Image Size:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_mask_fname
	:owner                  controls
	:help                   ""
	:x                      75
	:y                      56
	:width                  69
	:height                 13
	:foreground-color       ""
	:label                  "Edit Mask:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_undo
	:owner                  controls
	:help                   ""
	:x                      388
	:y                      68
	:constant-width         nil
	:button-type            :normal
	:width                  50
	:height                 19
	:foreground-color       ""
	:label                  "Undo"
	:label-type             :string
	:menu                   nil
	:notify-handler         undo_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_pixedit
	:owner                  controls
	:help                   ""
	:x                      32
	:y                      76
	:width                  112
	:height                 13
	:foreground-color       ""
	:label                  "Edit Region Size:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_pixel_value
	:owner                  controls
	:help                   ""
	:x                      65
	:y                      96
	:width                  79
	:height                 13
	:foreground-color       ""
	:label                  "Pixel Value:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_original
	:owner                  controls
	:help                   ""
	:x                      376
	:y                      96
	:constant-width         nil
	:button-type            :normal
	:width                  67
	:height                 19
	:foreground-color       ""
	:label                  "Original"
	:label-type             :string
	:menu                   nil
	:notify-handler         original_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_edit
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      124
	:constant-width         nil
	:button-type            :normal
	:width                  95
	:height                 19
	:foreground-color       ""
	:label                  "Edit Window"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_edit_win
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_mask_log
	:owner                  controls
	:help                   ""
	:x                      124
	:y                      124
	:constant-width         nil
	:button-type            :normal
	:width                  73
	:height                 19
	:foreground-color       ""
	:label                  "Mask Log"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_mask_log
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_image_reg
	:owner                  controls
	:help                   ""
	:x                      244
	:y                      124
	:constant-width         nil
	:button-type            :normal
	:width                  108
	:height                 19
	:foreground-color       ""
	:label                  "Image Registry"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_image_reg
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_filter
	:owner                  controls
	:help                   ""
	:x                      388
	:y                      124
	:constant-width         nil
	:button-type            :normal
	:width                  56
	:height                 19
	:foreground-color       ""
	:label                  "Filters"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_filter
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_frame
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      156
	:constant-width         nil
	:button-type            :normal
	:width                  105
	:height                 19
	:foreground-color       ""
	:label                  "Frame Control"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_frame_control
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_display
	:owner                  controls
	:help                   ""
	:x                      124
	:y                      156
	:constant-width         nil
	:button-type            :normal
	:width                  112
	:height                 19
	:foreground-color       ""
	:label                  "Display Control"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_display_control
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_mask
	:owner                  controls
	:help                   ""
	:x                      244
	:y                      156
	:constant-width         nil
	:button-type            :normal
	:width                  98
	:height                 19
	:foreground-color       ""
	:label                  "Mask Control"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_mask_control
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_bp
	:owner                  controls
	:help                   ""
	:x                      368
	:y                      156
	:constant-width         nil
	:button-type            :normal
	:width                  78
	:height                 19
	:foreground-color       ""
	:label                  "BP's Tools"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_bp
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_hint
	:owner                  controls
	:help                   ""
	:x                      16
	:y                      184
	:width                  32
	:height                 13
	:foreground-color       ""
	:label                  "Hint:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :popup-window
	:name                   stats_win
	:owner                  win
	:width                  330
	:height                 202
	:background-color       ""
	:foreground-color       ""
	:label                  "Object Statistics"
	:label-type             :string
	:mapped                 nil
	:show-footer            nil
	:resizable              nil
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   stat_control
	:owner                  stats_win
	:help                   ""
	:x                      0
	:y                      0
	:width                  330
	:height                 202
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   stat_mess0
	:owner                  stat_control
	:help                   ""
	:x                      44
	:y                      8
	:width                  107
	:height                 13
	:foreground-color       ""
	:label                  "Object location:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   stat_mess1
	:owner                  stat_control
	:help                   ""
	:x                      15
	:y                      32
	:width                  87
	:height                 13
	:foreground-color       ""
	:label                  "Size (pixels):"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   stat_mess2
	:owner                  stat_control
	:help                   ""
	:x                      15
	:y                      56
	:width                  104
	:height                 13
	:foreground-color       ""
	:label                  "Total Intensity:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   stat_mess3
	:owner                  stat_control
	:help                   ""
	:x                      15
	:y                      80
	:width                  166
	:height                 13
	:foreground-color       ""
	:label                  "Average pixel intensity: "
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   stat_mess4
	:owner                  stat_control
	:help                   ""
	:x                      15
	:y                      104
	:width                  102
	:height                 13
	:foreground-color       ""
	:label                  "Perimeter size:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   stat_mess5
	:owner                  stat_control
	:help                   ""
	:x                      15
	:y                      128
	:width                  112
	:height                 13
	:foreground-color       ""
	:label                  "Min:  NN Max NN"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   stat_mess6
	:owner                  stat_control
	:help                   ""
	:x                      15
	:y                      152
	:width                  53
	:height                 13
	:foreground-color       ""
	:label                  "Median:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   stat_close
	:owner                  stat_control
	:help                   ""
	:x                      140
	:y                      176
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         stat_close_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
