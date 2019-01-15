;GIL-3
(
(
	:type                   :popup-window
	:name                   pop_preferences_display
	:owner                  win
	:width                  373
	:height                 221
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Display Preferences"
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
	:owner                  pop_preferences_display
	:help                   ""
	:x                      0
	:y                      0
	:width                  373
	:height                 221
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
	:type                   :slider
	:name                   set_image_contrast
	:owner                  controls
	:help                   ""
	:x                      16
	:y                      16
	:width                  264
	:height                 20
	:value-x                121
	:value-y                16
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Image contrast:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             nil
	:min-value              0
	:max-value              100
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          25
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_preferences_display set_image_contrast)
		:when                   (Notify )
		:to                     (pop_preferences_display set_image_contrast)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("cmap_init();
build_cmap();
redisplay_all();")
		)
	)
)
(
	:type                   :slider
	:name                   set_image_opacity
	:owner                  controls
	:help                   ""
	:x                      22
	:y                      42
	:width                  258
	:height                 20
	:value-x                121
	:value-y                42
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Image opacity:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             nil
	:min-value              0
	:max-value              100
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
		:from                   (pop_preferences_display set_image_opacity)
		:when                   (Notify )
		:to                     (pop_preferences_display set_image_opacity)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("cmap_init();
build_cmap();
redisplay_all();")
		)
	)
)
(
	:type                   :slider
	:name                   set_mask_opacity
	:owner                  controls
	:help                   ""
	:x                      28
	:y                      68
	:width                  252
	:height                 20
	:value-x                121
	:value-y                68
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Mask opacity:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             nil
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
	:actions                (
		(
		:from                   (pop_preferences_display set_mask_opacity)
		:when                   (Notify )
		:to                     (pop_preferences_display set_mask_opacity)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("cmap_init();
build_cmap();
redisplay_all();")
		)
	)
)
(
	:type                   :message
	:name                   msg_quantizing
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      94
	:width                  248
	:height                 13
	:foreground-color       ""
	:label                  "Quantizing ---------------------"
	:label-type             :string
	:label-bold             t
	:initial-state          :inactive
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   set_requant_win
	:owner                  controls
	:help                   ""
	:x                      32
	:y                      116
	:width                  286
	:height                 23
	:value-x                96
	:value-y                116
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Window:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("View X" "View Y" "View Z" "Paint" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:initial-selections     (nil nil t nil )
	:initial-state          :inactive
	:user-data              ()
	:actions                (
		(
		:from                   (pop_preferences_display set_requant_win "Paint")
		:when                   (Notify )
		:to                     (pop_preferences_display set_requant_win "Paint")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("rgb.quant.win = WIN_PAINT;")
		)
		(
		:from                   (pop_preferences_display set_requant_win "View X")
		:when                   (Notify )
		:to                     (pop_preferences_display set_requant_win "View X")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("rgb.quant.win = WIN_VX;")
		)
		(
		:from                   (pop_preferences_display set_requant_win "View Y")
		:when                   (Notify )
		:to                     (pop_preferences_display set_requant_win "View Y")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("rgb.quant.win = WIN_VY;")
		)
		(
		:from                   (pop_preferences_display set_requant_win "View Z")
		:when                   (Notify )
		:to                     (pop_preferences_display set_requant_win "View Z")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("rgb.quant.win = WIN_VZ;")
		)
	)
)
(
	:type                   :setting
	:name                   set_requant_quality
	:owner                  controls
	:help                   ""
	:x                      32
	:y                      142
	:width                  333
	:height                 23
	:value-x                94
	:value-y                142
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Quality:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Best (Slowest)" "Medium" "Worst (Fastest)" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:initial-selections     (nil nil nil )
	:initial-state          :inactive
	:user-data              ()
	:actions                (
		(
		:from                   (pop_preferences_display set_requant_quality "Best (Slowest)")
		:when                   (Notify )
		:to                     (pop_preferences_display set_requant_quality "Best (Slowest)")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("rgb.quant.quality = Q_BEST;")
		)
		(
		:from                   (pop_preferences_display set_requant_quality "Medium")
		:when                   (Notify )
		:to                     (pop_preferences_display set_requant_quality "Medium")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("rgb.quant.quality = Q_MEDIUM;")
		)
		(
		:from                   (pop_preferences_display set_requant_quality "Worst (Fastest)")
		:when                   (Notify )
		:to                     (pop_preferences_display set_requant_quality "Worst (Fastest)")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("rgb.quant.quality = Q_WORST;")
		)
	)
)
(
	:type                   :button
	:name                   but_requant
	:owner                  controls
	:help                   ""
	:x                      34
	:y                      172
	:width                  89
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "ReQuantize"
	:label-type             :string
	:initial-state          :inactive
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_preferences_display but_requant)
		:when                   (Notify )
		:to                     (pop_preferences_display but_requant)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("quantize();")
		)
	)
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      161
	:y                      192
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
		:from                   (pop_preferences_display but_close)
		:when                   (Notify )
		:to                     (pop_preferences_display)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
