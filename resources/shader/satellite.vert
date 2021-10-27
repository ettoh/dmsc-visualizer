#version 420
layout (location = 0) in vec3 coord3d;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 normal;
layout (location = 4) in mat4 transformation;
layout (location = 8) in vec3 dyn_color;

layout (std140) uniform Global{	
    mat4 camera_rotation; 
    mat4 view; 
    mat4 projection; 
    mat4 scaling;
    mat4 sun_angle;
};

out vec4 f_color;
out vec4 f_normal;
out vec3 f_coord3d;
out vec2 f_texcoord;

void main(void) {
    vec4 camera_coords = transformation * vec4(coord3d, 1.0);
	gl_Position = projection * view * camera_rotation * scaling * camera_coords;

    if(dyn_color.r >= 0){
        f_color = vec4(dyn_color, 1);
    }else{
        f_color = v_color;
    }
	
    f_normal = camera_rotation * transformation * vec4(normal, 0);
	f_coord3d = camera_coords.xyz;
}
