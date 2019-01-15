;GIL-3
(
(
	:type                   :popup-window
	:name                   histo_ctrl
	:owner                  nil
	:width                  311
	:height                 390
	:background-color       ""
	:foreground-color       ""
	:label                  "Histogram Color Control"
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
	:owner                  histo_ctrl
	:help                   ""
	:x                      0
	:y                      0
	:width                  311
	:height                 390
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
	:name                   histno
	:owner                  controls1
	:help                   ""
	:x                      4
	:y                      12
	:width                  153
	:height                 15
	:value-x                86
	:value-y                12
	:value-length           2
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Histogram:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         histno_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl histno)
		:when                   (Notify )
		:to                     (histo_ctrl histno)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histno_proc)
		)
	)
)
(
	:type                   :button
	:name                   clear_all
	:owner                  controls1
	:help                   ""
	:x                      236
	:y                      12
	:width                  70
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Clear All"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         clear_histo_color_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl clear_all)
		:when                   (Notify )
		:to                     (histo_ctrl clear_all)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (clear_histo_color_proc)
		)
	)
)
(
	:type                   :message
	:name                   message3
	:owner                  controls1
	:help                   ""
	:x                      4
	:y                      40
	:width                  35
	:height                 13
	:foreground-color       ""
	:label                  "Paint"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   message4
	:owner                  controls1
	:help                   ""
	:x                      100
	:y                      40
	:width                  41
	:height                 13
	:foreground-color       ""
	:label                  "Select"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   min1
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      60
	:width                  136
	:height                 15
	:value-x                193
	:value-y                60
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval1_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl min1)
		:when                   (Notify )
		:to                     (histo_ctrl min1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval1_proc)
		)
	)
)
(
	:type                   :setting
	:name                   pnt1
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      64
	:width                  34
	:height                 23
	:value-x                12
	:value-y                64
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_paint_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl pnt1)
		:when                   (Notify )
		:to                     (histo_ctrl pnt1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_paint_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sel1
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      64
	:width                  34
	:height                 23
	:value-x                104
	:value-y                64
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_select_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl sel1)
		:when                   (Notify )
		:to                     (histo_ctrl sel1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_select_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max1
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      76
	:width                  137
	:height                 15
	:value-x                194
	:value-y                76
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval1_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl max1)
		:when                   (Notify )
		:to                     (histo_ctrl max1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval1_proc)
		)
	)
)
(
	:type                   :setting
	:name                   pnt2
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      104
	:width                  34
	:height                 23
	:value-x                12
	:value-y                104
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_paint_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl pnt2)
		:when                   (Notify )
		:to                     (histo_ctrl pnt2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_paint_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sel2
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      104
	:width                  34
	:height                 23
	:value-x                104
	:value-y                104
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_select_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl sel2)
		:when                   (Notify )
		:to                     (histo_ctrl sel2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_select_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min2
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      104
	:width                  136
	:height                 15
	:value-x                193
	:value-y                104
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval2_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl min2)
		:when                   (Notify )
		:to                     (histo_ctrl min2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval2_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max2
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      120
	:width                  137
	:height                 15
	:value-x                194
	:value-y                120
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval2_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl max2)
		:when                   (Notify )
		:to                     (histo_ctrl max2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval2_proc)
		)
	)
)
(
	:type                   :setting
	:name                   pnt3
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      144
	:width                  34
	:height                 23
	:value-x                12
	:value-y                144
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_paint_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl pnt3)
		:when                   (Notify )
		:to                     (histo_ctrl pnt3)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_paint_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sel3
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      144
	:width                  34
	:height                 23
	:value-x                104
	:value-y                144
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_select_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl sel3)
		:when                   (Notify )
		:to                     (histo_ctrl sel3)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_select_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min3
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      144
	:width                  136
	:height                 15
	:value-x                193
	:value-y                144
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval3_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl min3)
		:when                   (Notify )
		:to                     (histo_ctrl min3)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval3_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max3
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      160
	:width                  137
	:height                 15
	:value-x                194
	:value-y                160
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval3_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl max3)
		:when                   (Notify )
		:to                     (histo_ctrl max3)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval3_proc)
		)
	)
)
(
	:type                   :setting
	:name                   pnt4
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      184
	:width                  34
	:height                 23
	:value-x                12
	:value-y                184
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_paint_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl pnt4)
		:when                   (Notify )
		:to                     (histo_ctrl pnt4)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_paint_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sel4
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      184
	:width                  34
	:height                 23
	:value-x                104
	:value-y                184
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_select_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl sel4)
		:when                   (Notify )
		:to                     (histo_ctrl sel4)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_select_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min4
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      184
	:width                  136
	:height                 15
	:value-x                193
	:value-y                184
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval4_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl min4)
		:when                   (Notify )
		:to                     (histo_ctrl min4)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval4_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max4
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      200
	:width                  137
	:height                 15
	:value-x                194
	:value-y                200
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval4_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl max4)
		:when                   (Notify )
		:to                     (histo_ctrl max4)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval4_proc)
		)
	)
)
(
	:type                   :setting
	:name                   pnt5
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      224
	:width                  34
	:height                 23
	:value-x                12
	:value-y                224
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_paint_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl pnt5)
		:when                   (Notify )
		:to                     (histo_ctrl pnt5)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_paint_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sel5
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      224
	:width                  34
	:height                 23
	:value-x                104
	:value-y                224
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_select_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl sel5)
		:when                   (Notify )
		:to                     (histo_ctrl sel5)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_select_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min5
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      224
	:width                  136
	:height                 15
	:value-x                193
	:value-y                224
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval5_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl min5)
		:when                   (Notify )
		:to                     (histo_ctrl min5)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval5_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max5
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      240
	:width                  137
	:height                 15
	:value-x                194
	:value-y                240
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval5_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl max5)
		:when                   (Notify )
		:to                     (histo_ctrl max5)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval5_proc)
		)
	)
)
(
	:type                   :setting
	:name                   pnt6
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      264
	:width                  34
	:height                 23
	:value-x                12
	:value-y                264
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_paint_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl pnt6)
		:when                   (Notify )
		:to                     (histo_ctrl pnt6)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_paint_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sel6
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      264
	:width                  34
	:height                 23
	:value-x                104
	:value-y                264
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_select_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl sel6)
		:when                   (Notify )
		:to                     (histo_ctrl sel6)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_select_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min6
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      264
	:width                  136
	:height                 15
	:value-x                193
	:value-y                264
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval6_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl min6)
		:when                   (Notify )
		:to                     (histo_ctrl min6)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval6_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max6
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      280
	:width                  137
	:height                 15
	:value-x                194
	:value-y                280
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval6_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl max6)
		:when                   (Notify )
		:to                     (histo_ctrl max6)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval6_proc)
		)
	)
)
(
	:type                   :setting
	:name                   pnt7
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      304
	:width                  34
	:height                 23
	:value-x                12
	:value-y                304
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_paint_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl pnt7)
		:when                   (Notify )
		:to                     (histo_ctrl pnt7)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_paint_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sel7
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      304
	:width                  34
	:height                 23
	:value-x                104
	:value-y                304
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_select_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl sel7)
		:when                   (Notify )
		:to                     (histo_ctrl sel7)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_select_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min7
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      304
	:width                  136
	:height                 15
	:value-x                193
	:value-y                304
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval7_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl min7)
		:when                   (Notify )
		:to                     (histo_ctrl min7)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval7_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max7
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      320
	:width                  137
	:height                 15
	:value-x                194
	:value-y                320
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval7_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl max7)
		:when                   (Notify )
		:to                     (histo_ctrl max7)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval7_proc)
		)
	)
)
(
	:type                   :setting
	:name                   pnt8
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      344
	:width                  34
	:height                 23
	:value-x                12
	:value-y                344
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_paint_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl pnt8)
		:when                   (Notify )
		:to                     (histo_ctrl pnt8)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_paint_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sel8
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      344
	:width                  34
	:height                 23
	:value-x                104
	:value-y                344
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         histo_color_select_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl sel8)
		:when                   (Notify )
		:to                     (histo_ctrl sel8)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_select_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min8
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      344
	:width                  136
	:height                 15
	:value-x                193
	:value-y                344
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval8_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl min8)
		:when                   (Notify )
		:to                     (histo_ctrl min8)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval8_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max8
	:owner                  controls1
	:help                   ""
	:x                      156
	:y                      360
	:width                  137
	:height                 15
	:value-x                194
	:value-y                360
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         interval8_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl max8)
		:when                   (Notify )
		:to                     (histo_ctrl max8)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (interval8_proc)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   palcanv
	:owner                  histo_ctrl
	:help                   ""
	:x                      44
	:y                      58
	:width                  43
	:height                 323
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       43
	:vertical-scrollbar     nil
	:scrollable-height      323
	:repaint-proc           palcanv_repaint_proc
	:event-handler          nil
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (histo_ctrl palcanv)
		:when                   (Repaint )
		:to                     (histo_ctrl palcanv)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (palcanv_repaint_proc)
		)
	)
)
(
	:type                   :base-window
	:name                   display
	:owner                  nil
	:width                  565
	:height                 419
	:background-color       ""
	:foreground-color       ""
	:label                  "Histogram Display"
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
	:name                   controls2
	:owner                  display
	:help                   ""
	:x                      0
	:y                      0
	:width                  565
	:height                 72
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
	:type                   :message
	:name                   message1
	:owner                  controls2
	:help                   ""
	:x                      32
	:y                      8
	:width                  113
	:height                 13
	:foreground-color       ""
	:label                  "Pixel Value: NNN"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   color_histo
	:owner                  controls2
	:help                   ""
	:x                      284
	:y                      8
	:width                  59
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Color..."
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         histo_color_opts
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (display color_histo)
		:when                   (Notify )
		:to                     (display color_histo)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_color_opts)
		)
	)
)
(
	:type                   :button
	:name                   refresh
	:owner                  controls2
	:help                   ""
	:x                      488
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
	:notify-handler         histo_refresh_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (display refresh)
		:when                   (Notify )
		:to                     (display refresh)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_refresh_proc)
		)
	)
)
(
	:type                   :message
	:name                   message2
	:owner                  controls2
	:help                   ""
	:x                      32
	:y                      28
	:width                  114
	:height                 13
	:foreground-color       ""
	:label                  "Pixel Count: NNN"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   magnify
	:owner                  controls2
	:help                   ""
	:x                      280
	:y                      40
	:width                  67
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Magnify"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         mag_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (display magnify)
		:when                   (Notify )
		:to                     (display magnify)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (mag_proc)
		)
	)
)
(
	:type                   :button
	:name                   unmagnify
	:owner                  controls2
	:help                   ""
	:x                      360
	:y                      40
	:width                  85
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Unmagnify"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         unmag_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (display unmagnify)
		:when                   (Notify )
		:to                     (display unmagnify)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (unmag_proc)
		)
	)
)
(
	:type                   :button
	:name                   restore
	:owner                  controls2
	:help                   ""
	:x                      488
	:y                      40
	:width                  63
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Restore"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         restore_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (display restore)
		:when                   (Notify )
		:to                     (display restore)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (restore_proc)
		)
	)
)
(
	:type                   :message
	:name                   message5
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      52
	:width                  138
	:height                 13
	:foreground-color       ""
	:label                  "Total Intensity: NNN"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   histocanv
	:owner                  display
	:help                   ""
	:x                      0
	:y                      72
	:width                  565
	:height                 325
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       565
	:vertical-scrollbar     nil
	:scrollable-height      325
	:repaint-proc           histo_repaint_proc
	:event-handler          histo_event_proc
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (display histocanv)
		:when                   (Repaint )
		:to                     (display histocanv)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_repaint_proc)
		)
		(
		:from                   (display histocanv)
		:when                   (AnyEvent )
		:to                     (display histocanv)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (histo_event_proc)
		)
	)
)
)
