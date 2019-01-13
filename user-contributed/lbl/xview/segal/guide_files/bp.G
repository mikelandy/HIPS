;GIL-2
(
(
	:type                   :menu
	:name                   menu_init
	:help                   ""
	:columns                1
	:label                  ""
	:label-type             :string
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Image Frame" "Edit Mask Frame" "Other Buffer" )
	:menu-item-label-types  (:string :string :string )
	:menu-item-defaults     (nil nil nil )
	:menu-item-handlers     (nil nil nil )
	:menu-item-menus        (nil nil nil )
	:menu-item-colors       ("" "" "" )
	:pinnable               t
	:user-data              ()
)
(
	:type                   :menu
	:name                   menu_segment
	:help                   ""
	:columns                1
	:label                  ""
	:label-type             :string
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("2D Analysis" "3D Analysis" )
	:menu-item-label-types  (:string :string )
	:menu-item-defaults     (nil nil )
	:menu-item-handlers     (nil nil )
	:menu-item-menus        (menu_2D menu_3D )
	:menu-item-colors       ("" "" )
	:pinnable               t
	:user-data              ()
)
(
	:type                   :menu
	:name                   menu_2D
	:help                   ""
	:columns                1
	:label                  ""
	:label-type             :string
	:menu-type              :command
	:menu-handler           nil
	:menu-title             "Menu 2D"
	:menu-item-labels       ("Area of Interest" "Surface Fitting" "Symbolic Representation" "Convexity Analysis" )
	:menu-item-label-types  (:string :string :string :string )
	:menu-item-defaults     (nil nil nil nil )
	:menu-item-handlers     (test_proc_2d menu_map_surf_fit test_proc_2d test_proc_2d )
	:menu-item-menus        (nil nil nil nil )
	:menu-item-colors       ("" "" "" "" )
	:pinnable               t
	:user-data              ()
)
(
	:type                   :menu
	:name                   menu_3D
	:help                   ""
	:columns                1
	:label                  ""
	:label-type             :string
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Volume of Interest" "Volume Fitting" "Volumetric Description" )
	:menu-item-label-types  (:string :string :string )
	:menu-item-defaults     (nil nil nil )
	:menu-item-handlers     (nil nil nil )
	:menu-item-menus        (nil nil nil )
	:menu-item-colors       ("" "" "" )
	:pinnable               nil
	:user-data              ()
)
(
	:type                   :menu
	:name                   menu_render
	:help                   ""
	:columns                1
	:label                  ""
	:label-type             :string
	:menu-type              :command
	:menu-handler           nil
	:menu-title             ""
	:menu-item-labels       ("Shading" "Map Intensity to Volume" )
	:menu-item-label-types  (:string :string )
	:menu-item-defaults     (nil nil )
	:menu-item-handlers     (nil nil )
	:menu-item-menus        (nil nil )
	:menu-item-colors       ("" "" )
	:pinnable               t
	:user-data              ()
)
(
	:type                   :base-window
	:name                   win
	:owner                  nil
	:width                  354
	:height                 67
	:background-color       ""
	:foreground-color       ""
	:label                  "Segal: Braham Parvin's Tools"
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
	:width                  354
	:height                 67
	:background-color       ""
	:foreground-color       ""
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_init
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      8
	:constant-width         nil
	:button-type            :normal
	:width                  110
	:height                 19
	:foreground-color       ""
	:label                  "Initialization"
	:label-type             :string
	:menu                   menu_init
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_segment
	:owner                  controls1
	:help                   ""
	:x                      128
	:y                      8
	:constant-width         nil
	:button-type            :normal
	:width                  116
	:height                 19
	:foreground-color       ""
	:label                  "Segmentation"
	:label-type             :string
	:menu                   menu_segment
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_render
	:owner                  controls1
	:help                   ""
	:x                      256
	:y                      8
	:constant-width         nil
	:button-type            :normal
	:width                  94
	:height                 19
	:foreground-color       ""
	:label                  "Rendering"
	:label-type             :string
	:menu                   menu_render
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
(
	:type                   :button
	:name                   but_close
	:owner                  controls1
	:help                   ""
	:x                      160
	:y                      44
	:constant-width         nil
	:button-type            :normal
	:width                  51
	:height                 19
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:menu                   nil
	:notify-handler         map_bp
	:event-handler          nil
	:events                 ()
	:user-data              ()
)
)
