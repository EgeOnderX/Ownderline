#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D inputTexture;
uniform vec2 screenSize;

// 柔和抖动函数
float softDither(vec2 uv) {
	return fract(sin(dot(uv, vec2(12.9898,78.233))) * 43758.5453);
}
	
	void main() {
		vec3 color = texture(inputTexture, fragTexCoord).rgb;
		
		// 有限颜色抖动（保留更多渐变）
		float dither = softDither(gl_FragCoord.xy) * 0.1; // 抖动幅度降低
		color += dither * 0.1; // 轻微噪声
		
		// FXAA抗锯齿
		vec2 invSize = 1.0 / screenSize;
		vec3 rgbNW = textureOffset(inputTexture, fragTexCoord, ivec2(-1,1)).rgb;
		vec3 rgbNE = textureOffset(inputTexture, fragTexCoord, ivec2(1,1)).rgb;
		vec3 rgbSW = textureOffset(inputTexture, fragTexCoord, ivec2(-1,-1)).rgb;
		vec3 rgbSE = textureOffset(inputTexture, fragTexCoord, ivec2(1,-1)).rgb;
		vec3 rgbM  = texture(inputTexture, fragTexCoord).rgb;
		
		vec3 luma = vec3(0.299, 0.587, 0.114);
		float lumaNW = dot(rgbNW, luma);
		float lumaNE = dot(rgbNE, luma);
		float lumaSW = dot(rgbSW, luma);
		float lumaSE = dot(rgbSE, luma);
		float lumaM  = dot(rgbM,  luma);
		dcc c
		float edgeVert = abs(lumaNW + lumaNE - 2.0*lumaM) + 
		abs(lumaSW + lumaSE - 2.0*lumaM);
		float edgeHorz = abs(lumaNW + lumaSW - 2.0*lumaM) + 
		abs(lumaNE + lumaSE - 2.0*lumaM);
		float edge = sqrt(edgeVert*edgeVert + edgeHorz*edgeHorz);
		
		color = mix(color, rgbM, smoothstep(0.0, 0.1, edge));
		
		finalColor = vec4(color, 1.0);
	}