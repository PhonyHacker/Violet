// MSDF text shader

#type vertex
#version 450 core

// 输入顶点属性
layout(location = 0) in vec3 a_Position;  // 顶点位置
layout(location = 1) in vec4 a_Color;     // 顶点颜色
layout(location = 2) in vec2 a_TexCoord;  // 纹理坐标
layout(location = 3) in int a_EntityID;   // 实体ID

// 相机变换矩阵
layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;  // 视图投影矩阵
};

// 顶点着色器输出结构
struct VertexOutput
{
	vec4 Color;   // 颜色
	vec2 TexCoord; // 纹理坐标
};

// 输出到片段着色器的数据
layout (location = 0) out VertexOutput Output;
layout (location = 2) out flat int v_EntityID;  // 实体ID

void main()
{
	Output.Color = a_Color;      // 传递颜色
	Output.TexCoord = a_TexCoord; // 传递纹理坐标
	v_EntityID = a_EntityID;     // 传递实体ID

	// 计算屏幕空间位置
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

// 片段着色器输出
layout(location = 0) out vec4 o_Color;  // 输出颜色
layout(location = 1) out int o_EntityID; // 输出实体ID

// 顶点着色器输出结构
struct VertexOutput
{
	vec4 Color;   // 颜色
	vec2 TexCoord; // 纹理坐标
};

// 输入从顶点着色器传递的数据
layout (location = 0) in VertexOutput Input;
layout (location = 2) in flat int v_EntityID;  // 实体ID

// 字体纹理
layout (binding = 0) uniform sampler2D u_FontAtlas;

// 计算屏幕像素范围的辅助函数
float screenPxRange() {
	const float pxRange = 2.0; // 距离场的像素范围
	vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_FontAtlas, 0)); // 单位范围
	vec2 screenTexSize = vec2(1.0)/fwidth(Input.TexCoord); // 屏幕纹理大小
	return max(0.5 * dot(unitRange, screenTexSize), 1.0); // 计算屏幕像素范围
}

// 计算三个值的中值
float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

void main()
{
	// 获取纹理颜色
	vec4 texColor = Input.Color * texture(u_FontAtlas, Input.TexCoord);

	// 获取距离场的 RGB 通道
	vec3 msd = texture(u_FontAtlas, Input.TexCoord).rgb;
	float sd = median(msd.r, msd.g, msd.b); // 计算中值
	float screenPxDistance = screenPxRange() * (sd - 0.5); // 计算屏幕像素距离
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0); // 计算不透明度
	if (opacity == 0.0)
		discard; // 如果不透明度为0，丢弃该片段

	// 背景颜色
	vec4 bgColor = vec4(0.0);
	o_Color = mix(bgColor, Input.Color, opacity); // 混合背景颜色和输入颜色
	if (o_Color.a == 0.0)
		discard; // 如果最终颜色的 alpha 为0，丢弃该片段

	// 输出实体ID
	o_EntityID = v_EntityID;
}
