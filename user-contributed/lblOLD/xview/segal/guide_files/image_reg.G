;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  493
	:height                 253
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Image Registry"
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
	:width                  493
	:height                 253
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   text_ref_frame
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      8
	:width                  162
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Reference Frame:"
	:label-type             :string
	:value-x                138
	:value-y                8
	:layout-type            :horizontal
	:value-length           4
	:stored-length          80
	:read-only              nil
	:notify-handler         set_ref_frame_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_save_image_frame
	:owner                  controls
	:help                   ""
	:x                      360
	:y                      16
	:constant-width         nil
	:button-type            :normal
	:width                  129
	:height                 19
	:foreground-color       ""
	:label                  "Save Image Frame"
	:label-type             :string
	:menu                   nil
	:notify-handler         write_image_frame
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_line_matching
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      32
	:width                  100
	:height                 13
	:foreground-color       ""
	:label                  "Line Matching:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_manual
	:owner                  controls
	:help                   ""
	:x                      132
	:y                      32
	:width                  54
	:height                 13
	:foreground-color       ""
	:label                  "Manual:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_line1
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      44
	:width                  104
	:height                 13
	:foreground-color       ""
	:label                  "-------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_line2
	:owner                  controls
	:help                   ""
	:x                      132
	:y                      44
	:width                  360
	:height                 13
	:foreground-color       ""
	:label                  "---------------------------------------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_line_scale
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      72
	:constant-width         nil
	:button-type            :normal
	:width                  78
	:height                 19
	:foreground-color       ""
	:label                  "Line Scale"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_line_scale_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_scale
	:owner                  controls
	:help                   ""
	:x                      136
	:y                      72
	:constant-width         nil
	:button-type            :normal
	:width                  49
	:height                 19
	:foreground-color       ""
	:label                  "Scale"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_manual_scale_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   text_scale_factor
	:owner                  controls
	:help                   ""
	:x                      196
	:y                      76
	:width                  156
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Scale Factor:"
	:label-type             :string
	:value-x                288
	:value-y                76
	:layout-type            :horizontal
	:value-length           8
	:stored-length          80
	:read-only              nil
	:notify-handler         set_scale_factor
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_line4
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      104
	:width                  104
	:height                 13
	:foreground-color       ""
	:label                  "-------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_line5
	:owner                  controls
	:help                   ""
	:x                      129
	:y                      104
	:width                  360
	:height                 13
	:foreground-color       ""
	:label                  "---------------------------------------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_up
	:owner                  controls
	:help                   ""
	:x                      433
	:y                      116
	:constant-width         nil
	:button-type            :normal
	:width                  26
	:height                 19
	:foreground-color       ""
	:label                  "^"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_up_translate_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_point_translate
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      136
	:constant-width         nil
	:button-type            :normal
	:width                  111
	:height                 19
	:foreground-color       ""
	:label                  "Point Translate"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_point_translate_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_left
	:owner                  controls
	:help                   ""
	:x                      409
	:y                      136
	:constant-width         nil
	:button-type            :normal
	:width                  26
	:height                 19
	:foreground-color       ""
	:label                  "<"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_left_translate_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_right
	:owner                  controls
	:help                   ""
	:x                      457
	:y                      136
	:constant-width         nil
	:button-type            :normal
	:width                  26
	:height                 19
	:foreground-color       ""
	:label                  ">"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_right_translate_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   set_trans_pixel_amount
	:owner                  controls
	:help                   ""
	:x                      144
	:y                      140
	:width                  160
	:height                 20
	:value-x                196
	:value-y                140
	:slider-width           20
	:foreground-color       ""
	:label                  "Pixels:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              20
	:ticks                  0
	:notify-handler         set_trans_pixel_amount
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_translate
	:owner                  controls
	:help                   ""
	:x                      333
	:y                      140
	:width                  69
	:height                 13
	:foreground-color       ""
	:label                  "Translate:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_down
	:owner                  controls
	:help                   ""
	:x                      433
	:y                      156
	:constant-width         nil
	:button-type            :normal
	:width                  25
	:height                 19
	:foreground-color       ""
	:label                  "v"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_down_translate_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_line3
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      176
	:width                  104
	:height                 13
	:foreground-color       ""
	:label                  "-------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_line6
	:owner                  controls
	:help                   ""
	:x                      129
	:y                      176
	:width                  360
	:height                 13
	:foreground-color       ""
	:label                  "---------------------------------------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_line_rotate
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      196
	:constant-width         nil
	:button-type            :normal
	:width                  86
	:height                 19
	:foreground-color       ""
	:label                  "Line Rotate"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_line_rotate_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_rotate
	:owner                  controls
	:help                   ""
	:x                      136
	:y                      196
	:constant-width         nil
	:button-type            :normal
	:width                  57
	:height                 19
	:foreground-color       ""
	:label                  "Rotate"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_manual_rotate_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   text_rotation_degrees
	:owner                  controls
	:help                   ""
	:x                      208
	:y                      200
	:width                  192
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Rotation Degrees:"
	:label-type             :string
	:value-x                336
	:value-y                200
	:layout-type            :horizontal
	:value-length           8
	:stored-length          80
	:read-only              nil
	:notify-handler         set_rotation_degrees
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      208
	:y                      224
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_image_reg
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :popup-window
	:name                   pop_ref_frame
	:owner                  win
	:width                  190
	:height                 64
	:background-color       ""
	:foreground-color       ""
	:label                  "Reference Frame"
	:label-type             :string
	:mapped                 nil
	:show-footer            nil
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :canvas-pane
	:name                   canv_ref_frame
	:owner                  pop_ref_frame
	:help                   ""
	:x                      0
	:y                      0
	:width                  64
	:height                 64
	:background-color       ""
	:foreground-color       ""
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       64
	:vertical-scrollbar     nil
	:scrollable-height      64
	:repaint-proc           ref_frame_image_repaint_proc
	:event-handler          ref_frame_mouse_proc
	:events                 (:mouse :mouse-drag )
	:drawing-model          :xview
	:user-data              ()
)
)
