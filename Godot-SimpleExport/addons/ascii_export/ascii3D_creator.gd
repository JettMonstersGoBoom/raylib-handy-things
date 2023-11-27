@tool
extends Panel

var editor_interface

func _ready():
	# Connect all of the signals we'll need to save and load silly materials.
	get_node(^"SaveButton").pressed.connect(self.save_pressed)
	get_node(^"SaveAscii3DDialog").file_selected.connect(self.save_file_selected)
	RenderingServer.canvas_item_set_clip(get_canvas_item(), true)

func save_pressed():
	get_node(^"SaveAscii3DDialog").popup_centered()

func _add_node_data(node,node_list,resource_list, resource_check_dict):
	if node.get_class()=="Node3D":
		node_list.append(node);
	if node.get_class()=="Skeleton3D":
		node_list.append(node);
	if node.get_class()=="MeshInstance3D":
		if node.mesh!=null:
			node_list.append(node);
			var fpath = node.mesh.get_path()
			if fpath not in resource_list:
				resource_list.append(fpath)
#				print(some_array[0])
	if node.get_class()=="OmniLight3D":
		node_list.append(node);

	var child_nodes_for_node = node.get_children();
	for child_node in child_nodes_for_node:
		_add_node_data(child_node,node_list,resource_list, resource_check_dict);

func save_file_selected(path):
	var resource_list = [];
	var resource_check_dict = {};
	var node_list = [];
	#open file 
	var file = FileAccess.open(path, FileAccess.WRITE)

	# get root
	var scene_root = get_tree().root.get_child(get_tree().root.get_child_count()-1);
	# collect scene
	_add_node_data(scene_root,node_list,resource_list, resource_check_dict)

	# make a string of everything 	
	var content = ""
	# all resources 
	content += "resources:" + str(resource_list.size()) + "\n"
	for r in resource_list:
		var fpath = r.trim_prefix("res://")
		var some_array = fpath.split(":", true, 1)
		content += some_array[0] + "\n"

	# all nodes	
	content += "nodes:" + str(node_list.size()) + "\n"
	# ClassName , NodeName, ParentName, ResourceID ( -1 if no mesh ), 3x4 floats matrix 

	for n in node_list:
		var s = ""
		var t = n.get_transform()
		s = n.get_class() 
		s += ","
		s += n.get_name() 
		s += ","
		s += n.get_parent().get_name()
		s += ","
		if n.get_class() == "MeshInstance3D":
			s += str(resource_list.find(n.mesh.get_path()))
		else:
			s += "-1"
		s += ","
		s += str(t.basis.x.x)
		s += ","
		s += str(t.basis.x.y)
		s += ","
		s += str(t.basis.x.z)
		s += ","
		s += str(t.basis.y.x)
		s += ","
		s += str(t.basis.y.y)
		s += ","
		s += str(t.basis.y.z)
		s += ","
		s += str(t.basis.z.x)
		s += ","
		s += str(t.basis.z.y)
		s += ","
		s += str(t.basis.z.z)
		s += ","
		s += str(t.origin.x)
		s += ","
		s += str(t.origin.y)
		s += ","
		s += str(t.origin.z)

		# if we're a light we add some stuff TBD 
		if n.get_class() == "OmniLight3D":
			var col = n.get_color()
			s += ",$"
			s += "%x" % (col.r * 255)
			s += "%x" % (col.g * 255)
			s += "%x" % (col.b * 255)
			s += "%x" % (col.a * 255)
		
		content += s + "\n"

	file.store_string(content)
	file = null
	return true
