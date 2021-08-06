#version 420
layout (location = 0) in vec3 coord3d;
layout (location = 1) in vec3 v_color;
layout (std140) uniform Global{	
    mat4 model; 
    mat4 view; 
    mat4 projection; 
    mat4 scaling;
    mat4 sun_angle;
};

varying vec3 f_color;

void main(void) {
	vec4 camera_coords = view * model * scaling * vec4(coord3d, 1.0);
	gl_Position = projection * camera_coords;
	f_color = v_color;
}
