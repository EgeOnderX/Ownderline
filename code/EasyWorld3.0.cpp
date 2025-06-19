#include <raylib.h>
#include <raymath.h>
#include <bits/stdc++.h>
using namespace std;
// 方块类型枚举
// 地形参数
const int n = 192, m = 192;
const int WORLD_HEIGHT = 256;      // 世界总高度
int a[245][WORLD_HEIGHT][245];  // 三维方块数据 [x][y][z]
double heightMap[8245][8245];     // 保留高度图用于地形生成
double w[8245][8245];
double l[1145][1145];
double neww[8245][8245];
double scale = 0.05;
const int octaves = 3;
double BeAbleSee=64;

// 玩家参数
float playerX = n / 2.0f;
float playerY = 255;
float playerZ = m / 2.0f;
float playerRotation = 0.0f;      // 玩家水平旋转角度
float moveSpeed = 11.4514f; 

// 摄像机参数
float cameraDistance = 10.0f;      // 摄像机跟随距离
float cameraHeight = 2.5f;        // 摄像机高度
float mouseSensitivity = 0.3f;    // 鼠标灵敏度
float cameraPitch = 0.0f;          // 新增垂直视角角度
const float MAX_PITCH = 89.0f;     // 最大俯仰角度
void DrawTerrain() {
	for (int i = playerX-BeAbleSee; i <= playerX+BeAbleSee; ++i) {
		for (int j = playerZ-BeAbleSee; j <= playerZ+BeAbleSee; ++j) {
			if (i < 1 || i >= n || j < 1 || j >= m) continue;
			
			// 从顶部向下扫描找到第一个非空气方块
			for (int y = WORLD_HEIGHT-1; y >= 0; y--) {
				if (a[i][y][j] != 0) {
					Color blockColor = GRAY;
					switch (a[i][y][j]) {
						case 3: blockColor = DARKGRAY; break;
						case 2: blockColor = BROWN; break;
						case 1: blockColor = GREEN; break;
						case 4: blockColor = BLUE; break;
					}
					
					Vector3 pos = { (float)i, (float)y, (float)j };
					DrawCubeV(pos, Vector3{1.0f, 1.0f, 1.0f}, blockColor);
					if(a[i+1][y][j]>0&&a[i-1][y][j]>0&&a[i][y+1][j]>0&&a[i][y-1][j]>0&&a[i][y][j+1]>0&&a[i][y][j-1]>0) continue;
					//break; // 只绘制最顶层的方块
				}
			}
		}
	}
}


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


