;GIL-3
(
(
	:type                   :menu
	:name                   menu_edit_mask
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "Edit mask"
	:menu-item-labels       ("Cut" "Copy" "Paste" "Undo" "Original" "" "Clear frame" "Clear whole mask" "Fill frame" "Fill whole mask" "Invert frame" "Invert whole mask" )
	:menu-item-label-types  (:string :string :string :string :string :string :string :string :string :string :string :string )
	:menu-item-states       (:active :active :active :active :active :active :active :active :active :active :active :active )
	:menu-item-defaults     (nil nil nil nil nil nil nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil nil nil nil nil nil nil )
	:menu-item-handlers     (nil nil nil nil nil nil nil nil nil nil nil nil )
	:menu-item-menus        (nil nil nil nil nil nil nil nil nil nil nil nil )
	:menu-item-colors       ("" "" "" "" "" "" "" "" "" "" "" "" )
	:pinnable               t
	:user-data              ()
	:actions                (
		(
		:from                   (menu_edit_mask "Copy")
		:when                   (Notify )
		:to                     (menu_edit_mask "Copy")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("copy_mask(win[WIN_PAINT].m_data[0], bm,
	m[segal.e_m].bit_key, bm_key[win[WIN_PAINT].aspect + 3],
	win[WIN_PAINT].img_size);")
		)
		(
		:from                   (menu_edit_mask "Fill frame")
		:when                   (Notify )
		:to                     (menu_edit_mask "Fill frame")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("fill_mask(R2d_WHOLE, WIN_PAINT, WHITE);")
		)
		(
		:from                   (menu_edit_mask "Clear frame")
		:when                   (Notify )
		:to                     (menu_edit_mask "Clear frame")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("fill_mask(R2d_WHOLE, WIN_PAINT, BLACK);")
		)
		(
		:from                   (menu_edit_mask "Clear whole mask")
		:when                   (Notify )
		:to                     (menu_edit_mask "Clear whole mask")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("fill_mask(R3d_WHOLE, WIN_PAINT, BLACK);")
		)
		(
		:from                   (menu_edit_mask "Fill whole mask")
		:when                   (Notify )
		:to                     (menu_edit_mask "Fill whole mask")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("fill_mask(R3d_WHOLE, WIN_PAINT, WHITE);")
		)
		(
		:from                   (menu_edit_mask "Cut")
		:when                   (Notify )
		:to                     (menu_edit_mask "Cut")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("copy_mask(win[WIN_PAINT].m_data[0], bm,
	m[segal.e_m].bit_key, bm_key[win[WIN_PAINT].aspect + 3],
	win[WIN_PAINT].img_size);
fill_mask(R2d_WHOLE, WIN_PAINT, BLACK);")
		)
		(
		:from                   (menu_edit_mask "Invert frame")
		:when                   (Notify )
		:to                     (menu_edit_mask "Invert frame")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("fill_mask(R2d_WHOLE, WIN_PAINT, INVERT);")
		)
		(
		:from                   (menu_edit_mask "Invert whole mask")
		:when                   (Notify )
		:to                     (menu_edit_mask "Invert whole mask")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("fill_mask(R3d_WHOLE, WIN_PAINT, INVERT);")
		)
		(
		:from                   (menu_edit_mask "Undo")
		:when                   (Notify )
		:to                     (menu_edit_mask "Undo")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("load_mask_undo_2d(WIN_PAINT);")
		)
		(
		:from                   (menu_edit_mask "Paste")
		:when                   (Notify )
		:to                     (menu_edit_mask "Paste")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("copy_mask(bm, win[WIN_PAINT].m_data[0],
	bm_key[win[WIN_PAINT].aspect + 3], m[segal.e_m].bit_key,
	win[WIN_PAINT].img_size);
m[segal.e_m].changed_frame = TRUE;
save_mask_frame(WIN_PAINT);
redisplay_paint();")
		)
	)
)
(
	:type                   :menu
	:name                   menu_edit_image
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "Edit image"
	:menu-item-labels       ("Undo - Frame" "Original - Frame" )
	:menu-item-label-types  (:string :string )
	:menu-item-states       (:active :active )
	:menu-item-defaults     (nil nil )
	:initial-selections     (nil nil )
	:menu-item-handlers     (nil nil )
	:menu-item-menus        (nil nil )
	:menu-item-colors       ("" "" )
	:pinnable               t
	:user-data              ()
	:actions                (
		(
		:from                   (menu_edit_image "Original - Frame")
		:when                   (Notify )
		:to                     (menu_edit_image "Original - Frame")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("load_image_orig(WIN_PAINT);
redisplay_all();")
		)
		(
		:from                   (menu_edit_image "Undo - Frame")
		:when                   (Notify )
		:to                     (menu_edit_image "Undo - Frame")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("load_image_undo(WIN_PAINT);
redisplay_all();")
		)
	)
)
(
	:type                   :menu
	:name                   menu_edit
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Mask" "Image" )
	:menu-item-label-types  (:string :string )
	:menu-item-states       (:active :active )
	:menu-item-defaults     (nil nil )
	:initial-selections     (nil nil )
	:menu-item-handlers     (nil nil )
	:menu-item-menus        (menu_edit_mask menu_edit_image )
	:menu-item-colors       ("" "" )
	:pinnable               nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :menu
	:name                   menu_apply_log
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("This frame" "Whole image" )
	:menu-item-label-types  (:string :string )
	:menu-item-states       (:active :active )
	:menu-item-defaults     (nil nil )
	:initial-selections     (nil nil )
	:menu-item-handlers     (nil nil )
	:menu-item-menus        (nil nil )
	:menu-item-colors       ("" "" )
	:pinnable               nil
	:user-data              ()
	:actions                (
		(
		:from                   (menu_apply_log "This frame")
		:when                   (Notify )
		:to                     (menu_apply_log "This frame")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("apply_log(R2d_WHOLE);")
		)
		(
		:from                   (menu_apply_log "Whole image")
		:when                   (Notify )
		:to                     (menu_apply_log "Whole image")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("apply_log(R3d_WHOLE);")
		)
	)
)
(
	:type                   :base-window
	:name                   win_paint
	:owner                  win
	:width                  230
	:height                 57
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Paint"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              t
	:icon-file              "icon/paint.xv.icon"
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  win_paint
	:help                   ""
	:x                      0
	:y                      0
	:width                  230
	:height                 57
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
	:name                   msg_mask
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      12
	:width                  94
	:height                 13
	:foreground-color       ""
	:label                  "Mask: <None>"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_brush
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      32
	:width                  65
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Brush ..."
	:label-type             :string
	:initial-state          :active
	:menu                   menu_brush
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (win_paint but_brush)
		:when                   (Notify )
		:to                     (pop_brush)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
	)
)
(
	:type                   :button
	:name                   but_edit
	:owner                  controls1
	:help                   ""
	:x                      78
	:y                      32
	:width                  56
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Edit"
	:label-type             :string
	:initial-state          :active
	:menu                   menu_edit
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_apply_log
	:owner                  controls1
	:help                   ""
	:x                      138
	:y                      32
	:width                  89
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Apply log"
	:label-type             :string
	:initial-state          :active
	:menu                   menu_apply_log
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   set_aspect
	:owner                  controls1
	:help                   ""
	:x                      20
	:y                      60
	:width                  129
	:height                 23
	:value-x                77
	:value-y                60
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Aspect:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("X axis" "Y axis" "Z axis" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:choice-defaults        (nil nil t )
	:initial-selections     (nil nil t )
	:initial-state          :invisible
	:user-data              ()
	:actions                (
		(
		:from                   (win_paint set_aspect)
		:when                   (Notify )
		:to                     (win_paint set_aspect)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_PAINT].aspect = value;
win[WIN_PAINT].f = win[value].f;
paint_setup();
realloc_window_buffers(WIN_PAINT);
realloc_window_ximage(WIN_PAINT);
redisplay_paint();
")
		)
	)
)
(
	:type                   :setting
	:name                   set_zoom_mag
	:owner                  controls1
	:help                   ""
	:x                      28
	:y                      84
	:width                  103
	:height                 23
	:value-x                76
	:value-y                84
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Zoom:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("x 1" "x 2" "x 3" "x 4" "x 5" "x 6" )
	:choice-label-types     (:string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" )
	:choice-defaults        (t nil nil nil nil nil )
	:initial-selections     (t nil nil nil nil nil )
	:initial-state          :invisible
	:user-data              ()
	:actions                (
		(
		:from                   (win_paint set_zoom_mag)
		:when                   (Notify )
		:to                     (win_paint set_zoom_mag)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_PAINT].zoom_mag = value + 1;
paint_setup();
realloc_window_ximage(WIN_PAINT);
redisplay_paint();")
		)
	)
)
(
	:type                   :slider
	:name                   set_paint_frame
	:owner                  controls1
	:help                   ""
	:x                      24
	:y                      112
	:width                  119
	:height                 94
	:value-x                72
	:value-y                112
	:slider-width           40
	:ticks                  0
	:foreground-color       ""
	:label                  "Frame:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :vertical
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              0
	:max-value              256
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :invisible
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (win_paint set_paint_frame)
		:when                   (Notify )
		:to                     (win_paint set_paint_frame)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_PAINT].f = value;
win[WIN_PAINT].repaint = TRUE;
redisplay_paint();

save_image_orig(WIN_PAINT);

if(xv_get(Threshold_pop_threshold->pop_threshold,
		XV_SHOW, NULL)
&& (threshold.roi == R2d_WHOLE
|| threshold.roi == R2d_CROP
|| threshold.roi == R2d_PT_LIST)) refresh_histogram();
")
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas
	:owner                  win_paint
	:help                   ""
	:x                      24
	:y                      133
	:width                  93
	:height                 1
	:background-color       ""
	:foreground-color       ""
	:initial-state          :invisible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       93
	:vertical-scrollbar     nil
	:scrollable-height      1
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (win_paint canvas)
		:when                   (Repaint )
		:to                     (win_paint canvas)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_PAINT].repaint = TRUE;
map_buffers();")
		)
		(
		:from                   (win_paint canvas)
		:when                   (AnyEvent )
		:to                     (win_paint canvas)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("paint_event(event);")
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_brush
	:owner                  win_paint
	:width                  405
	:height                 151
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Paintbrush"
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
	:name                   controls2
	:owner                  pop_brush
	:help                   ""
	:x                      0
	:y                      0
	:width                  405
	:height                 151
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
	:name                   set_mode
	:owner                  controls2
	:help                   ""
	:x                      20
	:y                      16
	:width                  201
	:height                 23
	:value-x                67
	:value-y                16
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Mode:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Mask" "Image" "Points" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:initial-selections     (t nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_brush set_mode "Mask")
		:when                   (Notify )
		:to                     (pop_brush set_mode "Mask")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.mode = BRUSH_MASK;
xv_set(Paint_pop_brush->set_affect_image,
	XV_SHOW, FALSE,
	NULL);
xv_set(Paint_pop_brush->set_degree,
	XV_SHOW, FALSE,
	NULL);
xv_set(Paint_pop_brush->set_affect_mask,
	XV_SHOW, TRUE,
	NULL);")
		)
		(
		:from                   (pop_brush set_mode "Image")
		:when                   (Notify )
		:to                     (pop_brush set_mode "Image")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.mode = BRUSH_IMAGE;
xv_set(Paint_pop_brush->set_affect_mask,
	XV_SHOW, FALSE,
	NULL);
xv_set(Paint_pop_brush->set_affect_image,
	XV_SHOW, TRUE,
	NULL);
xv_set(Paint_pop_brush->set_degree,
	XV_SHOW, TRUE,
	NULL);
")
		)
		(
		:from                   (pop_brush set_mode "Points")
		:when                   (Notify )
		:to                     (pop_brush set_mode "Points")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.mode = BRUSH_PTS;
xv_set(Paint_pop_brush->set_affect_image,
	XV_SHOW, FALSE,
	NULL);
xv_set(Paint_pop_brush->set_degree,
	XV_SHOW, FALSE,
	NULL);
xv_set(Paint_pop_brush->set_affect_mask,
	XV_SHOW, TRUE,
	NULL);")
		)
	)
)
(
	:type                   :setting
	:name                   set_affect_mask
	:owner                  controls2
	:help                   ""
	:x                      15
	:y                      44
	:width                  148
	:height                 23
	:value-x                67
	:value-y                44
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Affect:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Paint" "Erase" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_brush set_affect_mask "Erase")
		:when                   (Notify )
		:to                     (pop_brush set_affect_mask "Erase")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.mask_affect = MASK_ERASE;
change_cursor_proc();")
		)
		(
		:from                   (pop_brush set_affect_mask "Paint")
		:when                   (Notify )
		:to                     (pop_brush set_affect_mask "Paint")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.mask_affect = MASK_PAINT;
change_cursor_proc();")
		)
	)
)
(
	:type                   :setting
	:name                   set_affect_image
	:owner                  controls2
	:help                   ""
	:x                      15
	:y                      44
	:width                  166
	:height                 23
	:value-x                67
	:value-y                44
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Affect:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Darker" "Lighter" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :invisible
	:user-data              ()
	:actions                (
		(
		:from                   (pop_brush set_affect_image "Lighter")
		:when                   (Notify )
		:to                     (pop_brush set_affect_image "Lighter")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.image_affect = brush.degree;
")
		)
		(
		:from                   (pop_brush set_affect_image "Darker")
		:when                   (Notify )
		:to                     (pop_brush set_affect_image "Darker")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.image_affect = -1 * brush.degree;
")
		)
	)
)
(
	:type                   :setting
	:name                   set_degree
	:owner                  controls2
	:help                   ""
	:x                      186
	:y                      44
	:width                  212
	:height                 23
	:value-x                214
	:value-y                44
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "by:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("2" "5" "10" "20" "50" "100" )
	:choice-label-types     (:string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" )
	:initial-selections     (nil nil nil t nil nil )
	:initial-state          :invisible
	:user-data              ()
	:actions                (
		(
		:from                   (pop_brush set_degree "10")
		:when                   (Notify )
		:to                     (pop_brush set_degree "10")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.degree = 10;
")
		)
		(
		:from                   (pop_brush set_degree "100")
		:when                   (Notify )
		:to                     (pop_brush set_degree "100")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.degree = 100;
")
		)
		(
		:from                   (pop_brush set_degree "2")
		:when                   (Notify )
		:to                     (pop_brush set_degree "2")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.degree = 2;
")
		)
		(
		:from                   (pop_brush set_degree "20")
		:when                   (Notify )
		:to                     (pop_brush set_degree "20")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.degree = 20;
")
		)
		(
		:from                   (pop_brush set_degree "5")
		:when                   (Notify )
		:to                     (pop_brush set_degree "5")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.degree = 5;
")
		)
		(
		:from                   (pop_brush set_degree "50")
		:when                   (Notify )
		:to                     (pop_brush set_degree "50")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.degree = 50;
")
		)
	)
)
(
	:type                   :setting
	:name                   set_shape
	:owner                  controls2
	:help                   ""
	:x                      14
	:y                      70
	:width                  165
	:height                 23
	:value-x                67
	:value-y                70
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Shape:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Square" "Round" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_brush set_shape "Round")
		:when                   (Notify )
		:to                     (pop_brush set_shape "Round")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.shape = BRUSH_ROUND;
change_cursor_proc();")
		)
		(
		:from                   (pop_brush set_shape "Square")
		:when                   (Notify )
		:to                     (pop_brush set_shape "Square")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.shape = BRUSH_SQUARE;
change_cursor_proc();")
		)
	)
)
(
	:type                   :setting
	:name                   set_size
	:owner                  controls2
	:help                   ""
	:x                      28
	:y                      97
	:width                  344
	:height                 23
	:value-x                67
	:value-y                97
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Size:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("1x1" "2x2" "3x3" "4x4" "5x5" "10x10" "20x20" )
	:choice-label-types     (:string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" )
	:initial-selections     (nil t nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (pop_brush set_size)
		:when                   (Notify )
		:to                     (pop_brush set_size)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("brush.size = value;
change_cursor_proc();")
		)
	)
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls2
	:help                   ""
	:x                      348
	:y                      128
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
		:from                   (pop_brush but_close)
		:when                   (Notify )
		:to                     (pop_brush)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
