#version 330

uniform sampler2D tex;

in vec2 texCoord; //текстурные координаты (интерполированы между вершинами треугольника)

out vec4 fragColor; //выходной цвет фрагмента

uniform float power = 1;

void main()
{
	float depth = texture(tex, texCoord).r;	

	fragColor = pow(vec4(depth, depth, depth, 1.0), vec4(power));
}