void build_PerlinNoise() {
	PerlinNoise pn(time(0));
	
	// 生成高度图
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= m; j++) {
			double x = i * scale;
			double y = j * scale;
			
			double total = 0.0;
			double frequency = 1.0;
			double amplitude = 1.0;
			double maxAmplitude = 0.0;
			
			for (int k = 0; k < octaves; ++k) {
				total += pn.noise(x * frequency, y * frequency) * amplitude;
				maxAmplitude += amplitude;
				amplitude *= 0.4;
				frequency *= 1.8;
			}
			heightMap[i][j] = (total / maxAmplitude + 1.0) / 2.0 * WORLD_HEIGHT;
		}
	}
	
	// 根据高度图生成方块
	for (int x = 1; x <= n; x++) {
		for (int z = 1; z <= m; z++) {
			int surfaceHeight = (int)heightMap[x][z];
			
			for (int y = 0; y < WORLD_HEIGHT; y++) {
				if (y > surfaceHeight) {
					a[x][y][z] = 0;
				} else if (y == surfaceHeight) {
					a[x][y][z] = 1;
				} else if (y > surfaceHeight - 4) {
					a[x][y][z] = 2;
				} else {
					a[x][y][z] = 3;
				}
			}
			
			// 处理水面
			if (surfaceHeight < WORLD_HEIGHT / 2) {
				for (int y = surfaceHeight + 1; y <= WORLD_HEIGHT / 2; y++) {
					a[x][y][z] = 4;
				}
			}
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
	playerX = Clamp(playerX + moveDir.x * moveSpeed * deltaTime, 1.0f, n);
	playerZ = Clamp(playerZ + moveDir.z * moveSpeed * deltaTime, 1.0f, m);
	
	
	// 交互操作
	if (IsKeyPressed(KEY_SPACE)) w[(int)playerX][(int)playerZ] += 10.0;
	if (IsKeyPressed(KEY_T)) {
		for (int dx = -1; dx <= 1; ++dx)
			for (int dy = -1; dy <= 1; ++dy)
				w[(int)playerX+dx][(int)playerZ+dy] = 0;
	}
}
/*
void flow(){
memset(neww,0,sizeof(neww));

for(int i=1;i<=n;i++){
for(int j=1;j<=m;j++){
if(w[i][j] < 0.1) continue;

// 边界消减（减缓消减速度）
if((i==1||i==n||j==1||j==m) && w[i][j]>=1){
neww[i][j] -= 0.1; // 改为每次减少0.1
}

// 四方向流动（使用实际地形a数组）
int dx[] = {-1, 0, 1, 0};
int dy[] = {0, -1, 0, 1};
for(int k=0; k<4; k++){
int ni = i + dx[k];
int nj = j + dy[k];

if(ni>=1 && ni<=n && nj>=1 && nj<=m){
// 比较实际地形高度a，而不是l
if(a[i][j] + w[i][j] > a[ni][nj] + w[ni][nj]){ // 考虑水位高度
double heightDiff = (a[i][j] + w[i][j]) - (a[ni][nj] + w[ni][nj]);
double flowAmount = min(w[i][j], heightDiff * 0.2); // 增加流动系数
flowAmount = max(flowAmount, 0.0);
neww[i][j] -= flowAmount;
neww[ni][nj] += flowAmount;
}
}
}
}
}

// 应用变化
for(int i=1;i<=n;i++){
for(int j=1;j<=m;j++){
w[i][j] += neww[i][j];
w[i][j] = max(w[i][j], 0.0);
}
}
}
*/
int main() {
	build_PerlinNoise();
	//LoadHeightmapFromPNG("global_heightmap_8192.png");
	InitWindow(1920, 1080, "3D Terrain");
	SetTargetFPS(60);
	DisableCursor();
	
	Camera3D camera = { 0 };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	camera.up = {0, 1, 0};
	
	while (!WindowShouldClose()) {
		// 鼠标控制
		Vector2 mouseDelta = GetMouseDelta();
		playerRotation -= mouseDelta.x * mouseSensitivity;  // 水平旋转
		cameraPitch += mouseDelta.y * mouseSensitivity * 0.5f; // 垂直视角（降低灵敏度）
		cameraPitch = Clamp(cameraPitch, -MAX_PITCH, MAX_PITCH); // 限制角度
		// 更新摄像机
		Vector3 playerPos = {
			playerX,
			playerY,
			playerZ
		};
		camera.position = {
			// X轴坐标 = 距离 * cos(俯仰) * sin(偏航)
			playerPos.x - cameraDistance * cosf(DEG2RAD*cameraPitch) * sinf(DEG2RAD*playerRotation),
			
			// Y轴坐标 = 距离 * sin(俯仰) + 基础高度
			playerPos.y + cameraDistance * sinf(DEG2RAD*cameraPitch) + cameraHeight,
			
			// Z轴坐标 = 距离 * cos(俯仰) * cos(偏航)
			playerPos.z - cameraDistance * cosf(DEG2RAD*cameraPitch) * cosf(DEG2RAD*playerRotation)
		};
		camera.target = playerPos;
		
		Playerdo();
		//flow();
		BeginDrawing();
		ClearBackground(SKYBLUE);
		BeginMode3D(camera);
		
		// 绘制地形
		DrawTerrain();
		// 绘制玩家
		DrawCubeV(playerPos, {0.8f, 1.6f, 0.8f}, RED);
		DrawCubeWiresV(playerPos, {0.8f, 1.6f, 0.8f}, DARKGRAY);
		
		EndMode3D();
		
		// 调试信息
		DrawText(TextFormat("Position: (%.1f, %.1f)", playerX, playerZ), 10, 10, 20, BLACK);
		DrawFPS(10, 40);
		EndDrawing();
	}
	
	CloseWindow();
	return 0;
}
