;GIL-3
(
(
	:type                   :menu
	:name                   menu_image
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Load ..." "Save as ..." "List ..." "Quit" )
	:menu-item-label-types  (:string :string :string :string )
	:menu-item-states       (:active :active :active :active )
	:menu-item-defaults     (nil nil nil nil )
	:initial-selections     (nil nil nil nil )
	:menu-item-handlers     (nil nil nil nil )
	:menu-item-menus        (nil nil nil nil )
	:menu-item-colors       ("" "" "" "" )
	:pinnable               nil
	:user-data              ()
	:actions                (
		(
		:from                   (menu_image "Quit")
		:when                   (Notify )
		:to                     (win)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("/* FALSE -> save only if image changed */
save_image(FALSE);
unload_all_masks();

if(!begin_timer()) enq_bg_job(JOB_QUIT, 0);
else {
	xv_set(View_win->win,
		FRAME_NO_CONFIRM, TRUE,
		NULL);

	xv_destroy_safe(View_win->win);
	exit(0);
}")
		)
	)
)
(
	:type                   :menu
	:name                   menu_segment
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Threshold ..." "Mask growing ..." "Filter ..." )
	:menu-item-label-types  (:string :string :string )
	:menu-item-states       (:active :active :active )
	:menu-item-defaults     (nil nil nil )
	:initial-selections     (nil nil nil )
	:menu-item-handlers     (nil nil nil )
	:menu-item-menus        (nil nil nil )
	:menu-item-colors       ("" "" "" )
	:pinnable               nil
	:user-data              ()
	:actions                (
		(
		:from                   (menu_segment "Threshold ...")
		:when                   (Notify )
		:to                     (menu_segment "Threshold ...")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("refresh_histogram();
draw_thresh_bounds();")
		)
	)
)
(
	:type                   :menu
	:name                   menu_preferences
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Display ..." )
	:menu-item-label-types  (:string )
	:menu-item-states       (:active )
	:menu-item-defaults     (nil )
	:initial-selections     (nil )
	:menu-item-handlers     (nil )
	:menu-item-menus        (nil )
	:menu-item-colors       ("" )
	:pinnable               nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :menu
	:name                   menu_masks
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Mask log ..." "Load ..." "New ..." "Save as ..." )
	:menu-item-label-types  (:string :string :string :string )
	:menu-item-states       (:active :active :active :active )
	:menu-item-defaults     (nil nil nil nil )
	:initial-selections     (nil nil nil nil )
	:menu-item-handlers     (nil nil nil nil )
	:menu-item-menus        (nil nil nil nil )
	:menu-item-colors       ("" "" "" "" )
	:pinnable               nil
	:user-data              ()
	:actions                (
	)
)
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  340
	:height                 120
	:background-color       ""
	:foreground-color       ""
	:label                  "Bryan Skene's Segal (v. 3d)"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              "icon/segal.xv.icon"
	:icon-label             "Segal: View"
	:icon-mask-file         ""
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  win
	:help                   ""
	:x                      0
	:y                      0
	:width                  340
	:height                 120
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
	:name                   set_zoom_x
	:owner                  controls1
	:help                   ""
	:x                      484
	:y                      0
	:width                  120
	:height                 23
	:value-x                549
	:value-y                0
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "X-Zoom:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("x 1" "x 2" "x 3" "x 4" "x 5" "x 6" )
	:choice-label-types     (:string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil )
	:initial-state          :invisible
	:user-data              ()
	:actions                (
		(
		:from                   (win set_zoom_x)
		:when                   (Notify )
		:to                     (win set_zoom_x)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VX].zoom_mag = value + 1;
view_setup();
realloc_window_ximage(WIN_VX);
redisplay_view();")
		)
	)
)
(
	:type                   :message
	:name                   msg_image
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      8
	:width                  100
	:height                 13
	:foreground-color       ""
	:label                  "Image: <None>"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_mask
	:owner                  controls1
	:help                   ""
	:x                      14
	:y                      24
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
	:type                   :slider
	:name                   set_frame_x
	:owner                  controls1
	:help                   ""
	:x                      484
	:y                      24
	:width                  119
	:height                 310
	:value-x                532
	:value-y                24
	:slider-width           256
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
	:max-value              255
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
		:from                   (win set_frame_x)
		:when                   (Notify )
		:to                     (win set_frame_x)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VX].f = value;
win[WIN_VX].repaint = TRUE;
map_buffers();")
		)
	)
)
(
	:type                   :setting
	:name                   set_mode
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      48
	:width                  134
	:height                 40
	:value-x                8
	:value-y                65
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Mode:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Segment" "Register" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :inactive
	:user-data              ()
	:actions                (
		(
		:from                   (win set_mode "Register")
		:when                   (Notify )
		:to                     (win set_mode "Register")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.mode = MODE_REGISTER;")
		)
		(
		:from                   (win set_mode "Segment")
		:when                   (Notify )
		:to                     (win set_mode "Segment")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.mode = MODE_SEGMENT;
if(img.loaded) xv_set(Paint_win_paint->win_paint,
	XV_SHOW, TRUE,
	NULL);")
		)
		(
		:from                   (win set_mode "Register")
		:when                   (Notify )
		:to                     (win but_masks)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (win set_mode "Segment")
		:when                   (Notify )
		:to                     (win but_masks)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
		(
		:from                   (win set_mode "Segment")
		:when                   (Notify )
		:to                     (win but_segment)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
		(
		:from                   (win set_mode "Register")
		:when                   (Notify )
		:to                     (win but_segment)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (win set_mode "Segment")
		:when                   (Notify )
		:to                     (win but_preferences)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
		(
		:from                   (win set_mode "Register")
		:when                   (Notify )
		:to                     (win but_preferences)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (win set_mode "Segment")
		:when                   (Notify )
		:to                     (win msg_not_implemented)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
		(
		:from                   (win set_mode "Register")
		:when                   (Notify )
		:to                     (win msg_not_implemented)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Show)
		)
	)
)
(
	:type                   :setting
	:name                   set_display
	:owner                  controls1
	:help                   ""
	:x                      160
	:y                      48
	:width                  171
	:height                 40
	:value-x                160
	:value-y                65
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  "Display:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Image" "Mask" "X & Y" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:initial-selections     (t t t )
	:initial-state          :inactive
	:user-data              ()
	:actions                (
		(
		:from                   (win set_display "Image")
		:when                   (Notify )
		:to                     (win set_display "Image")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.disp_image = TRUE;
")
		)
		(
		:from                   (win set_display "Mask")
		:when                   (Notify )
		:to                     (win set_display "Image")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.disp_mask = TRUE;
")
		)
		(
		:from                   (win set_display "X & Y")
		:when                   (Notify )
		:to                     (win set_display "X & Y")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.disp_xy = TRUE;
")
		)
	)
)
(
	:type                   :button
	:name                   but_image
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      96
	:width                  69
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Image"
	:label-type             :string
	:initial-state          :active
	:menu                   menu_image
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_masks
	:owner                  controls1
	:help                   ""
	:x                      79
	:y                      96
	:width                  63
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Mask"
	:label-type             :string
	:initial-state          :inactive
	:menu                   menu_masks
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_segment
	:owner                  controls1
	:help                   ""
	:x                      144
	:y                      96
	:width                  85
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Segment"
	:label-type             :string
	:initial-state          :inactive
	:menu                   menu_segment
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_preferences
	:owner                  controls1
	:help                   ""
	:x                      231
	:y                      96
	:width                  103
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Preferences"
	:label-type             :string
	:initial-state          :inactive
	:menu                   menu_preferences
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_not_implemented
	:owner                  controls1
	:help                   ""
	:x                      144
	:y                      118
	:width                  118
	:height                 13
	:foreground-color       ""
	:label                  "Not Implemented"
	:label-type             :string
	:label-bold             t
	:initial-state          :invisible
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_pixel_value
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      124
	:width                  78
	:height                 13
	:foreground-color       ""
	:label                  "Pixel value:"
	:label-type             :string
	:label-bold             t
	:initial-state          :invisible
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   set_zoom_z
	:owner                  controls1
	:help                   ""
	:x                      173
	:y                      152
	:width                  119
	:height                 23
	:value-x                237
	:value-y                152
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Z-Zoom:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("x 1" "x 2" "x 3" "x 4" "x 5" "x 6" )
	:choice-label-types     (:string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil )
	:initial-state          :invisible
	:user-data              ()
	:actions                (
		(
		:from                   (win set_zoom_z)
		:when                   (Notify )
		:to                     (win set_zoom_z)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VZ].zoom_mag = value + 1;
view_setup();
realloc_window_ximage(WIN_VZ);
redisplay_view();")
		)
	)
)
(
	:type                   :slider
	:name                   set_frame_z
	:owner                  controls1
	:help                   ""
	:x                      172
	:y                      176
	:width                  119
	:height                 310
	:value-x                220
	:value-y                176
	:slider-width           256
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
	:max-value              255
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
		:from                   (win set_frame_z)
		:when                   (Notify )
		:to                     (win set_frame_z)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VZ].f = value;
win[WIN_VZ].repaint = TRUE;
map_buffers();")
		)
	)
)
(
	:type                   :setting
	:name                   set_zoom_y
	:owner                  controls1
	:help                   ""
	:x                      324
	:y                      320
	:width                  120
	:height                 23
	:value-x                389
	:value-y                320
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Y-Zoom:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("x 1" "x 2" "x 3" "x 4" "x 5" "x 6" )
	:choice-label-types     (:string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil )
	:initial-state          :invisible
	:user-data              ()
	:actions                (
		(
		:from                   (win set_zoom_y)
		:when                   (Notify )
		:to                     (win set_zoom_y)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VY].zoom_mag = value + 1;
view_setup();
realloc_window_ximage(WIN_VY);
redisplay_view();")
		)
	)
)
(
	:type                   :slider
	:name                   set_frame_y
	:owner                  controls1
	:help                   ""
	:x                      484
	:y                      320
	:width                  119
	:height                 310
	:value-x                532
	:value-y                320
	:slider-width           256
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
		:from                   (win set_frame_y)
		:when                   (Notify )
		:to                     (win set_frame_y)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VY].f = value;
