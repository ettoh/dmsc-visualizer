#version 330
layout (location = 0) in vec3 coord3d;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;
layout (std140) uniform Global{	mat4 modelview; mat4 projection; mat4 normal_transform;};
varying vec2 f_texcoord;
varying vec3 f_normal;
varying vec3 f_coord3d;

void main(void) {
	vec4 camera_coords = modelview * vec4(coord3d, 1.0);
	gl_Position = projection * camera_coords;

	f_texcoord = texcoord;
	f_normal = normal;
	f_coord3d = camera_coords.xyz;
}
