;GIL-3
(
(
	:type                   :popup-window
	:name                   tcntrl
	:owner                  nil
	:width                  264
	:height                 131
	:background-color       ""
	:foreground-color       ""
	:label                  "Trace Control"
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
	:name                   controls1
	:owner                  tcntrl
	:help                   ""
	:x                      0
	:y                      0
	:width                  264
	:height                 131
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            t
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   traceno
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      20
	:width                  120
	:height                 15
	:value-x                65
	:value-y                20
	:value-length           2
	:stored-length          4
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Trace:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              1
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         traceno_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (tcntrl traceno)
		:when                   (Notify )
		:to                     (tcntrl traceno)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (traceno_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   radius
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      45
	:width                  151
	:height                 15
	:value-x                96
	:value-y                45
	:value-length           1
	:stored-length          1
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Averaging:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              10
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         rad_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (tcntrl radius)
		:when                   (Notify )
		:to                     (tcntrl radius)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (rad_proc)
		)
	)
)
(
	:type                   :setting
	:name                   scale
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      70
	:width                  177
	:height                 23
	:value-x                77
	:value-y                70
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Scaling:"
	:label-type             :string
	:notify-handler         scale_proc
	:event-handler          nil
	:choices                ("Image Scaled" "Trace Scaled" "Manual" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:choice-defaults        (nil nil nil )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (tcntrl scale)
		:when                   (Notify )
		:to                     (tcntrl scale)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (scale_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      103
	:width                  108
	:height                 15
	:value-x                53
	:value-y                103
	:value-length           4
	:stored-length          4
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              255
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         min_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (tcntrl min)
		:when                   (Notify )
		:to                     (tcntrl min)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (min_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max
	:owner                  controls1
	:help                   ""
	:x                      144
	:y                      103
	:width                  109
	:height                 15
	:value-x                182
	:value-y                103
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
	:notify-handler         max_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (tcntrl max)
		:when                   (Notify )
		:to                     (tcntrl max)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (max_proc)
		)
	)
)
(
	:type                   :popup-window
	:name                   trwin
	:owner                  nil
	:width                  268
	:height                 384
	:background-color       ""
	:foreground-color       ""
	:label                  "GENIAL trace"
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
	:owner                  trwin
	:help                   ""
	:x                      0
	:y                      0
	:width                  268
	:height                 67
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
	:name                   mes1
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      8
	:width                  86
	:height                 13
	:foreground-color       ""
	:label                  "Value: 11111"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   tr_refresh
	:owner                  controls2
	:help                   ""
	:x                      160
	:y                      8
	:width                  64
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Refresh"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         trace_refresh_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (trwin tr_refresh)
		:when                   (Notify )
		:to                     (trwin tr_refresh)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (trace_refresh_proc)
		)
	)
)
(
	:type                   :message
	:name                   mes3
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      24
	:width                  85
	:height                 13
	:foreground-color       ""
	:label                  "Sum: 111111"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   options
	:owner                  controls2
	:help                   ""
	:x                      160
	:y                      32
	:width                  75
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Options..."
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         trace_opt_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (trwin options)
		:when                   (Notify )
		:to                     (trwin options)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (trace_opt_proc)
		)
	)
)
(
	:type                   :message
	:name                   mes4
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      40
	:width                  102
	:height                 13
	:foreground-color       ""
	:label                  "Average: 11111"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   trcanv
	:owner                  trwin
	:help                   ""
	:x                      0
	:y                      67
	:width                  268
	:height                 275
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       268
	:vertical-scrollbar     nil
	:scrollable-height      275
	:repaint-proc           trcanv_repaint_proc
	:event-handler          trcanv_event_proc
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (trwin trcanv)
		:when                   (Repaint )
		:to                     (trwin trcanv)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (trcanv_repaint_proc)
		)
		(
		:from                   (trwin trcanv)
		:when                   (AnyEvent )
		:to                     (trwin trcanv)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (trcanv_event_proc)
		)
	)
)
(
	:type                   :control-area
	:name                   controls3
	:owner                  trwin
	:help                   ""
	:x                      0
	:y                      342
	:width                  268
	:height                 42
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
	:name                   integrate
	:owner                  controls3
	:help                   ""
	:x                      8
	:y                      8
	:width                  214
	:height                 23
	:value-x                188
	:value-y                8
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Select Integration region:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   intg_eval
	:owner                  controls3
	:help                   ""
	:x                      216
	:y                      8
	:width                  43
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "eval"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         integrate_eval_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (trwin intg_eval)
		:when                   (Notify )
		:to                     (trwin intg_eval)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (integrate_eval_proc)
		)
	)
)
(
	:type                   :popup-window
	:name                   intgwin
	:owner                  nil
	:width                  212
	:height                 295
	:background-color       ""
	:foreground-color       ""
	:label                  "Integrate"
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
	:name                   controls4
	:owner                  intgwin
	:help                   ""
	:x                      0
	:y                      0
	:width                  212
	:height                 32
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
	:name                   message1
	:owner                  controls4
	:help                   ""
	:x                      8
	:y                      8
	:width                  180
	:height                 13
	:foreground-color       ""
	:label                  "Integral Total: 9999999999"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   intgcanv
	:owner                  intgwin
	:help                   ""
	:x                      0
	:y                      32
	:width                  212
	:height                 265
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       212
	:vertical-scrollbar     nil
	:scrollable-height      265
	:repaint-proc           icanv_repaint_proc
	:event-handler          nil
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (intgwin intgcanv)
		:when                   (Repaint )
		:to                     (intgwin intgcanv)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (icanv_repaint_proc)
		)
	)
)
)