win[WIN_VY].repaint = TRUE;
map_buffers();")
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canv_x
	:owner                  win
	:help                   ""
	:x                      389
	:y                      55
	:width                  1
	:height                 65
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       1
	:vertical-scrollbar     nil
	:scrollable-height      65
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (win canv_x)
		:when                   (Repaint )
		:to                     (win canv_x)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VX].repaint = TRUE;
map_buffers();")
		)
		(
		:from                   (win canv_x)
		:when                   (AnyEvent )
		:to                     (win canv_x)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("view_event(WIN_VX, event);")
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canv_z
	:owner                  win
	:help                   ""
	:x                      20
	:y                      212
	:width                  256
	:height                 1
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       256
	:vertical-scrollbar     nil
	:scrollable-height      1
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (win canv_z)
		:when                   (Repaint )
		:to                     (win canv_z)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VZ].repaint = TRUE;
map_buffers();")
		)
		(
		:from                   (win canv_z)
		:when                   (AnyEvent )
		:to                     (win canv_z)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("view_event(WIN_VZ, event);")
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canv_y
	:owner                  win
	:help                   ""
	:x                      366
	:y                      356
	:width                  1
	:height                 1
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       1
	:vertical-scrollbar     nil
	:scrollable-height      1
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (win canv_y)
		:when                   (Repaint )
		:to                     (win canv_y)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("win[WIN_VY].repaint = TRUE;
map_buffers();")
		)
		(
		:from                   (win canv_y)
		:when                   (AnyEvent )
		:to                     (win canv_y)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("view_event(WIN_VY, event);")
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_timer
	:owner                  win
	:width                  400
	:height                 96
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Timer"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_timer)
		:when                   (Resize )
		:to                     (pop_timer)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("timer_resize_proc();")
		)
	)
)
(
	:type                   :control-area
	:name                   controls2
	:owner                  pop_timer
	:help                   ""
	:x                      0
	:y                      0
	:width                  400
	:height                 96
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
	:name                   msg_working
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      8
	:width                  69
	:height                 13
	:foreground-color       ""
	:label                  "Working ..."
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_timer
	:owner                  controls2
	:help                   ""
	:x                      12
	:y                      72
	:width                  60
	:height                 13
	:foreground-color       ""
	:label                  "Message"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   canv_timer
	:owner                  pop_timer
	:help                   ""
	:x                      0
	:y                      23
	:width                  400
	:height                 40
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       400
	:vertical-scrollbar     nil
	:scrollable-height      40
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xwindows
	:user-data              ()
	:actions                ()
)
)
