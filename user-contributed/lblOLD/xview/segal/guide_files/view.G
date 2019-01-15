;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  800
	:height                 800
	:background-color       ""
	:foreground-color       ""
	:label                  "Image "
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
	:type                   :canvas-pane
	:name                   canvas
	:owner                  win
	:help                   ""
	:x                      0
	:y                      0
	:width                  800
	:height                 800
	:background-color       ""
	:foreground-color       ""
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       800
	:vertical-scrollbar     nil
	:scrollable-height      800
	:repaint-proc           image_repaint_proc
	:event-handler          disp_event_proc
	:events                 (:mouse :mouse-drag )
	:drawing-model          :xwindows
	:user-data              ()
)
)
