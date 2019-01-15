;GIL-3
(
(
	:type                   :popup-window
	:name                   pop_load_image
	:owner                  win
	:width                  377
	:height                 266
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Load Image"
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
	:name                   controls1
	:owner                  pop_load_image
	:help                   ""
	:x                      0
	:y                      0
	:width                  377
	:height                 266
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
	:type                   :text-field
	:name                   text_image_dname
	:owner                  controls1
	:help                   ""
	:x                      51
	:y                      20
	:width                  314
	:height                 15
	:value-x                125
	:value-y                20
	:value-length           30
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
	:name                   text_image_fname
	:owner                  controls1
	:help                   ""
	:x                      30
	:y                      45
	:width                  335
	:height                 15
	:value-x                125
	:value-y                45
	:value-length           30
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Image name:"
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
	:name                   msg_format
	:owner                  controls1
	:help                   ""
	:x                      22
	:y                      76
	:width                  95
	:height                 13
	:foreground-color       ""
	:label                  "Image format:"
	:label-type             :string
	:label-bold             t
	:initial-state          :inactive
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   msg_bytes_required
	:owner                  controls1
	:help                   ""
	:x                      14
	:y                      99
	:width                  103
	:height                 13
	:foreground-color       ""
	:label                  "Bytes required:"
	:label-type             :string
	:label-bold             t
	:initial-state          :inactive
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   row_from
	:owner                  controls1
	:help                   ""
	:x                      33
	:y                      124
	:width                  163
	:height                 15
	:value-x                125
	:value-y                124
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Row-> from:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_image row_from)
		:when                   (Notify )
		:to                     (pop_load_image row_from)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.r1 = value;
if(segal.r2 < value) {
	segal.r2 = value;
	xv_set(File_pop_load_image->row_to,
		PANEL_VALUE, value,
		NULL);
}
update_bytes_required();")
		)
	)
)
(
	:type                   :text-field
	:name                   row_to
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      124
	:width                  95
	:height                 15
	:value-x                256
	:value-y                124
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "to:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_image row_to)
		:when                   (Notify )
		:to                     (pop_load_image row_to)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.r2 = value;
if(segal.r1 > value) {
	segal.r1 = value;
	xv_set(File_pop_load_image->row_from,
		PANEL_VALUE, value,
		NULL);
}
update_bytes_required();")
		)
	)
)
(
	:type                   :text-field
	:name                   col_from
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      147
	:width                  188
	:height                 15
	:value-x                125
	:value-y                147
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Column-> from:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_image col_from)
		:when                   (Notify )
		:to                     (pop_load_image col_from)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.c1 = value;
if(segal.c2 < value) {
	segal.c2 = value;
	xv_set(File_pop_load_image->col_to,
		PANEL_VALUE, value,
		NULL);
}
update_bytes_required();")
		)
	)
)
(
	:type                   :text-field
	:name                   col_to
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      149
	:width                  95
	:height                 15
	:value-x                256
	:value-y                149
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "to:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_image col_to)
		:when                   (Notify )
		:to                     (pop_load_image col_to)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.c2 = value;
if(segal.c1 > value) {
	segal.c1 = value;
	xv_set(File_pop_load_image->col_from,
		PANEL_VALUE, value,
		NULL);
}
update_bytes_required();")
		)
	)
)
(
	:type                   :text-field
	:name                   frm_from
	:owner                  controls1
	:help                   ""
	:x                      19
	:y                      172
	:width                  177
	:height                 15
	:value-x                125
	:value-y                172
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Frame-> from:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_image frm_from)
		:when                   (Notify )
		:to                     (pop_load_image frm_from)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.f1 = value;
if(segal.f2 < value) {
	segal.f2 = value;
	xv_set(File_pop_load_image->frm_to,
		PANEL_VALUE, value,
		NULL);
}
update_bytes_required();")
		)
	)
)
(
	:type                   :text-field
	:name                   frm_to
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      174
	:width                  95
	:height                 15
	:value-x                256
	:value-y                174
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "to:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              100
	:min-value              0
	:initial-value          0
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_image frm_to)
		:when                   (Notify )
		:to                     (pop_load_image frm_to)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.f2 = value;
