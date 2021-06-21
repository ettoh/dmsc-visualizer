#version 330
uniform sampler2D earth_day;
uniform sampler2D earth_water;

varying vec2 f_texcoord;
varying vec4 f_normal;
varying vec3 f_coord3d;
layout (std140) uniform Global{	
    mat4 model; 
    mat4 view; 
    mat4 projection; 
    mat4 scaling;
    mat4 sun_angle;
};

void main(void) {
    float ambient = 0.09;
    vec3 light_source = (model * sun_angle * vec4(0.0, 0.0, 20.0, 1.0)).xyz;
    vec3 N = normalize(f_normal).xyz;
    vec3 L = light_source - f_coord3d; // light source
    float sqr_distance = dot(L, L);
    L = normalize(L);
    vec3 V = -normalize(f_coord3d - inverse(view)[3].xyz);
    vec3 R = -reflect(L, N);

    float lambertian = max(dot(N, L), 0.0);
    float specular = 0.0;
    float spec_angle = max(dot(V, R), 0.0);
    specular = pow(spec_angle, 20);

    vec3 earth_color = vec3(texture2D(earth_day, f_texcoord + vec2(0.5, 0.0)));
    vec3 water_map = vec3(texture2D(earth_water, f_texcoord + vec2(0.5, 0.0))); // black where water
    float is_water = (water_map.r + water_map.g + water_map.b) / 3.0; // 0 -> land
    if(is_water <= 0.1){
        specular *= min(is_water + 0.15, 1);
    }
    vec3 diffuse_color = earth_color;
    vec3 light_color = vec3(0.89, 0.855, 0.686) * 500.0;

    vec3 output_color = vec3(0.0);
    output_color += diffuse_color * vec3(0.07); // ambient
    output_color += diffuse_color * lambertian * light_color / sqr_distance; // diffuse
    output_color += 0.2 * vec3(1.0) * specular * light_color / sqr_distance; // specular

    gl_FragColor = vec4(output_color, 1.0);    
}
