import bpy
import json
import mathutils

view_list = []
light_list = []
actor_list = []

def calc_aabb(obj):
    corners = [obj.matrix_world * mathutils.Vector(corner) for corner in obj.bound_box]
    
    x_coords = [v.x for v in corners]
    y_coords = [v.y for v in corners]
    z_coords = [v.z for v in corners]

    min_vec = (min(x_coords), min(y_coords), min(z_coords))
    max_vec = (max(x_coords), max(y_coords), max(z_coords))
    
    return (min_vec, max_vec)

for obj in bpy.data.objects:
    if obj.type == 'CAMERA':
        cam = obj.data
        view_list.append({
            'name' : cam.name,
            'near' : cam.clip_start,
            'far' : cam.clip_end,
            'fov' : cam.angle_y,
            'position' : [x for x in obj.location],
            'rotation' : [x for x in obj.rotation_quaternion],
            'light_primary' : cam['light_primary'],
            'light_secondary' : cam['light_secondary']
        })
    if obj.type == 'LAMP':
        if obj.get('ignore', False):
            continue 
        
        lamp = obj.data
        if not lamp.type == 'POINT':
            continue
        
        light_list.append({
            'name' : lamp.name,
            'color' : [x for x in lamp.color],
            'position' : [x for x in obj.location]    
        })
    if 'actor' in obj:
        (bounds_min, bounds_max) = calc_aabb(obj)
        
        actor_dict = {
            'name' : obj.name,
            'type' : obj['actor'],
            'position' : [x for x in obj.location],
            'rotation' : [x for x in obj.rotation_quaternion],
            'bounds_min' : bounds_min,
            'bounds_max' : bounds_max,
        }
        
        for key in obj.keys():
            if key not in '_RNA_UI' and 'cycles' not in key:
                actor_dict[key] = obj[key]
        
        actor_list.append(actor_dict)
        
scene_dict = {
    'name' : bpy.context.scene.name,
    'views' : view_list,
    'lights' : light_list,
    'actors' : actor_list
}

with open('/users/justin/desktop/manifest.json', 'w') as file:
    file.write(json.dumps(scene_dict, sort_keys=True, indent=4))






