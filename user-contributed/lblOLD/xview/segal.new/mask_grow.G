;GIL-3
(
(
	:type                   :menu
	:name                   menu_set_grow_parameters
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "Set Params"
	:menu-item-labels       ("Default" "From edit mask" "From point list" )
	:menu-item-label-types  (:string :string :string )
	:menu-item-states       (:active :active :active )
	:menu-item-defaults     (nil nil nil )
	:initial-selections     (nil nil nil )
	:menu-item-handlers     (nil nil nil )
	:menu-item-menus        (nil nil nil )
	:menu-item-colors       ("" "" "" )
	:pinnable               t
	:user-data              ()
	:actions                (
		(
		:from                   (menu_set_grow_parameters "From edit mask")
		:when                   (Notify )
		:to                     (menu_set_grow_parameters "From edit mask")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("auto_set_grow_params(SEED_EDIT);")
		)
		(
		:from                   (menu_set_grow_parameters "From point list")
		:when                   (Notify )
		:to                     (menu_set_grow_parameters "From point list")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("auto_set_grow_params(SEED_PTS);")
		)
		(
		:from                   (menu_set_grow_parameters "Default")
		:when                   (Notify )
		:to                     (menu_set_grow_parameters "Default")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(grow.apply_thresholds) {
	grow.threshold_min = 0;
	grow.threshold_max = 255;
}
if(grow.apply_gradient) {
	grow.gradient_rad = 1;
	grow.gradient_min = 0;
	grow.gradient_max = 100;
}
if(grow.apply_bridge) {
	grow.bridge_dist = 5;
	grow.bridge_min = 1;
	grow.bridge_max = 8;
}

/* reset ui fields */
set_grow_params();")
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_mask_grow
	:owner                  win
	:width                  381
	:height                 359
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Mask Growing"
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
	:name                   controls
	:owner                  pop_mask_grow
	:help                   ""
	:x                      0
	:y                      0
	:width                  381
	:height                 359
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
	:name                   set_apply_threshold
	:owner                  controls
	:help                   ""
	:x                      4
	:y                      4
	:width                  149
	:height                 23
	:value-x                85
	:value-y                4
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Threshold:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("apply" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (t )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_apply_threshold)
		:when                   (Notify )
		:to                     (pop_mask_grow set_apply_threshold)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(grow.apply_thresholds) {
	grow.apply_thresholds = FALSE;
	xv_set(Mask_grow_pop_mask_grow->set_threshold_max,
		PANEL_INACTIVE, TRUE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_threshold_min,
		PANEL_INACTIVE, TRUE,
		NULL);
}
else {
	grow.apply_thresholds = TRUE;
	xv_set(Mask_grow_pop_mask_grow->set_threshold_max,
		PANEL_INACTIVE, FALSE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_threshold_min,
		PANEL_INACTIVE, FALSE,
		NULL);
}")
		)
	)
)
(
	:type                   :slider
	:name                   set_threshold_max
	:owner                  controls
	:help                   "Setting: in the paint image, point to the brightest
	pixel that is still part of the region of
	interest.  Hold down the ctrl button and
	click the right mouse button.

Effect: when flooding, the algorithm only chooses
	pixels whose value is <= to the upper
	threshold."
	:x                      18
	:y                      30
	:width                  353
	:height                 20
	:value-x                52
	:value-y                30
	:slider-width           256
	:ticks                  0
	:foreground-color       ""
	:label                  "max:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              255
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          255
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_threshold_max)
		:when                   (Notify )
		:to                     (pop_mask_grow set_threshold_max)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.threshold_max = value;
if(value < grow.threshold_min) {
	grow.threshold_min = value;
	xv_set(Mask_grow_pop_mask_grow->set_threshold_min,
		PANEL_VALUE, value,
		NULL);
}")
		)
	)
)
(
	:type                   :slider
	:name                   set_threshold_min
	:owner                  controls
	:help                   "Setting: in the paint image, point to the darkest
	pixel that is still part of the region of
	interest.  Hold down the ctrl button and
	click the left mouse button.

Effect: when flooding, the algorithm only chooses
	pixels whose value is >= to the lower
	threshold."
	:x                      18
	:y                      54
	:width                  352
	:height                 20
	:value-x                51
	:value-y                54
	:slider-width           256
	:ticks                  0
	:foreground-color       ""
	:label                  "min:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              255
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_threshold_min)
		:when                   (Notify )
		:to                     (pop_mask_grow set_threshold_min)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.threshold_min = value;
