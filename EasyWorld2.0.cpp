#include <raylib.h>
#include <raymath.h>
#include <algorithm>
#include <random>
using namespace std;

// 地形参数
const int n = 314, m = 314;
double a[1145][1145] = {0};
double w[1145][1145] = {0};
double l[1145][1145] = {0};
double scale = 0.03;

// 玩家参数
float playerX = n / 2.0f;
float playerY = m / 2.0f;
float playerRotation = 0.0f;      // 玩家水平旋转角度
float moveSpeed = 11.4514f; 

// 摄像机参数
float cameraDistance = 8.0f;      // 摄像机跟随距离
float cameraHeight = 2.5f;        // 摄像机高度
float mouseSensitivity = 0.2f;    // 鼠标灵敏度

class PerlinNoise {
private:
	int permutation[512];
	
public:
	PerlinNoise(unsigned int seed) {
		// 初始化排列表
		iota(permutation, permutation + 256, 0);
		shuffle(permutation, permutation + 256, default_random_engine(seed));
		// 重复排列表以避免越界
		for (int i = 0; i < 256; ++i)
			permutation[256 + i] = permutation[i];
	}
	
	// 生成二维柏林噪声
	double noise(double x, double y) {
		// 确定网格单元
		int X = (int)floor(x) & 255;
		int Y = (int)floor(y) & 255;
		
		// 网格内相对坐标
		x -= floor(x);
		y -= floor(y);
		
		// 计算缓和曲线
		double u = fade(x);
		double v = fade(y);
		
		// 哈希索引周围四个点
		int A = permutation[X] + Y;
		int AA = permutation[A & 255];
		int AB = permutation[(A + 1) & 255];
		int B = permutation[X + 1] + Y;
		int BA = permutation[B & 255];
		int BB = permutation[(B + 1) & 255];
		
		// 混合梯度贡献
		double gradAA = grad(AA, x, y);
		double gradBA = grad(BA, x - 1, y);
		double gradAB = grad(AB, x, y - 1);
		double gradBB = grad(BB, x - 1, y - 1);
		
		// 双线性插值
		double lerp1 = lerp(u, gradAA, gradBA);
		double lerp2 = lerp(u, gradAB, gradBB);
		return lerp(v, lerp1, lerp2);
	}
	
private:
	// 缓和曲线：6t^5 - 15t^4 + 10t^3
	static double fade(double t) {
		return t * t * t * (t * (t * 6 - 15) + 10);
	}
	
	// 线性插值
	static double lerp(double t, double a, double b) {
		return a + t * (b - a);
	}
	
	// 计算梯度值（使用预定义的四个方向）
	static double grad(int hash, double x, double y) {
		int h = hash & 3;
		switch (h) {
			case 0: return x + y;    // 右上方
			case 1: return -x + y;   // 左上方
			case 2: return x - y;    // 右下方
			case 3: return -x - y;   // 左下方
			default: return 0; // 不会执行
		}
	}
};


void build_PerlinNoise(){
	PerlinNoise pn(time(0)); // 使用当前时间作为种子
	// 生成柏林噪声地形
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= m; j++) {
			// 调整缩放因子以改变地形形态
			double x = i * scale;
			double y = j * scale;
			
			// 生成分形噪声（叠加多个倍频）
			double total = 0.0;
			double frequency = 1.0;
			double amplitude = 1.0;
			double maxAmplitude = 0.0;
			int octaves = 4;
			
			for (int k = 0; k < octaves; ++k) {
				total += pn.noise(x * frequency, y * frequency) * amplitude;
				maxAmplitude += amplitude;
				amplitude *= 0.5;
				frequency *= 2.0;
			}
			total /= maxAmplitude; // 归一化
			
			// 将噪声值转换到0~255并存入数组
			a[i][j] = (total + 1.0) / 2.0 * 255.0;
			// = max(mx, (int)a[i][j]);
			//mn = min(mn, (int)a[i][j]);
		}
	}
}

void Playerdo() {
	Vector2 inputDir = {0};
	if (IsKeyDown(KEY_W)) inputDir.y = 1;
	if (IsKeyDown(KEY_S)) inputDir.y = -1;
	if (IsKeyDown(KEY_A)) inputDir.x = 1;
	if (IsKeyDown(KEY_D)) inputDir.x = -1;
	
	// 修复方向计算
	float cosTheta = cosf(DEG2RAD * playerRotation);
	float sinTheta = sinf(DEG2RAD * playerRotation);
	Vector3 moveDir = {
		inputDir.x * cosTheta + inputDir.y * sinTheta,  // 修复方向符号
		0.0f,
		inputDir.y * cosTheta - inputDir.x * sinTheta   // 修复方向符号
	};
	
	// 应用速度参数和帧时间
	float deltaTime = GetFrameTime();
	playerX = Clamp(playerX + moveDir.x * moveSpeed * deltaTime, 1.0f, n-1.0f);
	playerY = Clamp(playerY + moveDir.z * moveSpeed * deltaTime, 1.0f, m-1.0f);
	
	
	// 交互操作
	if (IsKeyPressed(KEY_SPACE)) w[(int)playerX][(int)playerY] += 10.0;
	if (IsKeyPressed(KEY_T)) {
		for (int dx = -1; dx <= 1; ++dx)
			for (int dy = -1; dy <= 1; ++dy)
				w[(int)playerX+dx][(int)playerY+dy] = 0;
	}
}

int main() {
	build_PerlinNoise();
	InitWindow(1280, 720, "3D Terrain");
	SetTargetFPS(60);
	DisableCursor();
	
	Camera3D camera = { 0 };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	camera.up = {0, 1, 0};
	
	while (!WindowShouldClose()) {
		// 鼠标控制
		Vector2 mouseDelta = GetMouseDelta();
		playerRotation -= mouseDelta.x * mouseSensitivity;
		
		// 更新摄像机
		Vector3 playerPos = {
			playerX,
			a[(int)playerX][(int)playerY] * 0.1f,
			playerY
		};
		
		camera.position = {
			playerPos.x - cameraDistance * sinf(DEG2RAD * playerRotation),
			playerPos.y + cameraHeight,
			playerPos.z - cameraDistance * cosf(DEG2RAD * playerRotation)
		};
		camera.target = playerPos;
		
		Playerdo();
		
		BeginDrawing();
		ClearBackground(SKYBLUE);
		BeginMode3D(camera);
		
		// 绘制地形
		for (int i = 1; i <= n; ++i) {
			for (int j = 1; j <= m; ++j) {
				Vector3 pos = { (float)i, a[i][j] * 0.05f, (float)j };
				DrawCubeV(pos, {1.0f, a[i][j]*0.1f, 1.0f}, Color{ 100, (unsigned char)a[i][j], 100, 255 });
				
				if (w[i][j] > 0.1f) {
					Vector3 waterPos = { pos.x, pos.y + w[i][j]*0.05f, pos.z };
					DrawCubeV(waterPos, {0.8f, w[i][j]*0.1f, 0.8f}, BLUE);
				}
			}
		}
		
		// 绘制玩家
		DrawCubeV(playerPos, {0.8f, 1.6f, 0.8f}, RED);
		DrawCubeWiresV(playerPos, {0.8f, 1.6f, 0.8f}, DARKGRAY);
		
		EndMode3D();
		
		// 调试信息
		DrawText(TextFormat("Position: (%.1f, %.1f)", playerX, playerY), 10, 10, 20, BLACK);
		DrawFPS(10, 40);
		EndDrawing();
	}
	
	CloseWindow();
	return 0;
}
