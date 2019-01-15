;GIL-2
(
(
	:type                   :menu
	:name                   mask_menu
	:help                   ""
	:columns                1
	:label                  ""
	:label-type             :string
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "Mask Operations Menu"
	:menu-item-labels       ("Fill Mask" "Clear Mask" "Invert Mask" )
	:menu-item-label-types  (:string :string :string )
	:menu-item-defaults     (nil nil nil )
	:menu-item-handlers     (nil nil nil )
	:menu-item-menus        (nil nil nil )
	:menu-item-colors       ("" "" "" )
	:pinnable               t
	:user-data              ()
)
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  416
	:height                 375
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal : Mask Control"
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
	:width                  416
	:height                 375
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
	:name                   display_type
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      8
	:width                  207
	:height                 23
	:value-x                72
	:value-y                8
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Display:"
	:label-type             :string
	:notify-handler         change_image_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Image" "Mask" "Both" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:user-data              ()
)
(
	:type                   :button
	:name                   but_threshold
	:owner                  controls
	:help                   ""
	:x                      326
	:y                      12
	:constant-width         nil
	:button-type            :normal
	:width                  79
	:height                 19
	:foreground-color       ""
	:label                  "Threshold"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_threshold
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_fill
	:owner                  controls
	:help                   ""
	:x                      9
	:y                      44
	:constant-width         nil
	:button-type            :normal
	:width                  37
	:height                 19
	:foreground-color       ""
	:label                  "Fill"
	:label-type             :string
	:menu                   nil
	:notify-handler         mask_fill_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_clear
	:owner                  controls
	:help                   ""
	:x                      56
	:y                      44
	:constant-width         nil
	:button-type            :normal
	:width                  50
	:height                 19
	:foreground-color       ""
	:label                  "Clear"
	:label-type             :string
	:menu                   nil
	:notify-handler         mask_clear_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_invert
	:owner                  controls
	:help                   ""
	:x                      116
	:y                      44
	:constant-width         nil
	:button-type            :normal
	:width                  54
	:height                 19
	:foreground-color       ""
	:label                  "Invert"
	:label-type             :string
	:menu                   nil
	:notify-handler         mask_invert_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_poly
	:owner                  controls
	:help                   ""
	:x                      304
	:y                      44
	:constant-width         nil
	:button-type            :normal
	:width                  101
	:height                 19
	:foreground-color       ""
	:label                  "Polygon Mask"
	:label-type             :string
	:menu                   nil
	:notify-handler         poly_mask_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_growth
	:owner                  controls
	:help                   ""
	:x                      10
	:y                      68
	:width                  395
	:height                 13
	:foreground-color       ""
	:label                  "-------------------Growth Options-----------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_original
	:owner                  controls
	:help                   ""
	:x                      8
	:y                      84
	:constant-width         nil
	:button-type            :normal
	:width                  67
	:height                 19
	:foreground-color       ""
	:label                  "Original"
	:label-type             :string
	:menu                   nil
	:notify-handler         original_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_undo
	:owner                  controls
	:help                   ""
	:x                      88
	:y                      84
	:constant-width         nil
	:button-type            :normal
	:width                  50
	:height                 19
	:foreground-color       ""
	:label                  "Undo"
	:label-type             :string
	:menu                   nil
	:notify-handler         undo_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_stop
	:owner                  controls
	:help                   ""
	:x                      296
	:y                      84
	:constant-width         nil
	:button-type            :normal
	:width                  45
	:height                 19
	:foreground-color       ""
	:label                  "Stop"
	:label-type             :string
	:menu                   nil
	:notify-handler         nil
	:event-handler          grow_stop_proc
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_grow
	:owner                  controls
	:help                   ""
	:x                      354
	:y                      84
	:constant-width         nil
	:button-type            :normal
	:width                  50
	:height                 19
	:foreground-color       ""
	:label                  "Grow"
	:label-type             :string
	:menu                   nil
	:notify-handler         grow_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_portion
	:owner                  controls
	:help                   ""
	:x                      36
	:y                      110
	:width                  170
	:height                 23
	:value-x                97
	:value-y                110
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Portion:"
	:label-type             :string
	:notify-handler         portion_set_proc
	:event-handler          nil
	:events                 ()
	:choices                ("Whole" "Region" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_extent_3D
	:owner                  controls
	:help                   ""
	:x                      42
	:y                      136
	:width                  211
	:height                 23
	:value-x                95
	:value-y                136
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  "Extent:"
	:label-type             :string
	:notify-handler         set_grow_extent_3D
	:event-handler          nil
	:events                 ()
	:choices                ("Grow In" "Grow In, Out" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_region
	:owner                  controls
	:help                   ""
	:x                      50
	:y                      164
	:width                  295
	:height                 23
	:value-x                95
	:value-y                164
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:rows                   1
	:columns                0
	:label                  "From:"
	:label-type             :string
	:notify-handler         draw_hints_grow_region
	:event-handler          nil
	:events                 ()
	:choices                ("Seed Points In This Frame" "Beginning Frame to Ending Frame" "This Frame --to-> Previous Frame" "This Frame --to-> Next Frame" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:user-data              ()
)
(
	:type                   :text-field
	:name                   beg_frame
	:owner                  controls
	:help                   ""
	:x                      4
	:y                      188
	:width                  155
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "From Frame:"
	:label-type             :string
	:value-x                95
	:value-y                188
	:layout-type            :horizontal
	:value-length           8
	:stored-length          80
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   end_frame
	:owner                  controls
	:help                   ""
	:x                      170
	:y                      188
	:width                  138
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "To Frame:"
	:label-type             :string
	:value-x                244
	:value-y                188
	:layout-type            :horizontal
	:value-length           8
	:stored-length          80
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   grow_gradient
	:owner                  controls
	:help                   ""
	:x                      25
	:y                      216
	:width                  383
	:height                 20
	:value-x                94
	:value-y                216
	:slider-width           256
	:foreground-color       ""
	:label                  "Gradient:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              255
	:ticks                  0
	:notify-handler         set_grow_gradient
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_matrix
	:owner                  controls
	:help                   ""
	:x                      10
	:y                      236
	:width                  395
	:height                 13
	:foreground-color       ""
	:label                  "Matrix--------------------------------------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_matrix_type
	:owner                  controls
	:help                   ""
	:x                      50
	:y                      248
	:width                  258
	:height                 23
	:value-x                95
	:value-y                248
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:rows                   1
	:columns                0
	:label                  "Type:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Simple Gradient" "Sobel (3x3 max)  " "Laplacian (3x3 max)" "Kasvand Laplacian (5x5 max)" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_apply_in
	:owner                  controls
	:help                   ""
	:x                      266
	:y                      248
	:width                  143
	:height                 23
	:value-x                335
	:value-y                248
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:rows                   1
	:columns                0
	:label                  "Apply In:"
	:label-type             :string
	:notify-handler         set_grow_apply_in
	:event-handler          nil
	:events                 ()
	:choices                ("Frame" "3-d" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_matrix_size
	:owner                  controls
	:help                   ""
	:x                      56
	:y                      268
	:width                  106
	:height                 23
	:value-x                95
	:value-y                268
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:rows                   1
	:columns                0
	:label                  "Size:"
	:label-type             :string
	:notify-handler         set_grow_matrix_size
	:event-handler          nil
	:events                 ()
	:choices                ("3 x 3" "5 x 5" "7 x 7" "9 x 9" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_threshold
	:owner                  controls
	:help                   ""
	:x                      10
	:y                      292
	:width                  393
	:height                 13
	:foreground-color       ""
	:label                  "Threshold Values----------------------------------"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   grow_threshold_high
	:owner                  controls
	:help                   ""
	:x                      37
	:y                      310
	:width                  367
	:height                 20
	:value-x                90
	:value-y                310
	:slider-width           256
	:foreground-color       ""
	:label                  "Upper:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              255
	:ticks                  0
	:notify-handler         set_grow_threshold_high
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   grow_threshold_low
	:owner                  controls
	:help                   ""
	:x                      37
	:y                      330
	:width                  366
	:height                 20
	:value-x                89
	:value-y                330
	:slider-width           256
	:foreground-color       ""
	:label                  "Lower:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              255
	:ticks                  0
	:notify-handler         set_grow_threshold_low
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls
	:help                   ""
	:x                      190
	:y                      352
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_mask_control
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :popup-window
	:name                   pop_polygon
	:owner                  win
	:width                  400
	:height                 132
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal : Polygon Mask"
	:label-type             :string
	:mapped                 nil
	:show-footer            nil
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :control-area
	:name                   controls_poly
	:owner                  pop_polygon
	:help                   ""
	:x                      0
	:y                      0
	:width                  400
	:height                 132
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
	:name                   msg_directions
	:owner                  controls_poly
	:help                   ""
	:x                      24
	:y                      24
	:width                  353
	:height                 13
	:foreground-color       ""
	:label                  "Select Points for the Polygon, then select a function"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :setting
	:name                   set_polyfunct
	:owner                  controls_poly
	:help                   ""
	:x                      52
	:y                      56
	:width                  132
	:height                 23
	:value-x                61
	:value-y                56
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:rows                   1
	:columns                0
	:label                  ""
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:choices                ("Show" "Fill" "Clear" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:user-data              ()
)
(
	:type                   :message
	:name                   msg_poly
	:owner                  controls_poly
	:help                   ""
	:x                      208
	:y                      60
	:width                  135
	:height                 13
	:foreground-color       ""
	:label                  "The Polygon Region"
	:label-type             :string
	:label-bold             t
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_apply
	:owner                  controls_poly
	:help                   ""
	:x                      168
	:y                      96
	:constant-width         nil
	:button-type            :normal
	:width                  53
	:height                 19
	:foreground-color       ""
	:label                  "Apply"
	:label-type             :string
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
