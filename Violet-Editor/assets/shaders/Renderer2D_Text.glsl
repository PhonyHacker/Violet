// MSDF text shader

#type vertex
#version 450 core

// ���붥������
layout(location = 0) in vec3 a_Position;  // ����λ��
layout(location = 1) in vec4 a_Color;     // ������ɫ
layout(location = 2) in vec2 a_TexCoord;  // ��������
layout(location = 3) in int a_EntityID;   // ʵ��ID

// ����任����
layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;  // ��ͼͶӰ����
};

// ������ɫ������ṹ
struct VertexOutput
{
	vec4 Color;   // ��ɫ
	vec2 TexCoord; // ��������
};

// �����Ƭ����ɫ��������
layout (location = 0) out VertexOutput Output;
layout (location = 2) out flat int v_EntityID;  // ʵ��ID

void main()
{
	Output.Color = a_Color;      // ������ɫ
	Output.TexCoord = a_TexCoord; // ������������
	v_EntityID = a_EntityID;     // ����ʵ��ID

	// ������Ļ�ռ�λ��
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

// Ƭ����ɫ�����
layout(location = 0) out vec4 o_Color;  // �����ɫ
layout(location = 1) out int o_EntityID; // ���ʵ��ID

// ������ɫ������ṹ
struct VertexOutput
{
	vec4 Color;   // ��ɫ
	vec2 TexCoord; // ��������
};

// ����Ӷ�����ɫ�����ݵ�����
layout (location = 0) in VertexOutput Input;
layout (location = 2) in flat int v_EntityID;  // ʵ��ID

// ��������
layout (binding = 0) uniform sampler2D u_FontAtlas;

// ������Ļ���ط�Χ�ĸ�������
float screenPxRange() {
	const float pxRange = 2.0; // ���볡�����ط�Χ
	vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_FontAtlas, 0)); // ��λ��Χ
	vec2 screenTexSize = vec2(1.0)/fwidth(Input.TexCoord); // ��Ļ�����С
	return max(0.5 * dot(unitRange, screenTexSize), 1.0); // ������Ļ���ط�Χ
}

// ��������ֵ����ֵ
float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

void main()
{
	// ��ȡ������ɫ
	vec4 texColor = Input.Color * texture(u_FontAtlas, Input.TexCoord);

	// ��ȡ���볡�� RGB ͨ��
	vec3 msd = texture(u_FontAtlas, Input.TexCoord).rgb;
	float sd = median(msd.r, msd.g, msd.b); // ������ֵ
	float screenPxDistance = screenPxRange() * (sd - 0.5); // ������Ļ���ؾ���
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0); // ���㲻͸����
	if (opacity == 0.0)
		discard; // �����͸����Ϊ0��������Ƭ��

	// ������ɫ
	vec4 bgColor = vec4(0.0);
	o_Color = mix(bgColor, Input.Color, opacity); // ��ϱ�����ɫ��������ɫ
	if (o_Color.a == 0.0)
		discard; // ���������ɫ�� alpha Ϊ0��������Ƭ��

	// ���ʵ��ID
	o_EntityID = v_EntityID;
}
