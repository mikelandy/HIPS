;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  479
	:height                 201
	:background-color       ""
	:foreground-color       ""
	:label                  "Edit Window"
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
	:name                   control
	:owner                  win
	:help                   ""
	:x                      0
	:y                      0
	:width                  479
	:height                 132
	:background-color       ""
	:foreground-color       ""
	:show-border            t
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   brush_mode
	:owner                  control
	:help                   ""
	:x                      12
	:y                      4
	:width                  192
	:height                 23
	:value-x                104
	:value-y                4
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Brush Mode:"
	:label-type             :string
	:notify-handler         set_brush_mode
	:event-handler          nil
	:events                 ()
	:choices                ("Mask" "Image" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   brush_type
	:owner                  control
	:help                   ""
	:x                      263
	:y                      4
	:width                  210
	:height                 23
	:value-x                362
	:value-y                4
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Brush Shape:"
	:label-type             :string
	:notify-handler         change_cursor_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Square" "Round" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   mask_brush_mode
	:owner                  control
	:help                   ""
	:x                      12
	:y                      32
	:width                  192
	:height                 23
	:value-x                105
	:value-y                32
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Mask Brush:"
	:label-type             :string
	:notify-handler         change_cursor_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Paint " "Erase" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   brush_size
	:owner                  control
	:help                   ""
	:x                      276
	:y                      32
	:width                  159
	:height                 23
	:value-x                361
	:value-y                32
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:rows                   1
	:columns                0
	:label                  "Brush Size:"
	:label-type             :string
	:notify-handler         change_cursor_proc
	:event-handler          nil
	:events                 ()
	:choices                ("1x1" "2x2" "3x3" "4x4" "5x5" "6x6" "20x20" )
	:choice-label-types     (:string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   zoom_mag
	:owner                  control
	:help                   ""
	:x                      280
	:y                      56
	:width                  135
	:height                 23
	:value-x                361
	:value-y                56
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:rows                   1
	:columns                0
	:label                  "Zoom Mag:"
	:label-type             :string
	:notify-handler         mag_proc
	:event-handler          nil
	:events                 ()
	:choices                ("x 1" "x 2" "x 3" "x 4" "x 5" "x 6" )
	:choice-label-types     (:string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   image_brush_mode
	:owner                  control
	:help                   ""
	:x                      5
	:y                      60
	:width                  212
	:height                 23
	:value-x                103
	:value-y                60
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Image Brush:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Darker" "Lighter" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :slider
	:name                   image_brush_delta
	:owner                  control
	:help                   ""
	:x                      38
	:y                      88
	:width                  222
	:height                 20
	:value-x                102
	:value-y                88
	:slider-width           100
	:foreground-color       ""
	:label                  "Amount:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              100
	:ticks                  0
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_original
	:owner                  control
	:help                   ""
	:x                      220
	:y                      104
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
	:name                   but_undo
	:owner                  control
	:help                   ""
	:x                      300
	:y                      104
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
	:type                   :button
	:name                   but_save
	:owner                  control
	:help                   ""
	:x                      364
	:y                      104
	:constant-width         nil
	:button-type            :normal
	:width                  46
	:height                 19
	:foreground-color       ""
	:label                  "Save"
	:label-type             :string
	:menu                   nil
	:notify-handler         menu_save_frame_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  control
	:help                   ""
	:x                      424
	:y                      104
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_edit_win
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_pixel_value
	:owner                  control
	:help                   ""
	:x                      14
	:y                      110
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
	:type                   :canvas-pane
	:name                   canvas
	:owner                  win
	:help                   ""
	:x                      0
	:y                      132
	:width                  72
	:height                 69
	:background-color       ""
	:foreground-color       ""
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       72
	:vertical-scrollbar     nil
	:scrollable-height      69
	:repaint-proc           edit_repaint_proc
	:event-handler          edit_event_proc
	:events                 (:mouse :mouse-enter :mouse-exit :mouse-drag :mouse-move )
	:drawing-model          :xwindows
	:user-data              ()
)
)
