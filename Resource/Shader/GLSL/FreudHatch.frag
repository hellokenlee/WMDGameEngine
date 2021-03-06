#version 420 core
#define LIGHT_NUM 1

struct Light {
	float type;
	float range;
	float _pad0, _pad1;
	vec4 color;
	vec4 position;
	vec4 direction;
	float attenuation;
};


layout (binding = 0) uniform sampler2D tex0;

layout (std140, binding = 0) uniform UBO0 {
	mat4 view;
	mat4 proj;
	vec3 camera_position;
	float curr_time, sin_time, cos_time;
	float _pad0;
	float texcoord_scale;
};

layout (std140, binding = 2) uniform UBO2 {
	Light lights[LIGHT_NUM];
};


in vec2 texcoord_VS_out;
in vec3 normal_VS_out;
in vec3 position_VS_out;

out vec4 color_FS_out;

void main() {
	//
	vec2 texcoord = texcoord_VS_out * texcoord_scale;
	//
	vec3 normal = normalize(normal_VS_out);
	vec3 light_dir = normalize(lights[0].position.xyz - position_VS_out);
	float diffuse = max(dot(normal, light_dir), 0.0) * lights[0].color.r;
	//
	vec3 view_dir = normalize(camera_position - position_VS_out);
	vec3 halfway_dir = normalize(light_dir + view_dir);
	float specular = pow(max(dot(normal, halfway_dir), 0.0), 2.0) * lights[0].color.r;
	//
	float intensity = clamp(diffuse, 0.0, 1.0);
	//
	float threshold = texture(tex0, texcoord).r;
	//
	float final_color = 1.0 - 4.0 * (1.0 - (intensity + threshold));
	//
	color_FS_out = vec4(final_color, final_color, final_color, 1.0);
}