;GIL-3
(
(
	:type                   :popup-window
	:name                   ctrlwin
	:owner                  nil
	:width                  427
	:height                 184
	:background-color       ""
	:foreground-color       ""
	:label                  "Display Control"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              nil
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  ctrlwin
	:help                   ""
	:x                      0
	:y                      0
	:width                  427
	:height                 184
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
	:name                   message2
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      16
	:width                  34
	:height                 13
	:foreground-color       ""
	:label                  "Note:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   message1
	:owner                  controls1
	:help                   ""
	:x                      24
	:y                      44
	:width                  379
	:height                 13
	:foreground-color       ""
	:label                  "These settings affect only the DISPLAY image, not the dataset."
	:label-type             :string
	:label-bold             nil
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   gamma
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      80
	:width                  253
	:height                 20
	:value-x                77
	:value-y                80
	:slider-width           150
	:ticks                  0
	:foreground-color       ""
	:label                  "Contrast:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             nil
	:show-value             nil
	:min-value              0
	:max-value              100
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         gamma_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (ctrlwin gamma)
		:when                   (Notify )
		:to                     (ctrlwin gamma)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gamma_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   cmap_min
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      108
	:width                  108
	:height                 15
	:value-x                53
	:value-y                108
	:value-length           4
	:stored-length          8
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         cm_min_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (ctrlwin cmap_min)
		:when                   (Notify )
		:to                     (ctrlwin cmap_min)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cm_min_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   cmap_max
	:owner                  controls1
	:help                   "Minimun values for color map.
"
	:x                      140
	:y                      108
	:width                  109
	:height                 15
	:value-x                178
	:value-y                108
	:value-length           4
	:stored-length          8
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         cm_max_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (ctrlwin cmap_max)
		:when                   (Notify )
		:to                     (ctrlwin cmap_max)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cm_max_proc)
		)
	)
)
(
	:type                   :setting
	:name                   shrink_fac
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      144
	:width                  145
	:height                 23
	:value-x                117
	:value-y                144
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Shrink Factor:"
	:label-type             :string
	:notify-handler         shrink_proc
	:event-handler          nil
	:choices                ("1" "2" "4" "8" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:choice-defaults        (nil nil nil nil )
	:initial-selections     (nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (ctrlwin shrink_fac)
		:when                   (Notify )
		:to                     (ctrlwin shrink_fac)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (shrink_proc)
		)
	)
)
)
