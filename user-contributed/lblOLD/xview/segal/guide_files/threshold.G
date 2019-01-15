;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  545
	:height                 311
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal : Thresholding"
	:label-type             :string
	:mapped                 t
	:show-footer            t
	:resizable              t
	:icon-file              ""
	:icon-mask-file         ""
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls2
	:owner                  win
	:help                   ""
	:x                      0
	:y                      0
	:width                  64
	:height                 256
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :canvas-pane
	:name                   canv_histo
	:owner                  win
	:help                   ""
	:x                      61
	:y                      0
	:width                  256
	:height                 256
	:background-color       ""
	:foreground-color       ""
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       256
	:vertical-scrollbar     nil
	:scrollable-height      256
	:repaint-proc           generate_histogram
	:event-handler          nil
	:events                 ()
	:drawing-model          :xview
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  win
	:help                   ""
	:x                      317
	:y                      0
	:width                  15
	:height                 256
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls
	:owner                  win
	:help                   ""
	:x                      332
	:y                      0
	:width                  213
	:height                 311
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
	:name                   msg_histogram
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      16
	:width                  129
	:height                 13
	:foreground-color       ""
	:label                  "Histogram Method:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   histo_set
	:owner                  controls
	:help                   ""
	:x                      0
	:y                      32
	:width                  167
	:height                 23
	:value-x                9
	:value-y                32
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:rows                   1
	:columns                0
	:label                  ""
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Normal" "Exclude Edge Points" "Smooth" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_upper
	:owner                  controls
	:help                   ""
	:x                      0
	:y                      268
	:width                  43
	:height                 13
	:foreground-color       ""
	:label                  "Upper"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_lower
	:owner                  controls
	:help                   ""
	:x                      0
	:y                      288
	:width                  42
	:height                 13
	:foreground-color       ""
	:label                  "Lower"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_threshold
	:owner                  controls
	:help                   ""
	:x                      68
	:y                      288
	:constant-width         nil
	:button-type            :normal
	:width                  79
	:height                 19
	:foreground-color       ""
	:label                  "Threshold"
	:label-type             :string
	:menu                   nil
	:notify-handler         threshold_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      156
	:y                      288
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_threshold
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls_stats
	:owner                  win
	:help                   ""
	:x                      0
	:y                      256
	:width                  332
	:height                 55
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
	:name                   msg_0
	:owner                  controls_stats
	:help                   ""
	:x                      60
	:y                      0
	:width                  8
	:height                 13
	:foreground-color       ""
	:label                  "0"
	:label-type             :string
	:label-bold             nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_256
	:owner                  controls_stats
	:help                   ""
	:x                      304
	:y                      0
	:width                  24
	:height                 13
	:foreground-color       ""
	:label                  "256"
	:label-type             :string
	:label-bold             nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   threshold_value_upper
	:owner                  controls_stats
	:help                   ""
	:x                      4
	:y                      12
	:width                  321
	:height                 15
	:value-x                10
	:value-y                12
	:slider-width           255
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              255
	:ticks                  0
	:notify-handler         set_thresh_upper
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   threshold_value_lower
	:owner                  controls_stats
	:help                   ""
	:x                      4
	:y                      32
	:width                  321
	:height                 15
	:value-x                13
	:value-y                32
	:slider-width           255
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              255
	:ticks                  0
	:notify-handler         set_thresh_lower
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
