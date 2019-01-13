;GIL-2
(
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  283
	:height                 132
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Surface Fitting"
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
	:width                  283
	:height                 132
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   set_alpha
	:owner                  controls1
	:help                   ""
	:x                      32
	:y                      16
	:width                  149
	:height                 20
	:value-x                83
	:value-y                16
	:slider-width           10
	:foreground-color       ""
	:label                  "Alpha:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              0
	:max-value              10
	:ticks                  0
	:notify-handler         set_surf_fit_alpha_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :text-field
	:name                   set_k
	:owner                  controls1
	:help                   ""
	:x                      62
	:y                      36
	:width                  60
	:height                 15
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "K:"
	:label-type             :string
	:value-x                82
	:value-y                36
	:layout-type            :horizontal
	:value-length           5
	:stored-length          32
	:read-only              nil
	:notify-handler         set_surf_fit_k_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :slider
	:name                   set_iterations
	:owner                  controls1
	:help                   ""
	:x                      4
	:y                      56
	:width                  274
	:height                 20
	:value-x                82
	:value-y                56
	:slider-width           100
	:foreground-color       ""
	:label                  "Iterations:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             t
	:min-value              0
	:max-value              100
	:ticks                  0
	:notify-handler         set_surf_fit_iterations_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_do_fitting
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      80
	:constant-width         nil
	:button-type            :normal
	:width                  78
	:height                 19
	:foreground-color       ""
	:label                  "Do Fitting"
	:label-type             :string
	:menu                   nil
	:notify-handler         surf_fit_proc
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls1
	:help                   ""
	:x                      116
	:y                      104
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_surf_fit
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
