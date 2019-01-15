;GIL-3
(
(
	:type                   :popup-window
	:name                   pop_image_reg
	:owner                  win
	:width                  444
	:height                 199
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Image Registry"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              nil
	:pinned                 t
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  pop_image_reg
	:help                   ""
	:x                      0
	:y                      0
	:width                  444
	:height                 199
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_automatic
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      15
	:width                  72
	:height                 13
	:foreground-color       ""
	:label                  "Automatic:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_manual
	:owner                  controls1
	:help                   ""
	:x                      370
	:y                      15
	:width                  54
	:height                 13
	:foreground-color       ""
	:label                  "Manual:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_line_scale
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      42
	:width                  78
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Line Scale"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_scale
	:owner                  controls1
	:help                   ""
	:x                      374
	:y                      42
	:width                  49
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Scale"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_scale_factor
	:owner                  controls1
	:help                   ""
	:x                      146
	:y                      45
	:width                  156
	:height                 15
	:value-x                238
	:value-y                45
	:value-length           8
	:stored-length          20
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Scale Factor:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          "1.0"
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_translate_up
	:owner                  controls1
	:help                   ""
	:x                      386
	:y                      77
	:width                  26
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "^"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_point_translate
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      93
	:width                  111
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Point Translate"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_translate_left
	:owner                  controls1
	:help                   ""
	:x                      362
	:y                      93
	:width                  26
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "<"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_translate_right
	:owner                  controls1
	:help                   ""
	:x                      410
	:y                      93
	:width                  26
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  ">"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   set_translate_pixels
	:owner                  controls1
	:help                   ""
	:x                      146
	:y                      97
	:width                  181
	:height                 20
	:value-x                193
	:value-y                97
	:slider-width           20
	:ticks                  0
	:foreground-color       ""
	:label                  "Pixels:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              20
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_translate_down
	:owner                  controls1
	:help                   ""
	:x                      386
	:y                      109
	:width                  25
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "v"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_line_rotate
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      146
	:width                  86
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Line Rotate"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_rotate
	:owner                  controls1
	:help                   ""
	:x                      371
	:y                      146
	:width                  57
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Rotate"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_rotation_degrees
	:owner                  controls1
	:help                   ""
	:x                      146
	:y                      149
	:width                  192
	:height                 15
	:value-x                274
	:value-y                149
	:value-length           8
	:stored-length          20
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Rotation Degrees:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          "0.0"
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_close_image_reg
	:owner                  controls1
	:help                   ""
	:x                      207
	:y                      174
	:width                  51
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_image_reg but_close_image_reg)
		:when                   (Notify )
		:to                     (pop_image_reg)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (pop_image_reg but_close_image_reg)
		:when                   (Notify )
		:to                     (pop_ref_frame)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_ref_frame
	:owner                  win
	:width                  346
	:height                 61
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Reference Frame"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              nil
	:pinned                 t
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   canvas
	:owner                  pop_ref_frame
	:help                   ""
	:x                      0
	:y                      0
	:width                  1
	:height                 1
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       1
	:vertical-scrollbar     nil
	:scrollable-height      1
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xview
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls2
	:owner                  pop_ref_frame
	:help                   ""
	:x                      0
	:y                      2
	:width                  346
	:height                 59
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   set_reference_aspect
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      4
	:width                  202
	:height                 23
	:value-x                138
	:value-y                4
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Reference Aspect:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("X axis" "Y axis" "Z axis" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:choice-defaults        (nil nil nil )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   set_reference_frame
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      33
	:width                  322
	:height                 20
	:value-x                129
	:value-y                33
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Reference Frame:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              0
	:max-value              100
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
)
