;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  318
	:height                 248
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal : Display Control"
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
	:width                  318
	:height                 248
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   display_type
	:owner                  controls
	:help                   ""
	:x                      24
	:y                      16
	:width                  207
	:height                 23
	:value-x                89
	:value-y                16
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Display:"
	:label-type             :string
	:notify-handler         change_image_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Image" "Mask" "Both" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   display_original
	:owner                  controls
	:help                   ""
	:x                      80
	:y                      44
	:width                  83
	:height                 23
	:value-x                89
	:value-y                44
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:rows                   1
	:columns                0
	:label                  ""
	:label-type             :string
	:notify-handler         orig_map_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Original" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   blend_type
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      72
	:width                  302
	:height                 23
	:value-x                162
	:value-y                72
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Combination Method:"
	:label-type             :string
	:notify-handler         blend_type_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Color Blend" "Overlay" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   mask_type
	:owner                  controls
	:help                   ""
	:x                      68
	:y                      100
	:width                  177
	:height                 23
	:value-x                161
	:value-y                100
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Mask Model:"
	:label-type             :string
	:notify-handler         mask_type_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Cut" "Build" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_image
	:owner                  controls
	:help                   ""
	:x                      40
	:y                      124
	:width                  42
	:height                 13
	:foreground-color       ""
	:label                  "Image"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   gamma_val
	:owner                  controls
	:help                   ""
	:x                      36
	:y                      140
	:width                  182
	:height                 20
	:value-x                103
	:value-y                140
	:slider-width           100
	:foreground-color       ""
	:label                  "Contrast:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             nil
	:min-value              0
	:max-value              100
	:ticks                  0
	:notify-handler         change_colormap
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   message
	:owner                  controls
	:help                   ""
	:x                      40
	:y                      168
	:width                  169
	:height                 13
	:foreground-color       ""
	:label                  "Opacity for image blend:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   bg_slider
	:owner                  controls
	:help                   ""
	:x                      52
	:y                      188
	:width                  204
	:height                 20
	:value-x                103
	:value-y                188
	:slider-width           100
	:foreground-color       ""
	:label                  "Image:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             nil
	:min-value              0
	:max-value              100
	:ticks                  0
	:notify-handler         slider_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   fg_slider
	:owner                  controls
	:help                   ""
	:x                      56
	:y                      208
	:width                  198
	:height                 20
	:value-x                101
	:value-y                208
	:slider-width           100
	:foreground-color       ""
	:label                  "Mask:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             nil
	:min-value              0
	:max-value              100
	:ticks                  0
	:notify-handler         slider_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      136
	:y                      228
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_display_control
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
