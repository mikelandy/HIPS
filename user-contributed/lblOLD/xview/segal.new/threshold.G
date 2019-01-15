;GIL-3
(
(
	:type                   :popup-window
	:name                   pop_threshold
	:owner                  nil
	:width                  430
	:height                 480
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Threshold"
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
	:owner                  pop_threshold
	:help                   ""
	:x                      0
	:y                      0
	:width                  430
	:height                 480
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
	:name                   set_roi
	:owner                  controls1
	:help                   ""
	:x                      39
	:y                      4
	:width                  318
	:height                 23
	:value-x                173
	:value-y                4
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Region of interest:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Frame - Whole" "Frame - Cropped region" "Frame - Point list" "Entire image - Whole" "Entire image - Cube" "Entire image - Point list" )
	:choice-label-types     (:string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold set_roi "Entire image - Cube")
		:when                   (Notify )
		:to                     (pop_threshold set_roi "Entire image - Cube")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.roi = R3d_CUBE;
refresh_histogram();")
		)
		(
		:from                   (pop_threshold set_roi "Entire image - Point list")
		:when                   (Notify )
		:to                     (pop_threshold set_roi "Entire image - Point list")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.roi = R3d_PT_LIST;
refresh_histogram();")
		)
		(
		:from                   (pop_threshold set_roi "Entire image - Whole")
		:when                   (Notify )
		:to                     (pop_threshold set_roi "Entire image - Whole")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.roi = R3d_WHOLE;
refresh_histogram();")
		)
		(
		:from                   (pop_threshold set_roi "Frame - Cropped region")
		:when                   (Notify )
		:to                     (pop_threshold set_roi "Frame - Cropped region")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.roi = R2d_CROP;
refresh_histogram();")
		)
		(
		:from                   (pop_threshold set_roi "Frame - Point list")
		:when                   (Notify )
		:to                     (pop_threshold set_roi "Frame - Point list")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.roi = R2d_PT_LIST;
refresh_histogram();")
		)
		(
		:from                   (pop_threshold set_roi "Frame - Whole")
		:when                   (Notify )
		:to                     (pop_threshold set_roi "Frame - Whole")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.roi = R2d_WHOLE;
refresh_histogram();")
		)
	)
)
(
	:type                   :setting
	:name                   set_plane
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      31
	:width                  287
	:height                 23
	:value-x                173
	:value-y                31
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Histogram color plane:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Gray/Intensity" "Red" "Green" "Blue" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:choice-defaults        (nil nil nil nil )
	:initial-selections     (nil nil nil nil )
	:initial-state          :inactive
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold set_plane "Blue")
		:when                   (Notify )
		:to                     (pop_threshold set_plane "Blue")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.plane = BP;
