#version 330
layout (location = 0) in vec3 coord3d;
layout (location = 1) in vec3 v_color;
layout (std140) uniform Global{	mat4 modelview; mat4 projection; mat4 normal_transform;};
varying vec3 f_color;

void main(void) {
	vec4 camera_coords = modelview * vec4(coord3d, 1.0);
	gl_Position = projection * camera_coords;
	f_color = v_color;
}