if(segal.f1 > value) {
	segal.f1 = value;
	xv_set(File_pop_load_image->frm_from,
		PANEL_VALUE, value,
		NULL);
}
update_bytes_required();")
		)
	)
)
(
	:type                   :setting
	:name                   set_color_format
	:owner                  controls1
	:help                   ""
	:x                      28
	:y                      197
	:width                  219
	:height                 23
	:value-x                125
	:value-y                197
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Color format:"
	:label-type             :string
	:notify-handler         nil
	:event-handler          nil
	:choices                ("Color" "Grayscale" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :inactive
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_image set_color_format "Color")
		:when                   (Notify )
		:to                     (pop_load_image set_color_format "Color")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.color = TRUE;
update_bytes_required();")
		)
		(
		:from                   (pop_load_image set_color_format "Grayscale")
		:when                   (Notify )
		:to                     (pop_load_image set_color_format "Grayscale")
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("segal.color = FALSE;
update_bytes_required();
img.hd.color_form = CFM_SGF;")
		)
	)
)
(
	:type                   :button
	:name                   but_load_image
	:owner                  controls1
	:help                   ""
	:x                      144
	:y                      240
	:width                  87
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Load image"
	:label-type             :string
	:initial-state          :inactive
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_image but_load_image)
		:when                   (Notify )
		:to                     (pop_load_image but_load_image)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("load_image();
")
		)
	)
)
(
	:type                   :button
	:name                   but_close_image
	:owner                  controls1
	:help                   ""
	:x                      317
	:y                      240
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
		:from                   (pop_load_image but_close_image)
		:when                   (Notify )
		:to                     (pop_load_image)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_load_mask
	:owner                  win
	:width                  352
	:height                 98
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Load Mask"
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
	:name                   controls3
	:owner                  pop_load_mask
	:help                   ""
	:x                      0
	:y                      0
	:width                  352
	:height                 98
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
	:type                   :text-field
	:name                   text_mask_dname
	:owner                  controls3
	:help                   ""
	:x                      27
	:y                      16
	:width                  314
	:height                 15
	:value-x                101
	:value-y                16
	:value-length           30
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
	:name                   text_mask_fname
	:owner                  controls3
	:help                   ""
	:x                      12
	:y                      41
	:width                  329
	:height                 15
	:value-x                101
	:value-y                41
	:value-length           30
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Mask name:"
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
	:name                   but_load_mask
	:owner                  controls3
	:help                   ""
	:x                      144
	:y                      72
	:width                  82
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Load mask"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         load_mask
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_load_mask but_load_mask)
		:when                   (Notify )
		:to                     (pop_load_mask but_load_mask)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (load_mask)
		)
	)
)
(
	:type                   :button
	:name                   but_close_mask
	:owner                  controls3
	:help                   ""
	:x                      290
	:y                      72
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
		:from                   (pop_load_mask but_close_mask)
		:when                   (Notify )
		:to                     (pop_load_mask)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_new_mask
	:owner                  win
	:width                  384
	:height                 98
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: New Mask"
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
	:owner                  pop_new_mask
	:help                   ""
	:x                      0
	:y                      0
	:width                  384
	:height                 98
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
	:type                   :text-field
	:name                   text_new_dname
	:owner                  controls4
	:help                   ""
	:x                      60
	:y                      16
	:width                  314
	:height                 15
	:value-x                134
	:value-y                16
	:value-length           30
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
	:name                   text_new_fname
	:owner                  controls4
	:help                   ""
	:x                      11
	:y                      41
	:width                  363
	:height                 15
	:value-x                134
	:value-y                41
	:value-length           30
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "New mask name:"
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
	:name                   but_create_new
	:owner                  controls4
	:help                   ""
	:x                      152
	:y                      72
	:width                  94
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Create mask"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_new_mask but_create_new)
		:when                   (Notify )
		:to                     (pop_new_mask but_create_new)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("new_mask_proc();")
		)
	)
)
(
	:type                   :button
	:name                   but_close_new
	:owner                  controls4
	:help                   ""
	:x                      323
	:y                      72
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
		:from                   (pop_new_mask but_close_new)
		:when                   (Notify )
		:to                     (pop_new_mask)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_save_as
	:owner                  win
	:width                  384
	:height                 98
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Save Mask As"
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
	:name                   controls5
	:owner                  pop_save_as
	:help                   ""
	:x                      0
	:y                      0
	:width                  384
	:height                 98
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
	:type                   :text-field
	:name                   text_save_dname
	:owner                  controls5
	:help                   ""
	:x                      50
	:y                      16
	:width                  314
	:height                 15
	:value-x                124
	:value-y                16
	:value-length           30
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
	:name                   text_save_fname
	:owner                  controls5
	:help                   ""
	:x                      20
	:y                      41
	:width                  344
	:height                 15
	:value-x                124
	:value-y                41
	:value-length           30
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Save mask as:"
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
	:name                   but_save_mask_as
	:owner                  controls5
	:help                   ""
	:x                      151
	:y                      72
	:width                  82
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Save mask"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_save_as but_save_mask_as)
		:when                   (Notify )
		:to                     (pop_save_as but_save_mask_as)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("save_mask_as();")
		)
	)
)
(
	:type                   :button
	:name                   but_close_save
	:owner                  controls5
	:help                   ""
	:x                      313
	:y                      72
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
		:from                   (pop_save_as but_close_save)
		:when                   (AnyEvent )
		:to                     (pop_save_as)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
(
	:type                   :popup-window
	:name                   pop_save_image
	:owner                  win
	:width                  384
	:height                 98
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Save Image As"
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
	:owner                  pop_save_image
	:help                   ""
	:x                      0
	:y                      0
	:width                  384
	:height                 98
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
	:type                   :text-field
	:name                   text_i_save_dname
	:owner                  controls2
	:help                   ""
	:x                      50
	:y                      16
	:width                  314
	:height                 15
	:value-x                124
	:value-y                16
	:value-length           30
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
	:name                   text_i_save_fname
	:owner                  controls2
	:help                   ""
	:x                      15
	:y                      41
	:width                  349
	:height                 15
	:value-x                124
	:value-y                41
	:value-length           30
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Save image as:"
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
	:name                   but_i_save_as
	:owner                  controls2
	:help                   ""
	:x                      149
	:y                      72
	:width                  87
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Save image"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (pop_save_image but_i_save_as)
		:when                   (Notify )
		:to                     (pop_save_image but_i_save_as)
		:function_type          ExecuteCode
		:arg_type               (:string)
		:action                 ("save_image_as();")
		)
	)
)
(
	:type                   :button
	:name                   but_i_close_save_as
	:owner                  controls2
	:help                   ""
	:x                      312
	:y                      72
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
		:from                   (pop_save_image but_i_close_save_as)
		:when                   (Notify )
		:to                     (pop_save_image)
		:function_type          :user_defined
		:arg_type               ()
		:action                 (Hide)
		)
	)
)
)
