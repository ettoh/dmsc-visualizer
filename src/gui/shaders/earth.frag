#version 330
uniform sampler2D texture;
varying vec2 f_texcoord;
varying vec3 f_normal;
varying vec3 f_coord3d;
layout (std140) uniform Global{	mat4 modelview; mat4 projection; mat4 normal_transform;};

void main(void) {
    float ambient = 0.09;
    vec3 light_source = (modelview * vec4(0.0, 0.0, -4.0, 1.0)).xyz;
    vec3 N = normalize((normal_transform * vec4(f_normal, 1.0)).xyz);
    vec3 L = normalize(light_source - f_coord3d); // light source
    vec3 V = normalize(-f_coord3d); // view
    vec3 R = -reflect(L, N); // reflection vector

    float diffuse = 0.8 * max(0.0, dot(L, N));
    float e = 21.0;
    vec3 specular = 0.4 * pow(max(0.0, dot(V, R)), e) * vec3(0.89, 0.855, 0.686) ;
    vec3 color = min(1.0, diffuse + ambient) * vec3(texture2D(texture, f_texcoord)) + specular;
    gl_FragColor = vec4(color, 1.0);    
}