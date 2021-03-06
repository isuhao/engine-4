add_subdirectory(voxedit-util)

project(voxedit)
set(SRCS
	ui/editorscene/EditorScene.h ui/editorscene/EditorScene.cpp
	ui/editorscene/Model.h ui/editorscene/Model.cpp
	ui/editorscene/Controller.h ui/editorscene/Controller.cpp
	ui/editorscene/Action.h
	ui/editorscene/voxelizer.h

	ui/palette/PaletteWidget.h ui/palette/PaletteWidget.cpp

	ui/lsystem/RuleItem.h
	ui/lsystem/RuleItemSource.cpp
	ui/lsystem/RuleItemSource.h
	ui/lsystem/RuleItemWidget.cpp
	ui/lsystem/RuleItemWidget.h
	ui/lsystem/SyntaxHighlighter.h

	ui/VoxEditWindow.h ui/VoxEditWindow.cpp
	ui/LSystemWindow.h ui/LSystemWindow.cpp
	ui/TreeWindow.h ui/TreeWindow.cpp
	ui/NoiseWindow.h ui/NoiseWindow.cpp

	VoxEdit.h VoxEdit.cpp
)

set(FILES
	voxedit/ui/skin/voxedit-axis-x.png
	voxedit/ui/skin/voxedit-axis-y.png
	voxedit/ui/skin/voxedit-axis-z.png
	voxedit/ui/skin/voxedit-color-picker.png
	voxedit/ui/skin/voxedit-copy.png
	voxedit/ui/skin/voxedit-crop.png
	voxedit/ui/skin/voxedit-cursor-circle.png
	voxedit/ui/skin/voxedit-cursor-cone.png
	voxedit/ui/skin/voxedit-cursor-dome.png
	voxedit/ui/skin/voxedit-cursor-plane.png
	voxedit/ui/skin/voxedit-cursor-single.png
	voxedit/ui/skin/voxedit-delete.png
	voxedit/ui/skin/voxedit-export.png
	voxedit/ui/skin/voxedit-extend.png
	voxedit/ui/skin/voxedit-fill.png
	voxedit/ui/skin/voxedit-flip-horizontal.png
	voxedit/ui/skin/voxedit-flip-vertical.png
	voxedit/ui/skin/voxedit-load.png
	voxedit/ui/skin/voxedit-lsystem.png
	voxedit/ui/skin/voxedit-new.png
	voxedit/ui/skin/voxedit-options.png
	voxedit/ui/skin/voxedit-override.png
	voxedit/ui/skin/voxedit-quadview.png
	voxedit/ui/skin/voxedit-quit.png
	voxedit/ui/skin/voxedit-redo.png
	voxedit/ui/skin/voxedit-reset-camera.png
	voxedit/ui/skin/voxedit-rotate-left.png
	voxedit/ui/skin/voxedit-rotate-right.png
	voxedit/ui/skin/voxedit-save.png
	voxedit/ui/skin/voxedit-selection.png
	voxedit/ui/skin/voxedit-show-grid.png
	voxedit/ui/skin/voxedit-show-grid-selected.png
	voxedit/ui/skin/voxedit-skin.tb.txt
	voxedit/ui/skin/voxedit-tree_pine.png
	voxedit/ui/skin/voxedit-tree.png
	voxedit/ui/skin/voxedit-undo.png
	voxedit/ui/skin/voxedit-voxel.png
	voxedit/ui/widget/voxedit-lsystem-item.tb.txt
	voxedit/ui/window/voxedit-lsystem.tb.txt
	voxedit/ui/window/voxedit-main.tb.txt
	voxedit/ui/window/voxedit-noise.tb.txt
	voxedit/ui/window/voxedit-tree.tb.txt
	voxedit/ui/window/voxedit-world.tb.txt
)

engine_add_executable(TARGET ${PROJECT_NAME} SRCS ${SRCS} FILES ${FILES} WINDOWED)
engine_target_link_libraries(TARGET ${PROJECT_NAME} DEPENDENCIES turbobadger voxedit-util nativefiledialog)
include_directories(.)
check_ui_turbobadger(${PROJECT_NAME})
