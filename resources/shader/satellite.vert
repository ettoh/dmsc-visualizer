#version 330
layout (location = 0) in vec3 coord3d;
layout (location = 1) in vec3 offset;
layout (std140) uniform Global{	
    mat4 model; 
    mat4 view; 
    mat4 projection; 
    mat4 scaling;
    mat4 sun_angle;
};

varying vec3 f_color;

void main(void) {
	vec3 position = coord3d + offset;
	vec4 camera_coords = view * model * scaling * vec4(position, 1.0);
	gl_Position = projection * camera_coords;
	f_color = vec3(1.0, 1.0, 1.0);
}
