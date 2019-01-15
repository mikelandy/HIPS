;GIL-3
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  356
	:height                 327
	:background-color       ""
	:foreground-color       ""
	:label                  "HIPS Image Display"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              ""
	:icon-label             ""
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
	:width                  360
	:height                 45
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
	:type                   :button
	:name                   button2
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      8
	:width                  61
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Perfect"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         make_perfect_colors
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (win button2)
		:when                   (Notify )
		:to                     (win button2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (make_perfect_colors)
		)
	)
)
(
	:type                   :slider
	:name                   slider1
	:owner                  controls1
	:help                   ""
	:x                      88
	:y                      8
	:width                  182
	:height                 20
	:value-x                154
	:value-y                8
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Contrast:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             nil
	:min-value              2
	:max-value              52
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (win slider1)
		:when                   (Notify )
		:to                     (win button1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gamma_proc)
		)
	)
)
(
	:type                   :button
	:name                   button1
	:owner                  controls1
	:help                   ""
	:x                      280
	:y                      8
	:width                  45
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Quit"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         quit_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (win button1)
		:when                   (Notify )
		:to                     (win button1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (quit_proc)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   can
	:owner                  win
	:help                   ""
	:x                      0
	:y                      42
	:width                  356
	:height                 285
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   t
	:scrollable-width       356
	:vertical-scrollbar     t
	:scrollable-height      285
	:repaint-proc           canvas_repaint_proc
	:event-handler          nil
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (win can)
		:when                   (Repaint )
		:to                     (win can)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (canvas_repaint_proc)
		)
	)
)
)
