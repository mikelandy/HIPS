;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  385
	:height                 168
	:background-color       ""
	:foreground-color       ""
	:label                  "File I/O"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              nil
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
	:owner                  window1
	:help                   ""
	:x                      0
	:y                      0
	:width                  385
	:height                 168
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
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      16
	:width                  35
	:height                 13
	:foreground-color       ""
	:label                  "Load:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   lmode
	:owner                  controls1
	:help                   ""
	:x                      28
	:y                      31
	:width                  173
	:height                 23
	:value-x                76
	:value-y                31
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "mode:"
	:label-type             :string
	:notify-handler         lmode_proc
	:event-handler          nil
	:choices                ("GENIAL Image" "GENIAL log file" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:choice-defaults        (nil nil )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (window1 lmode)
		:when                   (Notify )
		:to                     (window1 lmode)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (lmode_proc)
		)
	)
)
(
	:type                   :button
	:name                   load
	:owner                  controls1
	:help                   ""
	:x                      329
	:y                      33
	:width                  44
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "load"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         load_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 load)
		:when                   (Notify )
		:to                     (window1 load)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (load_proc)
		)
	)
)
(
	:type                   :button
	:name                   chooser_load
	:owner                  controls1
	:help                   ""
	:x                      280
	:y                      57
	:width                  102
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "File Chooser..."
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         chooser_load_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 chooser_load)
		:when                   (Notify )
		:to                     (window1 chooser_load)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (chooser_load_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   l_fname
	:owner                  controls1
	:help                   ""
	:x                      28
	:y                      59
	:width                  234
	:height                 15
	:value-x                102
	:value-y                59
	:value-length           20
	:stored-length          255
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Filename:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         lname_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 l_fname)
		:when                   (Notify )
		:to                     (window1 l_fname)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (lname_proc)
		)
	)
)
(
	:type                   :message
	:name                   message2
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      84
	:width                  35
	:height                 13
	:foreground-color       ""
	:label                  "Save:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   smode
	:owner                  controls1
	:help                   ""
	:x                      32
	:y                      97
	:width                  266
	:height                 23
	:value-x                80
	:value-y                97
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "mode:"
	:label-type             :string
	:notify-handler         smode_proc
	:event-handler          nil
	:choices                ("GENIAL Image w/ log" "GENIAL Image w/o log" "Rectangular GENIAL subimage" "GENIAL log file" "Sun Rasterfile Image Dump" "ASCII trace vector" "ASCII histogram vector" )
	:choice-label-types     (:string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (window1 smode)
		:when                   (Notify )
		:to                     (window1 smode)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (smode_proc)
		)
	)
)
(
	:type                   :button
	:name                   save
	:owner                  controls1
	:help                   ""
	:x                      328
	:y                      99
	:width                  45
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "save"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         save_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 save)
		:when                   (Notify )
		:to                     (window1 save)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (save_proc)
		)
	)
)
(
	:type                   :button
	:name                   chooser_save
	:owner                  controls1
	:help                   ""
	:x                      280
	:y                      125
	:width                  102
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "File Chooser..."
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         chooser_save_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 chooser_save)
		:when                   (Notify )
		:to                     (window1 chooser_save)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (chooser_save_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   s_fname
	:owner                  controls1
	:help                   ""
	:x                      32
	:y                      127
	:width                  234
	:height                 15
	:value-x                106
	:value-y                127
	:value-length           20
	:stored-length          255
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Filename:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         sname_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 s_fname)
		:when                   (Notify )
		:to                     (window1 s_fname)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (sname_proc)
		)
	)
)
(
	:type                   :popup-window
	:name                   rect_choice_win
	:owner                  window1
	:width                  456
	:height                 133
	:background-color       ""
	:foreground-color       ""
	:label                  "Rectangular Subimage Save"
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
	:name                   controls2
	:owner                  rect_choice_win
	:help                   ""
	:x                      0
	:y                      0
	:width                  456
	:height                 133
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
	:name                   message3
	:owner                  controls2
	:help                   ""
	:x                      20
	:y                      28
	:width                  268
	:height                 13
	:foreground-color       ""
	:label                  "Please enter the log ID # of a rectangle:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   box_lid
	:owner                  controls2
	:help                   ""
	:x                      44
	:y                      52
	:width                  125
	:height                 15
	:value-x                98
	:value-y                52
	:value-length           2
	:stored-length          8
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Log ID:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         boxid_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (rect_choice_win box_lid)
		:when                   (Notify )
		:to                     (rect_choice_win box_lid)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (boxid_proc)
		)
	)
)
(
	:type                   :button
	:name                   box_save
	:owner                  controls2
	:help                   ""
	:x                      120
	:y                      88
	:width                  45
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "save"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         box_save_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (rect_choice_win box_save)
		:when                   (Notify )
		:to                     (rect_choice_win box_save)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (box_save_proc)
		)
	)
)
(
	:type                   :button
	:name                   box_cancel
	:owner                  controls2
	:help                   ""
	:x                      216
	:y                      88
	:width                  56
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "cancel"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         box_cancel_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (rect_choice_win box_cancel)
		:when                   (Notify )
		:to                     (rect_choice_win box_cancel)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (box_cancel_proc)
		)
	)
)
)