if(value > grow.threshold_max) {
	grow.threshold_max = value;
	xv_set(Mask_grow_pop_mask_grow->set_threshold_max,
		PANEL_VALUE, value,
		NULL);
}")
		)
	)
)
(
	:type                   :setting
	:name                   set_apply_gradient
	:owner                  controls
	:help                   ""
	:x                      4
	:y                      72
	:width                  137
	:height                 23
	:value-x                73
	:value-y                72
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Gradient:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("apply" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (t )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_apply_gradient)
		:when                   (Notify )
		:to                     (pop_mask_grow set_apply_gradient)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(grow.apply_gradient) {
	grow.apply_gradient = FALSE;
	xv_set(Mask_grow_pop_mask_grow->set_gradient_radius,
		PANEL_INACTIVE, TRUE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_gradient_max,
		PANEL_INACTIVE, TRUE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_gradient_min,
		PANEL_INACTIVE, TRUE,
		NULL);
}
else {
	grow.apply_gradient = TRUE;
	xv_set(Mask_grow_pop_mask_grow->set_gradient_radius,
		PANEL_INACTIVE, FALSE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_gradient_max,
		PANEL_INACTIVE, FALSE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_gradient_min,
		PANEL_INACTIVE, FALSE,
		NULL);
}")
		)
	)
)
(
	:type                   :slider
	:name                   set_gradient_radius
	:owner                  controls
	:help                   ""
	:x                      18
	:y                      100
	:width                  145
	:height                 20
	:value-x                68
	:value-y                100
	:slider-width           32
	:ticks                  0
	:foreground-color       ""
	:label                  "radius:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              1
	:max-value              8
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          1
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_gradient_radius)
		:when                   (Notify )
		:to                     (pop_mask_grow set_gradient_radius)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.gradient_rad = value;
")
		)
	)
)
(
	:type                   :slider
	:name                   set_gradient_max
	:owner                  controls
	:help                   "Setting: in the paint image, point to an edge
	pixel that is still part of the region of
	interest.  Hold down the ctrl button and
	click the right middle button.

Effect: when flooding, the algorithm only chooses
	pixels whose gradient is <= to the maximum
	gradient."
	:x                      18
	:y                      122
	:width                  353
	:height                 20
	:value-x                52
	:value-y                122
	:slider-width           256
	:ticks                  0
	:foreground-color       ""
	:label                  "max:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              5000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          100
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_gradient_max)
		:when                   (Notify )
		:to                     (pop_mask_grow set_gradient_max)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.gradient_max = value;
