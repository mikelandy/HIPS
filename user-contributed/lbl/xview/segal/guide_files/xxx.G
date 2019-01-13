;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  458
	:height                 253
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Mask Log"
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
	:name                   controls
	:owner                  win
	:help                   ""
	:x                      0
	:y                      0
	:width                  458
	:height                 253
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
	:name                   msg_mask_filenames
	:owner                  controls
	:help                   ""
	:x                      20
	:y                      8
	:width                  109
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
	:width                  100
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
	:width                  356
	:height                 23
	:value-x                106
	:value-y                220
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Mask Type:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Not Applied" "Edit" "Exclusive" "Inclusive" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:user-data              ()
)
)
