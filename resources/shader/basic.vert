#version 420
layout (location = 0) in vec3 coord3d;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 normal;

layout (std140) uniform Global{	
    mat4 camera_rotation; 
    mat4 view; 
    mat4 projection; 
    mat4 scaling;
    mat4 sun_angle;
};

varying vec3 f_color;
varying vec4 f_normal;
varying vec3 f_coord3d;
varying vec2 f_texcoord;

void main(void) {
    vec4 camera_coords = camera_rotation * vec4(coord3d, 1);
	gl_Position = projection * view * scaling * camera_coords;

	f_color = v_color;
    f_texcoord = texcoord;
    f_normal = camera_rotation * vec4(normal, 0);
	f_coord3d = camera_coords.xyz;
}
