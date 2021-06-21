#version 330
layout (location = 0) in vec3 coord3d;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;
layout (std140) uniform Global{	
    mat4 model; 
    mat4 view; 
    mat4 projection; 
    mat4 scaling;
    mat4 sun_angle;
};

varying vec2 f_texcoord;
varying vec4 f_normal;
varying vec3 f_coord3d;

void main(void) {
	vec4 camera_coords = model * vec4(coord3d, 1.0);
	gl_Position = projection * view * scaling * camera_coords;

	f_texcoord = texcoord;
	f_normal = model * vec4(normal, 0);
	f_coord3d = camera_coords.xyz;
}
