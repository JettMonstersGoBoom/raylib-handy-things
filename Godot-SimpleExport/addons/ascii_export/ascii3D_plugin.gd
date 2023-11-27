# A simple (and silly) material resource plugin. Allows you to make a really simple material
# from a custom dock, that you can save and load, and apply to selected MeshInstances.
#
# SPECIAL NOTE: This technically should be using EditorImportPlugin and EditorExportPlugin
# to handle the input and output of the silly material. However, currently you cannot export
# custom resources in Godot, so instead we're using JSON files instead.
#
# This example should be replaced when EditorImportPlugin and EditorExportPlugin are both
# fully working and you can save custom resources.

@tool
extends EditorPlugin

var io_export_dialog

func _enter_tree():
	io_export_dialog = preload("res://addons/ascii_export/ascii_dock.tscn").instantiate()
	io_export_dialog.editor_interface = get_editor_interface()
	add_control_to_container(EditorPlugin.CONTAINER_SPATIAL_EDITOR_MENU,io_export_dialog)

func _exit_tree():
	remove_control_from_docks(io_export_dialog)
