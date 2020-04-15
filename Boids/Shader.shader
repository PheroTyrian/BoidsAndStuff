#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_texCoord;

uniform mat4 u_modelViewProjection;

void main()
{
	gl_Position = u_modelViewProjection * position;
	v_texCoord = texCoord;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 colour;

in vec2 v_texCoord;

uniform vec4 u_colour;
uniform sampler2D u_texture;

void main()
{
	vec4 texColour = texture(u_texture, v_texCoord);
	colour = texColour * u_colour;
};