if(value < grow.gradient_min) {
	grow.gradient_min = value;
	xv_set(Mask_grow_pop_mask_grow->set_gradient_min,
		PANEL_VALUE, value,
		NULL);
}")
		)
	)
)
(
	:type                   :slider
	:name                   set_gradient_min
	:owner                  controls
	:help                   ""
	:x                      18
	:y                      146
	:width                  352
	:height                 20
	:value-x                51
	:value-y                146
	:slider-width           256
	:ticks                  0
	:foreground-color       ""
	:label                  "min:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              5000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_gradient_min)
		:when                   (Notify )
		:to                     (pop_mask_grow set_gradient_min)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.gradient_min = value;
if(value > grow.gradient_max) {
	grow.gradient_max = value;
	xv_set(Mask_grow_pop_mask_grow->set_gradient_max,
		PANEL_VALUE, value,
		NULL);
}")
		)
	)
)
(
	:type                   :setting
	:name                   set_apply_bridge
	:owner                  controls
	:help                   ""
	:x                      4
	:y                      164
	:width                  123
	:height                 23
	:value-x                59
	:value-y                164
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Bridge:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("apply" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (t )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_apply_bridge)
		:when                   (Notify )
		:to                     (pop_mask_grow set_apply_bridge)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(grow.apply_bridge) {
	grow.apply_bridge = FALSE;
	xv_set(Mask_grow_pop_mask_grow->set_bridge_distance,
		PANEL_INACTIVE, TRUE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_bridge_max,
		PANEL_INACTIVE, TRUE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_bridge_min,
		PANEL_INACTIVE, TRUE,
		NULL);
}
else {
	grow.apply_bridge = TRUE;
	xv_set(Mask_grow_pop_mask_grow->set_bridge_distance,
		PANEL_INACTIVE, FALSE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_bridge_max,
		PANEL_INACTIVE, FALSE,
		NULL);
	xv_set(Mask_grow_pop_mask_grow->set_bridge_min,
		PANEL_INACTIVE, FALSE,
		NULL);
}")
		)
	)
)
(
	:type                   :slider
	:name                   set_bridge_distance
	:owner                  controls
	:help                   ""
	:x                      18
	:y                      190
	:width                  351
	:height                 20
	:value-x                50
	:value-y                190
	:slider-width           256
	:ticks                  0
	:foreground-color       ""
	:label                  "dist:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              100
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          5
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_bridge_distance)
		:when                   (Notify )
		:to                     (pop_mask_grow set_bridge_distance)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.bridge_dist = value;")
		)
	)
)
(
	:type                   :slider
	:name                   set_bridge_max
	:owner                  controls
	:help                   ""
	:x                      18
	:y                      212
	:width                  129
	:height                 20
	:value-x                52
	:value-y                212
	:slider-width           32
	:ticks                  0
	:foreground-color       ""
	:label                  "max:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              1
	:max-value              8
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          8
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_bridge_max)
		:when                   (Notify )
		:to                     (pop_mask_grow set_bridge_max)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.bridge_max = value;
if(value < grow.bridge_min) {
	grow.bridge_min = value;
	xv_set(Mask_grow_pop_mask_grow->set_bridge_min,
		PANEL_VALUE, value,
		NULL);
}")
		)
	)
)
(
	:type                   :slider
	:name                   set_bridge_min
	:owner                  controls
	:help                   ""
	:x                      18
	:y                      236
	:width                  128
	:height                 20
	:value-x                51
	:value-y                236
	:slider-width           32
	:ticks                  0
	:foreground-color       ""
	:label                  "min:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              1
	:max-value              8
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          1
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_bridge_min)
		:when                   (Notify )
		:to                     (pop_mask_grow set_bridge_min)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.bridge_min = value;
