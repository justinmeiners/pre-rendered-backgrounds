import bpy
import os
from subprocess import call 
from mathutils import *
from math import radians

cam = bpy.context.scene.camera

cam_rotations = (
    Euler((radians(90.0), radians(90.0), radians(-90.0)), 'XYZ'),
    Euler((radians(90.0), radians(-90.0), radians(90.0)), 'XYZ'),
    Euler((radians(-90.0), radians(0.0), radians(0.0)), 'XYZ'),
    Euler((radians(-90.0), radians(180.0), radians(180.0)), 'XYZ'),
    Euler((radians(0.0), radians(180.0), radians(180.0)), 'XYZ'),
    Euler((radians(0.0), radians(0.0), radians(180.0)), 'XYZ')
)

cam.data.angle_y = radians(90.0)
bpy.context.scene.render.resolution_x = 128
bpy.context.scene.render.resolution_y = 128

paths = []

for i, rotation in enumerate(cam_rotations):
    path = '//%s%i.png' % (cam.data.name, i)
    paths.append(bpy.path.abspath(path))
    bpy.context.scene.render.filepath = path
    cam.rotation_euler = rotation
    bpy.ops.render.render(write_still=True)
    
output_file = bpy.path.abspath('//%s.dds' % (cam.data.name,))
call(['/usr/local/bin/nvassemble', '-noalpha', '-cube', '-o', output_file] + paths) 

for path in paths:
    os.remove(path)
