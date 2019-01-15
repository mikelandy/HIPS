;GIL-3
(
(
	:type                   :base-window
	:name                   zmwin
	:owner                  nil
	:width                  256
	:height                 256
	:background-color       ""
	:foreground-color       ""
	:label                  "Zoom Window"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              t
	:icon-file              ""
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (zmwin)
		:when                   (Resize )
		:to                     (zmwin)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (zoom_resize_proc)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   zmcanv
	:owner                  zmwin
	:help                   ""
	:x                      0
	:y                      0
	:width                  256
	:height                 256
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   t
	:scrollable-width       256
	:vertical-scrollbar     t
	:scrollable-height      256
	:repaint-proc           zmcanv_repaint_proc
	:event-handler          zmcanv_event_proc
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (zmwin zmcanv)
		:when                   (Repaint )
		:to                     (zmwin zmcanv)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (zmcanv_repaint_proc)
		)
		(
		:from                   (zmwin zmcanv)
		:when                   (AnyEvent )
		:to                     (zmwin zmcanv)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (zmcanv_event_proc)
		)
	)
)
)
