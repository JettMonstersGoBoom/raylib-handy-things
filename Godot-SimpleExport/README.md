# Godot-SimpleExport
Export a scene from Godot into an easier to parse ascii format

Copy into your Godot project. ( keep addons/ascii_export filepath ) 

THIS ISN'T A COMPLETE SOLUTION. I'm hoping it's hackable enough to get what you need quickly and modify it to fit your wants.



file contains one line per "thing" 

first Resources. which are simply pathnames for 3d models 

```
resources:<int>
filename.obj
house.gltf
.....
```

then 

```
nodes:<int>
ClassName , NodeName , ParentName , Resource ID , 12 floats for a 3x4 matrix

eg. 
MeshInstance3D,rock_B,rock_B2,37,1,0,0,0,1,0,0,0,1,0,0,0
```

NOTES:

ResourceID of -1 indicates there's no 3D model for this Node. 

ALL matrices are in local space. 
When rendering you need to reference the parent node worldMatrix and multiply the local and parent world together. 

example.c shows one quick way to load the file and render it









