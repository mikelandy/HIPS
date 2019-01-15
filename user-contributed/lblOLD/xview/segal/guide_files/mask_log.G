;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  458
	:height                 327
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Mask Log"
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
	:width                  458
	:height                 327
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          edit_mask_defined
	:events                 (:mouse-exit )
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_mask_filenames
	:owner                  controls
	:help                   ""
	:x                      20
	:y                      8
	:width                  108
	:height                 13
	:foreground-color       ""
	:label                  "Mask File Name"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_masks_loaded
	:owner                  controls
	:help                   ""
	:x                      280
	:y                      8
	:width                  99
	:height                 13
	:foreground-color       ""
	:label                  "Masks Loaded:"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :scrolling-list
	:name                   ls_mask_filenames
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      28
	:width                  400
	:height                 182
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:layout-type            :horizontal
	:rows                   9
	:read-only              nil
	:multiple-selections    nil
	:selection-required     nil
	:menu                   nil
	:notify-handler         update_mask_log_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_mask_type
	:owner                  controls
	:help                   ""
	:x                      20
	:y                      220
	:width                  355
	:height                 23
	:value-x                105
	:value-y                220
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Mask Type:"
	:label-type             :string
	:notify-handler         set_mask_type
	:event-handler          nil
	:events                 ()
	:choices                ("Not Applied" "Edit" "Exclusive" "Inclusive" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_which_apply_first
	:owner                  controls
	:help                   ""
	:x                      54
	:y                      248
	:width                  317
	:height                 23
	:value-x                105
	:value-y                248
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Apply:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Exclusive, Inclusive" "Inclusive, Exclusive" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_mask_color
	:owner                  controls
	:help                   ""
	:x                      18
	:y                      276
	:width                  277
	:height                 23
	:value-x                105
	:value-y                276
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Mask Color:"
	:label-type             :string
	:notify-handler         set_mask_color
	:event-handler          nil
	:events                 ()
	:choices                ("Green" "Red" "Yellow" "Blue" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      200
	:y                      304
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_mask_log
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
