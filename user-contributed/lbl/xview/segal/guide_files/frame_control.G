;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  458
	:height                 239
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal : Frame Control"
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
	:name                   controls_fill1
	:owner                  win
	:help                   ""
	:x                      0
	:y                      0
	:width                  176
	:height                 26
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
	:name                   msg_image
	:owner                  controls_fill1
	:help                   ""
	:x                      124
	:y                      8
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
	:type                   :canvas-pane
	:name                   canv_image_frame_status
	:owner                  win
	:help                   ""
	:x                      176
	:y                      0
	:width                  282
	:height                 28
	:background-color       ""
	:foreground-color       ""
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       282
	:vertical-scrollbar     nil
	:scrollable-height      28
	:repaint-proc           nil
	:event-handler          nil
	:events                 ()
	:drawing-model          :xview
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls_fill2
	:owner                  win
	:help                   ""
	:x                      0
	:y                      26
	:width                  176
	:height                 26
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
	:name                   msg_mask
	:owner                  controls_fill2
	:help                   ""
	:x                      128
	:y                      8
	:width                  39
	:height                 13
	:foreground-color       ""
	:label                  "Mask:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :canvas-pane
	:name                   canv_mask_frame_status
	:owner                  win
	:help                   ""
	:x                      176
	:y                      26
	:width                  282
	:height                 26
	:background-color       ""
	:foreground-color       ""
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       282
	:vertical-scrollbar     nil
	:scrollable-height      26
	:repaint-proc           nil
	:event-handler          nil
	:events                 ()
	:drawing-model          :xview
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls
	:owner                  win
	:help                   ""
	:x                      0
	:y                      52
	:width                  458
	:height                 187
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   curr_frame
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      8
	:width                  443
	:height                 20
	:value-x                118
	:value-y                8
	:slider-width           255
	:foreground-color       ""
	:label                  "Current Frame:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             nil
	:show-value             t
	:min-value              1
	:max-value              256
	:ticks                  0
	:notify-handler         get_frame_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_frame_status
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      28
	:width                  89
	:height                 13
	:foreground-color       ""
	:label                  "Frame Status"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_image_frame_status
	:owner                  controls
	:help                   ""
	:x                      36
	:y                      44
	:width                  227
	:height                 23
	:value-x                89
	:value-y                44
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Image:"
	:label-type             :string
	:notify-handler         set_image_frame_status_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Unregistered" "Registered" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :text-field
	:name                   text_ref_frame
	:owner                  controls
	:help                   ""
	:x                      276
	:y                      52
	:width                  162
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Reference Frame:"
	:label-type             :string
	:value-x                402
	:value-y                52
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
	:type                   :setting
	:name                   set_mask_frame_status
	:owner                  controls
	:help                   ""
	:x                      44
	:y                      72
	:width                  265
	:height                 23
	:value-x                91
	:value-y                72
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Mask:"
	:label-type             :string
	:notify-handler         set_mask_frame_status_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Blank" "Altered" "Saved" "Locked" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:user-data              ()
)
(
	:type                   :button
	:name                   movie_go_button
	:owner                  controls
	:help                   ""
	:x                      344
	:y                      112
	:constant-width         nil
	:button-type            :normal
	:width                  47
	:height                 19
	:foreground-color       ""
	:label                  "Start"
	:label-type             :string
	:menu                   nil
	:notify-handler         movie_go_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_movie_stop
	:owner                  controls
	:help                   ""
	:x                      402
	:y                      112
	:constant-width         nil
	:button-type            :normal
	:width                  45
	:height                 19
	:foreground-color       ""
	:label                  "Stop"
	:label-type             :string
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   movie_beg_frame
	:owner                  controls
	:help                   ""
	:x                      26
	:y                      116
	:width                  154
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Movie From:"
	:label-type             :string
	:value-x                116
	:value-y                116
	:layout-type            :horizontal
	:value-length           8
	:stored-length          80
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   movie_end_frame
	:owner                  controls
	:help                   ""
	:x                      208
	:y                      116
	:width                  92
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "To:"
	:label-type             :string
	:value-x                236
	:value-y                116
	:layout-type            :horizontal
	:value-length           8
	:stored-length          80
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_stack_load
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      136
	:width                  162
	:height                 23
	:value-x                117
	:value-y                136
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:rows                   1
	:columns                0
	:label                  "Stack Loading:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Yes" "No" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      208
	:y                      164
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_frame_control
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