refresh_histogram();")
		)
		(
		:from                   (pop_threshold set_plane "Green")
		:when                   (Notify )
		:to                     (pop_threshold set_plane "Green")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.plane = GP;
refresh_histogram();")
		)
		(
		:from                   (pop_threshold set_plane "Red")
		:when                   (Notify )
		:to                     (pop_threshold set_plane "Red")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.plane = RP;
refresh_histogram();")
		)
		(
		:from                   (pop_threshold set_plane "Gray/Intensity")
		:when                   (Notify )
		:to                     (pop_threshold set_plane "Gray/Intensity")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.plane = VAL_RGB;
refresh_histogram();")
		)
	)
)
(
	:type                   :group
	:name                   group1
	:x                      10
	:y                      31
	:group-type             nil
	:members                (set_plane )
	:rows                   1
	:row-alignment          :top-edges
	:columns                0
	:column-alignment       :left-edges
	:horizontal-spacing     10
	:vertical-spacing       10
	:anchor-object          nil
	:anchor-point           :north-west
	:reference-point        :north-west
	:horizontal-offset      0
	:vertical-offset        0
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   set_mask_effect
	:owner                  controls1
	:help                   ""
	:x                      30
	:y                      58
	:width                  260
	:height                 23
	:value-x                173
	:value-y                58
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Effect on edit mask:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Overwrite" "Add to" "Remove from" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:choice-defaults        (nil nil nil )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold set_mask_effect "Add to")
		:when                   (Notify )
		:to                     (pop_threshold set_mask_effect "Add to")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.mask_effect = THRESH_ADD_TO;")
		)
		(
		:from                   (pop_threshold set_mask_effect "Overwrite")
		:when                   (Notify )
		:to                     (pop_threshold set_mask_effect "Overwrite")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.mask_effect = THRESH_OVERWRITE;")
		)
		(
		:from                   (pop_threshold set_mask_effect "Remove from")
		:when                   (Notify )
		:to                     (pop_threshold set_mask_effect "Remove from")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.mask_effect = THRESH_REMOVE_FROM;")
		)
	)
)
(
	:type                   :button
	:name                   but_threshold_mask
	:owner                  controls1
	:help                   ""
	:x                      310
	:y                      60
	:width                  115
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Threshold mask"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold but_threshold_mask)
		:when                   (Notify )
		:to                     (pop_threshold but_threshold_mask)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold_mask();")
		)
	)
)
(
	:type                   :setting
	:name                   set_image_effect
	:owner                  controls1
	:help                   ""
	:x                      55
	:y                      85
	:width                  238
	:height                 23
	:value-x                173
	:value-y                85
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Effect on image:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Darken" "Lighten" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold set_image_effect "Darken")
		:when                   (Notify )
		:to                     (pop_threshold set_image_effect "Darken")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.image_effect = -1 * threshold.degree;")
		)
		(
		:from                   (pop_threshold set_image_effect "Lighten")
		:when                   (Notify )
		:to                     (pop_threshold set_image_effect "Lighten")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.image_effect = threshold.degree;")
		)
	)
)
(
	:type                   :button
	:name                   but_threshold_image
	:owner                  controls1
	:help                   ""
	:x                      305
	:y                      88
	:width                  120
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Threshold image"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold but_threshold_image)
		:when                   (Notify )
		:to                     (pop_threshold but_threshold_image)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold_image();")
		)
	)
)
(
	:type                   :slider
	:name                   set_degree
	:owner                  controls1
	:help                   ""
	:x                      145
	:y                      112
	:width                  224
	:height                 20
	:value-x                173
	:value-y                112
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "By:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              255
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          20
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold set_degree)
		:when                   (Notify )
		:to                     (pop_threshold set_degree)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("threshold.degree = value;
if(threshold.image_effect < 0)
	threshold.image_effect = -1 * threshold.degree;
else threshold.image_effect = threshold.degree;")
		)
	)
)
(
	:type                   :message
	:name                   msg_line1
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      136
	:width                  416
	:height                 13
	:foreground-color       ""
	:label                  "----------------------------------------------------"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_min_down
	:owner                  controls1
	:help                   ""
	:x                      150
	:y                      152
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
	:actions                (
		(
		:from                   (pop_threshold but_min_down)
		:when                   (Notify )
		:to                     (pop_threshold but_min_down)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(threshold.min > 0) {
	erase_thresh_bounds();
	threshold.min--;
	draw_thresh_bounds();
}")
		)
	)
)
(
	:type                   :button
	:name                   but_min_up
	:owner                  controls1
	:help                   ""
	:x                      178
	:y                      152
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
	:actions                (
		(
		:from                   (pop_threshold but_min_up)
		:when                   (Notify )
		:to                     (pop_threshold but_min_up)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(threshold.min < threshold.max) {
	erase_thresh_bounds();
	threshold.min++;
	draw_thresh_bounds();
}")
		)
	)
)
(
	:type                   :button
	:name                   but_max_down
	:owner                  controls1
	:help                   ""
	:x                      310
	:y                      152
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
	:actions                (
		(
		:from                   (pop_threshold but_max_down)
		:when                   (Notify )
		:to                     (pop_threshold but_max_down)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(threshold.max > threshold.min) {
	erase_thresh_bounds();
	threshold.max--;
	draw_thresh_bounds();
}
")
		)
	)
)
(
	:type                   :button
	:name                   but_max_up
	:owner                  controls1
	:help                   ""
	:x                      338
	:y                      152
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
	:actions                (
		(
		:from                   (pop_threshold but_max_up)
		:when                   (Notify )
		:to                     (pop_threshold but_max_up)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(threshold.max < 255) {
	erase_thresh_bounds();
	threshold.max++;
	draw_thresh_bounds();
}")
		)
	)
)
(
	:type                   :message
	:name                   msg_min
	:owner                  controls1
	:help                   ""
	:x                      66
	:y                      158
	:width                  29
	:height                 13
	:foreground-color       ""
	:label                  "Min:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_max
	:owner                  controls1
	:help                   ""
	:x                      226
	:y                      158
	:width                  30
	:height                 13
	:foreground-color       ""
	:label                  "Max:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_max_histo
	:owner                  controls1
	:help                   ""
	:x                      348
	:y                      182
	:width                  32
	:height                 13
	:foreground-color       ""
	:label                  "2000"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_refresh
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      184
	:width                  64
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Refresh"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold but_refresh)
		:when                   (Notify )
		:to                     (pop_threshold but_refresh)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("draw_histogram();
draw_thresh_bounds();")
		)
	)
)
(
	:type                   :button
	:name                   but_stats
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      216
	:width                  48
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Stats"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold but_stats)
		:when                   (Notify )
		:to                     (pop_threshold but_stats)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("draw_histo_stats();")
		)
	)
)
(
	:type                   :button
	:name                   but_histoeq
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      248
	:width                  63
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "HistoEq"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold but_histoeq)
		:when                   (Notify )
		:to                     (pop_threshold but_histoeq)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("histoeq();")
		)
	)
)
(
	:type                   :message
	:name                   msg_mu
	:owner                  controls1
	:help                   ""
	:x                      344
	:y                      424
	:width                  46
	:height                 13
	:foreground-color       ""
	:label                  "mu = 1"
	:label-type             :string
	:label-bold             t
	:initial-state          :invisible
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_0
	:owner                  controls1
	:help                   ""
	:x                      90
	:y                      446
	:width                  8
	:height                 13
	:foreground-color       ""
	:label                  "0"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_255
	:owner                  controls1
	:help                   ""
	:x                      316
	:y                      446
	:width                  24
	:height                 13
	:foreground-color       ""
	:label                  "255"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls1
	:help                   ""
	:x                      189
	:y                      454
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
		:from                   (pop_threshold but_close)
		:when                   (Notify )
		:to                     (pop_threshold)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas
	:owner                  pop_threshold
	:help                   ""
	:x                      87
	:y                      184
	:width                  256
	:height                 256
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       256
	:vertical-scrollbar     nil
	:scrollable-height      256
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (pop_threshold canvas)
		:when                   (AnyEvent )
		:to                     (pop_threshold canvas)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("thresh_event(event);")
		)
	)
)
)
