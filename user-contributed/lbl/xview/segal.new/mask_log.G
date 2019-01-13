;GIL-3
(
(
	:type                   :menu
	:name                   menu_mask_unload
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "Unload"
	:menu-item-labels       ("Current selection" "All" )
	:menu-item-label-types  (:string :string )
	:menu-item-states       (:active :active )
	:menu-item-defaults     (t nil )
	:initial-selections     (nil nil )
	:menu-item-handlers     (nil nil )
	:menu-item-menus        (nil nil )
	:menu-item-colors       ("" "" )
	:pinnable               nil
	:user-data              ()
	:actions                (
		(
		:from                   (menu_mask_unload "All")
		:when                   (Notify )
		:to                     (menu_mask_unload "All")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("unload_all_masks(TRUE);")
		)
		(
		:from                   (menu_mask_unload "Current selection")
		:when                   (Notify )
		:to                     (menu_mask_unload "Current selection")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("unload_mask(TRUE);")
		)
	)
)
(
	:type                   :menu
	:name                   menu_mask
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Load ..." "New ..." "Save" "Save as ..." "Unload" )
	:menu-item-label-types  (:string :string :string :string :string )
	:menu-item-states       (:active :active :active :active :active )
	:menu-item-defaults     (t nil nil nil nil )
	:initial-selections     (nil nil nil nil nil )
	:menu-item-handlers     (nil nil nil nil nil )
	:menu-item-menus        (nil nil nil nil menu_mask_unload )
	:menu-item-colors       ("" "" "" "" "" )
	:pinnable               nil
	:user-data              ()
	:actions                (
		(
		:from                   (menu_mask "Save")
		:when                   (Notify )
		:to                     (menu_mask "Save")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("save_mask();")
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_mask_log
	:owner                  win
	:width                  382
	:height                 259
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Mask Log"
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
	:owner                  pop_mask_log
	:help                   ""
	:x                      0
	:y                      0
	:width                  382
	:height                 259
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          edit_mask_defined
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_log controls)
		:when                   (AnyEvent )
		:to                     (pop_mask_log controls)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (edit_mask_defined)
		)
	)
)
(
	:type                   :button
	:name                   but_mask
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      8
	:width                  63
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Mask"
	:label-type             :string
	:initial-state          :active
	:menu                   menu_mask
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_masks_loaded
	:owner                  controls
	:help                   ""
	:x                      244
	:y                      12
	:width                  111
	:height                 13
	:foreground-color       ""
	:label                  "Masks Loaded: 0"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :scrolling-list
	:name                   ls_mask_filenames
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      32
	:width                  350
	:height                 164
	:value-x                8
	:value-y                32
	:rows                   8
	:foreground-color       ""
	:label                  ""
	:title                  ""
	:label-type             :string
	:layout-type            :horizontal
	:read-only              nil
	:multiple-selections    nil
	:selection-required     nil
	:initial-state          :active
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_log ls_mask_filenames)
		:when                   (Notify )
		:to                     (pop_mask_log ls_mask_filenames)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("select_mask_in_list(string);")
		)
	)
)
(
	:type                   :setting
	:name                   set_mask_type
	:owner                  controls
	:help                   ""
	:x                      12
	:y                      204
	:width                  351
	:height                 23
	:value-x                93
	:value-y                204
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Mask type:"
	:label-type             :string
	:notify-handler         set_mask_type
	:event-handler          nil
	:choices                ("Edit" "Not Applied" "Inclusive" "Exclusive" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:initial-selections     (nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_mask_log set_mask_type)
		:when                   (Notify )
		:to                     (pop_mask_log set_mask_type)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_mask_type)
		)
	)
)
(
	:type                   :button
	:name                   but_options
	:owner                  controls
	:help                   ""
	:x                      152
	:y                      236
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
		:from                   (pop_mask_log but_options)
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
	:name                   but_close_mask_log
	:owner                  controls
	:help                   ""
	:x                      312
	:y                      236
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
		:from                   (pop_mask_log but_close_mask_log)
		:when                   (Notify )
		:to                     (pop_mask_log)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (pop_mask_log but_close_mask_log)
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
	:width                  234
	:height                 91
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Mask Log Options"
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
	:owner                  pop_options
	:help                   ""
	:x                      0
	:y                      0
	:width                  234
	:height                 91
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
	:name                   set_apply_log
	:owner                  controls
	:help                   ""
	:x                      19
	:y                      8
	:width                  133
	:height                 23
	:value-x                95
	:value-y                8
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Apply log:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Yes" "No" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:choice-defaults        (nil nil )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options set_apply_log "No")
		:when                   (Notify )
		:to                     (pop_options set_apply_log "No")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("mlog.apply_log = FALSE;")
		)
		(
		:from                   (pop_options set_apply_log "Yes")
		:when                   (Notify )
		:to                     (pop_options set_apply_log "Yes")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("mlog.apply_log = TRUE;")
		)
	)
)
(
	:type                   :setting
	:name                   set_apply_order
	:owner                  controls
	:help                   ""
	:x                      4
	:y                      35
	:width                  226
	:height                 23
	:value-x                95
	:value-y                35
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Apply order:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Exclude, include" "Include, exclude" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:choice-defaults        (nil nil )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_options set_apply_order "Exclude, include")
		:when                   (Notify )
		:to                     (pop_options set_apply_order "Exclude, include")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("mlog.apply_order = ORDER_EI;")
		)
		(
		:from                   (pop_options set_apply_order "Include, exclude")
		:when                   (Notify )
		:to                     (pop_options set_apply_order "Include, exclude")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("mlog.apply_order = ORDER_IE;")
		)
	)
)
(
	:type                   :button
	:name                   but_default
	:owner                  controls
	:help                   ""
	:x                      86
	:y                      68
	:width                  63
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Default"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_close_options
	:owner                  controls
	:help                   ""
	:x                      172
	:y                      68
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
