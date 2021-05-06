#version 330
layout (location = 0) in vec3 coord3d;
layout (location = 1) in vec3 offset;
layout (std140) uniform Global{	mat4 modelview; mat4 projection; mat4 normal_transform;};
varying vec3 f_color;

void main(void) {
	vec3 position = coord3d + offset;
	vec4 camera_coords = modelview * vec4(position, 1.0);
	gl_Position = projection * camera_coords;
	f_color = vec3(1.0, 1.0, 1.0);
}
