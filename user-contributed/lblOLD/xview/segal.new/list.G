;GIL-3
(
(
	:type                   :popup-window
	:name                   pop_list
	:owner                  win
	:width                  591
	:height                 319
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: List"
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
	:owner                  pop_list
	:help                   ""
	:x                      0
	:y                      0
	:width                  591
	:height                 319
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
	:name                   but_recall
	:owner                  controls1
	:help                   ""
	:x                      348
	:y                      12
	:width                  77
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Recall list"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_list but_recall)
		:when                   (Notify )
		:to                     (pop_list but_close)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("recall_list();")
		)
	)
)
(
	:type                   :button
	:name                   but_store
	:owner                  controls1
	:help                   ""
	:x                      429
	:y                      12
	:width                  72
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Store list"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_list but_store)
		:when                   (Notify )
		:to                     (pop_list but_store)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("store_list();")
		)
	)
)
(
	:type                   :button
	:name                   but_load
	:owner                  controls1
	:help                   ""
	:x                      505
	:y                      12
	:width                  69
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Load list"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_list but_load)
		:when                   (Notify )
		:to                     (pop_list but_load)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("load_list();")
		)
	)
)
(
	:type                   :text-field
	:name                   text_l_fname
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      16
	:width                  319
	:height                 15
	:value-x                95
	:value-y                16
	:value-length           30
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "List name:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_description
	:owner                  controls1
	:help                   ""
	:x                      4
	:y                      39
	:width                  571
	:height                 15
	:value-x                95
	:value-y                39
	:value-length           60
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Description:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_image
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      62
	:width                  54
	:height                 13
	:foreground-color       ""
	:label                  "Image -"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_i_dname
	:owner                  controls1
	:help                   ""
	:x                      22
	:y                      83
	:width                  554
	:height                 15
	:value-x                96
	:value-y                83
	:value-length           60
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Directory:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_i_fname
	:owner                  controls1
	:help                   ""
	:x                      22
	:y                      106
	:width                  554
	:height                 15
	:value-x                96
	:value-y                106
	:value-length           60
	:stored-length          80
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
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_masks
	:owner                  controls1
	:help                   ""
	:x                      15
	:y                      129
	:width                  56
	:height                 13
	:foreground-color       ""
	:label                  "Masks -"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_m_dname
	:owner                  controls1
	:help                   ""
	:x                      21
	:y                      150
	:width                  554
	:height                 15
	:value-x                95
	:value-y                150
	:value-length           60
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Directory:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_m1_fname
	:owner                  controls1
	:help                   ""
	:x                      36
	:y                      173
	:width                  539
	:height                 15
	:value-x                95
	:value-y                173
	:value-length           60
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Mask 1:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_m2_fname
	:owner                  controls1
	:help                   ""
	:x                      36
	:y                      196
	:width                  539
	:height                 15
	:value-x                95
	:value-y                196
	:value-length           60
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Mask 2:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_m3_fname
	:owner                  controls1
	:help                   ""
	:x                      36
	:y                      219
	:width                  539
	:height                 15
	:value-x                95
	:value-y                219
	:value-length           60
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Mask 3:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_m4_fname
	:owner                  controls1
	:help                   ""
	:x                      36
	:y                      242
	:width                  539
	:height                 15
	:value-x                95
	:value-y                242
	:value-length           60
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Mask 4:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   text_m5_fname
	:owner                  controls1
	:help                   ""
	:x                      36
	:y                      265
	:width                  539
	:height                 15
	:value-x                95
	:value-y                265
	:value-length           60
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Mask 5:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls1
	:help                   ""
	:x                      524
	:y                      292
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
		:from                   (pop_list but_close)
		:when                   (Notify )
		:to                     (pop_list)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