if(value > grow.bridge_max) {
	grow.bridge_max = value;
	xv_set(Mask_grow_pop_mask_grow->set_bridge_max,
		PANEL_VALUE, value,
		NULL);
}")
		)
	)
)
(
	:type                   :setting
	:name                   set_extent
	:owner                  controls
	:help                   ""
	:x                      4
	:y                      260
	:width                  343
	:height                 23
	:value-x                109
	:value-y                260
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Growth extent:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("This frame to this frame" "Beginning frame to ending frame" "This frame to all frames" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:choice-defaults        (nil nil nil )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow set_extent "This frame to all frames")
		:when                   (Notify )
		:to                     (pop_mask_grow set_extent "This frame to all frames")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.extent = GROW_ALL;")
		)
		(
		:from                   (pop_mask_grow set_extent "Beginning frame to ending frame")
		:when                   (Notify )
		:to                     (pop_mask_grow beg_frame)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Enable)
		)
		(
		:from                   (pop_mask_grow set_extent "This frame to this frame")
		:when                   (Notify )
		:to                     (pop_mask_grow beg_frame)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow set_extent "This frame to all frames")
		:when                   (Notify )
		:to                     (pop_mask_grow beg_frame)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow set_extent "Beginning frame to ending frame")
		:when                   (Notify )
		:to                     (pop_mask_grow end_frame)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Enable)
		)
		(
		:from                   (pop_mask_grow set_extent "This frame to this frame")
		:when                   (Notify )
		:to                     (pop_mask_grow end_frame)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow set_extent "This frame to all frames")
		:when                   (Notify )
		:to                     (pop_mask_grow end_frame)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow set_extent "This frame to this frame")
		:when                   (Notify )
		:to                     (pop_mask_grow but_shrink)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.extent = GROW_FRAME;")
		)
		(
		:from                   (pop_mask_grow set_extent "Beginning frame to ending frame")
		:when                   (Notify )
		:to                     (pop_mask_grow but_shrink)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.extent = GROW_BEG_TO_END;
region.beg_frame = win[grow.swin].f;
region.end_frame = win[grow.swin].f;
xv_set(Mask_grow_pop_mask_grow->beg_frame,
	PANEL_VALUE, region.beg_frame,
	NULL);
xv_set(Mask_grow_pop_mask_grow->end_frame,
	PANEL_VALUE, region.end_frame,
	NULL);")
		)
	)
)
(
	:type                   :text-field
	:name                   beg_frame
	:owner                  controls
	:help                   ""
	:x                      32
	:y                      288
	:width                  162
	:height                 15
	:value-x                123
	:value-y                288
	:value-length           4
	:stored-length          5
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "From Frame:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              9999
	:min-value              0
	:initial-value          0
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow beg_frame)
		:when                   (Notify )
		:to                     (pop_mask_grow beg_frame)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("region.beg_frame = value;
region.end_frame = value;
xv_set(Mask_grow_pop_mask_grow->end_frame,
	PANEL_VALUE, value,
	NULL);")
		)
	)
)
(
	:type                   :text-field
	:name                   end_frame
	:owner                  controls
	:help                   ""
	:x                      210
	:y                      288
	:width                  145
	:height                 15
	:value-x                284
	:value-y                288
	:value-length           4
	:stored-length          5
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "To Frame:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              9999
	:min-value              0
	:initial-value          0
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow end_frame)
		:when                   (Notify )
		:to                     (pop_mask_grow end_frame)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("region.end_frame = value;")
		)
	)
)
(
	:type                   :button
	:name                   but_grow
	:owner                  controls
	:help                   ""
	:x                      36
	:y                      312
	:width                  50
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Grow"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow but_grow)
		:when                   (Notify )
		:to                     (pop_mask_grow but_grow)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.direction = DIR_GROW;
grow_mask();")
		)
		(
		:from                   (pop_mask_grow but_grow)
		:when                   (Notify )
		:to                     (pop_mask_grow but_stop)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Enable)
		)
		(
		:from                   (pop_mask_grow but_grow)
		:when                   (Notify )
		:to                     (pop_mask_grow but_continue)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow but_grow)
		:when                   (Notify )
		:to                     (pop_mask_grow but_undo)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Enable)
		)
	)
)
(
	:type                   :button
	:name                   but_shrink
	:owner                  controls
	:help                   ""
	:x                      94
	:y                      312
	:width                  57
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Shrink"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow but_shrink)
		:when                   (Notify )
		:to                     (pop_mask_grow but_shrink)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.direction = DIR_SHRINK;
grow_mask();")
		)
		(
		:from                   (pop_mask_grow but_shrink)
		:when                   (Notify )
		:to                     (pop_mask_grow but_stop)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Enable)
		)
		(
		:from                   (pop_mask_grow but_shrink)
		:when                   (Notify )
		:to                     (pop_mask_grow but_continue)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow but_shrink)
		:when                   (Notify )
		:to                     (pop_mask_grow but_undo)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Enable)
		)
	)
)
(
	:type                   :button
	:name                   but_stop
	:owner                  controls
	:help                   ""
	:x                      159
	:y                      312
	:width                  45
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Stop"
	:label-type             :string
	:initial-state          :inactive
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow but_stop)
		:when                   (Notify )
		:to                     (pop_mask_grow but_stop)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow but_stop)
		:when                   (Notify )
		:to                     (pop_mask_grow but_stop)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("notify_set_itimer_func(Mask_grow_pop_mask_grow->pop_mask_grow,
	NOTIFY_FUNC_NULL, ITIMER_REAL, NULL, NULL);")
		)
		(
		:from                   (pop_mask_grow but_stop)
		:when                   (Notify )
		:to                     (pop_mask_grow but_continue)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Enable)
		)
	)
)
(
	:type                   :button
	:name                   but_continue
	:owner                  controls
	:help                   ""
	:x                      212
	:y                      312
	:width                  74
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Continue"
	:label-type             :string
	:initial-state          :inactive
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow but_continue)
		:when                   (Notify )
		:to                     (pop_mask_grow but_stop)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Enable)
		)
		(
		:from                   (pop_mask_grow but_continue)
		:when                   (Notify )
		:to                     (pop_mask_grow but_continue)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow but_continue)
		:when                   (Notify )
		:to                     (pop_mask_grow but_continue)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("go_grow_edges();")
		)
	)
)
(
	:type                   :button
	:name                   but_undo
	:owner                  controls
	:help                   ""
	:x                      294
	:y                      312
	:width                  50
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Undo"
	:label-type             :string
	:initial-state          :inactive
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow but_undo)
		:when                   (Notify )
		:to                     (pop_mask_grow but_stop)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow but_undo)
		:when                   (Notify )
		:to                     (pop_mask_grow but_continue)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow but_undo)
		:when                   (Notify )
		:to                     (pop_mask_grow but_undo)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Disable)
		)
		(
		:from                   (pop_mask_grow but_undo)
		:when                   (Notify )
		:to                     (pop_mask_grow but_undo)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(grow.extent == GROW_FRAME) load_mask_undo_2d(grow.swin);
else load_mask_undo_3d(segal.e_m);")
		)
	)
)
(
	:type                   :button
	:name                   but_set_grow_params
	:owner                  controls
	:help                   ""
	:x                      67
	:y                      336
	:width                  100
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Set params"
	:label-type             :string
	:initial-state          :active
	:menu                   menu_set_grow_parameters
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_options
	:owner                  controls
	:help                   ""
	:x                      175
	:y                      336
	:width                  79
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Options ..."
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_grow but_options)
		:when                   (Notify )
		:to                     (pop_options)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
	)
)
(
	:type                   :button
	:name                   but_close_grow
	:owner                  controls
	:help                   ""
	:x                      262
	:y                      336
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
		:from                   (pop_mask_grow but_close_grow)
		:when                   (Notify )
		:to                     (pop_mask_grow)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (pop_mask_grow but_close_grow)
		:when                   (Notify )
		:to                     (pop_mask_grow but_close_grow)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("if(!grow.stack_empty) redisplay_all();")
		)
		(
		:from                   (pop_mask_grow but_close_grow)
		:when                   (Notify )
		:to                     (pop_options)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_options
	:owner                  win
	:width                  351
	:height                 190
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Mask Growing Options"
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
	:owner                  pop_options
	:help                   ""
	:x                      0
	:y                      0
	:width                  351
	:height                 190
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
	:name                   set_grow_interractive
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      4
	:width                  199
	:height                 23
	:value-x                154
	:value-y                4
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Grow interractively:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Yes" "No" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:choice-defaults        (t nil )
	:initial-selections     (t nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options set_grow_interractive "No")
		:when                   (Notify )
		:to                     (pop_options set_grow_interractive "No")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.interractive = FALSE;")
		)
		(
		:from                   (pop_options set_grow_interractive "Yes")
		:when                   (Notify )
		:to                     (pop_options set_grow_interractive "Yes")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.interractive = TRUE;")
		)
	)
)
(
	:type                   :setting
	:name                   set_starting_window
	:owner                  controls1
	:help                   ""
	:x                      33
	:y                      31
	:width                  264
	:height                 23
	:value-x                154
	:value-y                31
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Starting window:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Paint window" "View window - X" "View window - Y" "View window - Z" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:choice-defaults        (nil nil nil nil )
	:initial-selections     (nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options set_starting_window "Paint window")
		:when                   (Notify )
		:to                     (pop_options set_starting_window "Paint window")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.swin = WIN_PAINT;")
		)
		(
		:from                   (pop_options set_starting_window "View window - X")
		:when                   (Notify )
		:to                     (pop_options set_starting_window "View window - X")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.swin = WIN_VX;")
		)
		(
		:from                   (pop_options set_starting_window "View window - Y")
		:when                   (Notify )
		:to                     (pop_options set_starting_window "View window - Y")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.swin = WIN_VY;")
		)
		(
		:from                   (pop_options set_starting_window "View window - Z")
		:when                   (Notify )
		:to                     (pop_options set_starting_window "View window - Z")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.swin = WIN_VZ;")
		)
	)
)
(
	:type                   :setting
	:name                   set_seed_points_source
	:owner                  controls1
	:help                   ""
	:x                      13
	:y                      58
	:width                  328
	:height                 23
	:value-x                154
	:value-y                58
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Seed points source:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Edit mask" "Point list" "Currently defined region" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:choice-defaults        (nil nil nil )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options set_seed_points_source "Currently defined region")
		:when                   (Notify )
		:to                     (pop_options set_seed_points_source "Currently defined region")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.seed_pt_src = SEED_ROI;")
		)
		(
		:from                   (pop_options set_seed_points_source "Edit mask")
		:when                   (Notify )
		:to                     (pop_options set_seed_points_source "Edit mask")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.seed_pt_src = SEED_EDIT;")
		)
		(
		:from                   (pop_options set_seed_points_source "Point list")
		:when                   (Notify )
		:to                     (pop_options set_seed_points_source "Point list")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.seed_pt_src = SEED_PTS;")
		)
	)
)
(
	:type                   :setting
	:name                   set_overwrite
	:owner                  controls1
	:help                   ""
	:x                      18
	:y                      85
	:width                  193
	:height                 23
	:value-x                154
	:value-y                85
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Growth overwrites:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Yes" "No" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:choice-defaults        (t nil )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options set_overwrite "Yes")
		:when                   (Notify )
		:to                     (pop_mask_grow beg_frame)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.overwrite = TRUE;")
		)
		(
		:from                   (pop_options set_overwrite "No")
		:when                   (Notify )
		:to                     (pop_mask_grow beg_frame)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.overwrite = FALSE;")
		)
	)
)
(
	:type                   :setting
	:name                   set_display_growth
	:owner                  controls1
	:help                   ""
	:x                      39
	:y                      112
	:width                  239
	:height                 23
	:value-x                154
	:value-y                112
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Display growth:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("All windows" "Grow window" "Not until done" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:choice-defaults        (t nil nil )
	:initial-selections     (t nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options set_display_growth "All windows")
		:when                   (Notify )
		:to                     (pop_options set_display_growth)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.disp_growth = DISP_ALL;")
		)
		(
		:from                   (pop_options set_display_growth "Grow window")
		:when                   (Notify )
		:to                     (pop_options set_display_growth)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.disp_growth = DISP_GROW;")
		)
		(
		:from                   (pop_options set_display_growth "Not until done")
		:when                   (Notify )
		:to                     (pop_options set_display_growth)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.disp_growth = DISP_WHEN_DONE;")
		)
	)
)
(
	:type                   :slider
	:name                   set_growth_speed
	:owner                  controls1
	:help                   ""
	:x                      50
	:y                      141
	:width                  278
	:height                 20
	:value-x                149
	:value-y                141
	:slider-width           70
	:ticks                  0
	:foreground-color       ""
	:label                  "Growth speed:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              1
	:max-value              2000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          2000
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options set_growth_speed)
		:when                   (Notify )
		:to                     (pop_options set_growth_speed)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.speed = value;")
		)
	)
)
(
	:type                   :button
	:name                   but_defaults_options
	:owner                  controls1
	:help                   ""
	:x                      140
	:y                      168
	:width                  69
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Defaults"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options but_defaults_options)
		:when                   (Notify )
		:to                     (pop_options but_defaults_options)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("grow.swin = WIN_PAINT;
grow.seed_pt_src = SEED_EDIT;
grow.overwrite = TRUE;
grow.disp_growth = DISP_ALL;
grow.speed = 2000;

/* reset ui fields */
xv_set(Mask_grow_pop_options->set_starting_window,
	PANEL_VALUE, grow.swin,
	NULL);
xv_set(Mask_grow_pop_options->set_seed_points_source,
	PANEL_VALUE, grow.seed_pt_src,
	NULL);
xv_set(Mask_grow_pop_options->set_overwrite,
	PANEL_VALUE, grow.overwrite,
	NULL);
xv_set(Mask_grow_pop_options->set_display_growth,
	PANEL_VALUE, grow.disp_growth,
	NULL);
xv_set(Mask_grow_pop_options->set_growth_speed,
	PANEL_VALUE, grow.speed,
	NULL);")
		)
	)
)
(
	:type                   :button
	:name                   but_close_options
	:owner                  controls1
	:help                   ""
	:x                      294
	:y                      168
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
		:from                   (pop_options but_close_options)
		:when                   (Notify )
		:to                     (pop_options)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
