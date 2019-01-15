;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  349
	:height                 174
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Filter Apply"
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
	:width                  349
	:height                 174
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
	:name                   filter_name
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      24
	:width                  330
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Filter Name:"
	:label-type             :string
	:value-x                102
	:value-y                24
	:layout-type            :horizontal
	:value-length           30
	:stored-length          100
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   filter_parameters
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      48
	:width                  153
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Parameters:"
	:label-type             :string
	:value-x                101
	:value-y                48
	:layout-type            :horizontal
	:value-length           30
	:stored-length          100
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_source_buf
	:owner                  controls
	:help                   ""
	:x                      52
	:y                      68
	:width                  200
	:height                 23
	:value-x                124
	:value-y                68
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Apply To:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Image" "Edit Mask" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_dest_buf
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      100
	:width                  240
	:height                 23
	:value-x                124
	:value-y                100
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Store Result In:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Image" "Edit Mask" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :button
	:name                   but_apply
	:owner                  controls
	:help                   ""
	:x                      16
	:y                      132
	:constant-width         nil
	:button-type            :normal
	:width                  53
	:height                 19
	:foreground-color       ""
	:label                  "Apply"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_apply_filter_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      152
	:y                      148
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_filter
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
