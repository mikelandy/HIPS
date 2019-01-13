;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  500
	:height                 535
	:background-color       ""
	:foreground-color       ""
	:label                  "Display Histogram of Image"
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
	:name                   controls1
	:owner                  win
	:help                   ""
	:x                      0
	:y                      0
	:width                  500
	:height                 120
	:background-color       ""
	:foreground-color       ""
	:show-border            t
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   file_item
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      7
	:width                  267
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Image File : "
	:label-type             :string
	:value-x                99
	:value-y                7
	:layout-type            :horizontal
	:value-length           22
	:stored-length          80
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   button4
	:owner                  controls1
	:help                   ""
	:x                      298
	:y                      7
	:constant-width         nil
	:button-type            :normal
	:width                  46
	:height                 20
	:foreground-color       ""
	:label                  "Load"
	:label-type             :string
	:menu                   nil
	:notify-handler         load_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   quit_item
	:owner                  controls1
	:help                   ""
	:x                      367
	:y                      7
	:constant-width         nil
	:button-type            :normal
	:width                  49
	:height                 20
	:foreground-color       ""
	:label                  "QUIT"
	:label-type             :string
	:menu                   nil
	:notify-handler         quit_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   button6
	:owner                  controls1
	:help                   ""
	:x                      437
	:y                      7
	:constant-width         nil
	:button-type            :normal
	:width                  46
	:height                 20
	:foreground-color       ""
	:label                  "Help"
	:label-type             :string
	:menu                   nil
	:notify-handler         help_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   button5
	:owner                  controls1
	:help                   ""
	:x                      270
	:y                      35
	:constant-width         nil
	:button-type            :normal
	:width                  126
	:height                 20
	:foreground-color       ""
	:label                  "Check pixel value"
	:label-type             :string
	:menu                   nil
	:notify-handler         check_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   pixel_value
	:owner                  controls1
	:help                   ""
	:x                      9
	:y                      37
	:width                  240
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "pixel value : "
	:label-type             :string
	:value-x                105
	:value-y                37
	:layout-type            :horizontal
	:value-length           18
	:stored-length          80
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   button1
	:owner                  controls1
	:help                   ""
	:x                      120
	:y                      64
	:constant-width         nil
	:button-type            :normal
	:width                  42
	:height                 20
	:foreground-color       ""
	:label                  "<--"
	:label-type             :string
	:menu                   nil
	:notify-handler         move_left_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   button2
	:owner                  controls1
	:help                   ""
	:x                      192
	:y                      64
	:constant-width         nil
	:button-type            :normal
	:width                  42
	:height                 20
	:foreground-color       ""
	:label                  "-->"
	:label-type             :string
	:menu                   nil
	:notify-handler         move_right_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   button3
	:owner                  controls1
	:help                   ""
	:x                      271
	:y                      64
	:constant-width         nil
	:button-type            :normal
	:width                  65
	:height                 20
	:foreground-color       ""
	:label                  "Select  |"
	:label-type             :string
	:menu                   nil
	:notify-handler         select_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   eval_item
	:owner                  controls1
	:help                   ""
	:x                      364
	:y                      64
	:constant-width         nil
	:button-type            :normal
	:width                  43
	:height                 20
	:foreground-color       ""
	:label                  "Eval"
	:label-type             :string
	:menu                   nil
	:notify-handler         eval_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   restore_item
	:owner                  controls1
	:help                   ""
	:x                      429
	:y                      64
	:constant-width         nil
	:button-type            :normal
	:width                  63
	:height                 20
	:foreground-color       ""
	:label                  "Restore"
	:label-type             :string
	:menu                   nil
	:notify-handler         restore_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   button7
	:owner                  controls1
	:help                   ""
	:x                      270
	:y                      95
	:constant-width         nil
	:button-type            :normal
	:width                  122
	:height                 20
	:foreground-color       ""
	:label                  "Set count margin"
	:label-type             :string
	:menu                   nil
	:notify-handler         set_count_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   count
	:owner                  controls1
	:help                   ""
	:x                      14
	:y                      97
	:width                  231
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "count margin : "
	:label-type             :string
	:value-x                125
	:value-y                97
	:layout-type            :horizontal
	:value-length           15
	:stored-length          80
	:read-only              nil
	:notify-handler         nil
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
	:y                      120
	:width                  500
	:height                 415
	:background-color       ""
	:foreground-color       ""
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       500
	:vertical-scrollbar     nil
	:scrollable-height      415
	:repaint-proc           histogram_repaint_proc
	:event-handler          canvas_event_proc
	:events                 (:mouse :mouse-drag )
	:drawing-model          :xwindows
	:user-data              ()
)
)
