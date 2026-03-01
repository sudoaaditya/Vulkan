#version 450 core
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec2 out_TexCoord;
layout(binding = 1) uniform sampler2D uTextureSampler;
layout(location = 0) out vec4 FragColor;
void main(void)
{
	//Code
	FragColor = texture(uTextureSampler, out_TexCoord);

	vec4 fragCoord = gl_FragCoord;

	for(int i = 0; i < 2; i++)
	{
		FragColor += texture(uTextureSampler, vec2((fragCoord.x + i)/512, (fragCoord.y)/512));
		FragColor += texture(uTextureSampler, vec2((fragCoord.x - i)/512, (fragCoord.y)/512));

		FragColor += texture(uTextureSampler, vec2((fragCoord.x)/512, (fragCoord.y + i)/512));
		FragColor += texture(uTextureSampler, vec2((fragCoord.x)/512, (fragCoord.y - i)/512));
	}
}