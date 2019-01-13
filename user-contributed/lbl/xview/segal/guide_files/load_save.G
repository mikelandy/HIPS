;GIL-2
(
(
	:type                   :popup-window
	:name                   pop_load
	:owner                  nil
	:width                  413
	:height                 126
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal : Load"
	:label-type             :string
	:mapped                 nil
	:show-footer            t
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls_load
	:owner                  pop_load
	:help                   ""
	:x                      0
	:y                      0
	:width                  413
	:height                 126
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
	:name                   load_which
	:owner                  controls_load
	:help                   ""
	:x                      44
	:y                      8
	:width                  180
	:height                 23
	:value-x                87
	:value-y                8
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Load:"
	:label-type             :string
	:notify-handler         set_load_which
	:event-handler          nil
	:events                 ()
	:choices                ("Image" "Mask" "List" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:user-data              ()
)
(
	:type                   :text-field
	:name                   load_dname
	:owner                  controls_load
	:help                   ""
	:x                      12
	:y                      44
	:width                  394
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Directory:"
	:label-type             :string
	:value-x                87
	:value-y                44
	:layout-type            :horizontal
	:value-length           40
	:stored-length          100
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   load_fname
	:owner                  controls_load
	:help                   ""
	:x                      12
	:y                      76
	:width                  394
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Filename:"
	:label-type             :string
	:value-x                87
	:value-y                76
	:layout-type            :horizontal
	:value-length           40
	:stored-length          100
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_load
	:owner                  controls_load
	:help                   ""
	:x                      200
	:y                      104
	:constant-width         nil
	:button-type            :normal
	:width                  46
	:height                 19
	:foreground-color       ""
	:label                  "Load"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_load_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :popup-window
	:name                   pop_create
	:owner                  nil
	:width                  418
	:height                 93
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal : Create Mask"
	:label-type             :string
	:mapped                 nil
	:show-footer            t
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls_create
	:owner                  pop_create
	:help                   ""
	:x                      0
	:y                      0
	:width                  418
	:height                 93
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
	:name                   create_dname
	:owner                  controls_create
	:help                   ""
	:x                      16
	:y                      12
	:width                  394
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Directory:"
	:label-type             :string
	:value-x                91
	:value-y                12
	:layout-type            :horizontal
	:value-length           40
	:stored-length          100
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   create_fname
	:owner                  controls_create
	:help                   ""
	:x                      16
	:y                      44
	:width                  394
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Filename:"
	:label-type             :string
	:value-x                91
	:value-y                44
	:layout-type            :horizontal
	:value-length           40
	:stored-length          100
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_create
	:owner                  controls_create
	:help                   ""
	:x                      192
	:y                      72
	:constant-width         nil
	:button-type            :normal
	:width                  58
	:height                 19
	:foreground-color       ""
	:label                  "Create"
	:label-type             :string
	:menu                   nil
	:notify-handler         but_create_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
