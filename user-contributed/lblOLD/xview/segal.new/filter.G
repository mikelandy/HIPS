;GIL-3
(
(
	:type                   :popup-window
	:name                   pop_filter
	:owner                  win
	:width                  346
	:height                 148
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Filter Apply"
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
	:name                   controls
	:owner                  pop_filter
	:help                   ""
	:x                      0
	:y                      0
	:width                  346
	:height                 148
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
	:type                   :text-field
	:name                   filter_name
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      8
	:width                  330
	:height                 15
	:value-x                98
	:value-y                8
	:value-length           30
	:stored-length          100
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Filter Name:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   filter_parameters
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      32
	:width                  329
	:height                 15
	:value-x                97
	:value-y                32
	:value-length           30
	:stored-length          100
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Parameters:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   set_source_buf
	:owner                  controls
	:help                   ""
	:x                      52
	:y                      56
	:width                  200
	:height                 23
	:value-x                124
	:value-y                56
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Apply To:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Image" "Edit Mask" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :inactive
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   set_dest_buf
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      88
	:width                  240
	:height                 23
	:value-x                124
	:value-y                88
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Store Result In:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Image" "Edit Mask" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :inactive
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_apply
	:owner                  controls
	:help                   ""
	:x                      148
	:y                      124
	:width                  53
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Apply"
	:label-type             :string
	:initial-state          :inactive
	:menu                   nil
	:notify-handler         but_apply_filter_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_filter but_apply)
		:when                   (Notify )
		:to                     (pop_filter but_apply)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (but_apply_filter_proc)
		)
	)
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      288
	:y                      124
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
		:from                   (pop_filter but_close)
		:when                   (Notify )
		:to                     (pop_filter)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
