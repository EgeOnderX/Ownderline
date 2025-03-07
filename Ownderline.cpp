#include <raylib.h>
#include <raymath.h>
#include <bits/stdc++.h>
#define min(a,b) (a)<(b)?(a):(b)
#define max(a,b) (a)>(b)?(a):(b)
using namespace std;
// 地形参数
const int n = 1192, m = 1192;
double a[8245][8245];
double w[8245][8245];
double l[1145][1145];
double neww[8245][8245];
double scale = 0.05;
const int octaves = 3;
double BeAbleSee=128;

const int hudX = 20;
const int hudY = 20;
const int barWidth = 200;
const int barHeight = 20;

// 玩家参数
float playerX = n / 2.0f;
float playerZ = m / 2.0f;
float playerRotation = 0.0f;      // 玩家水平旋转角度
float moveSpeed = 11.4514f; 

// 摄像机参数
float cameraDistance = 10.0f;      // 摄像机跟随距离
float cameraHeight = 2.5f;        // 摄像机高度
float mouseSensitivity = 0.3f;    // 鼠标灵敏度
float cameraPitch = 0.0f;          // 新增垂直视角角度
const float MAX_PITCH = 89.0f;     // 最大俯仰角度

// 在全局变量区添加生存状态结构体
struct SurvivalStats {
	double k=60/10;
	double health = 100;
	double hunger = 100;
	double thirst = 100;
	float bodyTemp = 37.0f;
	float ambientTemp = 25.0f;
	
	void Update(float deltaTime) {
		static float timer = 0;
		timer += deltaTime;
		
		// 每2秒更新一次状态
		if(timer >= 0.0f) {
			timer = 0;
			
			// 基础消耗
			hunger = max(hunger-2/k,0);
			thirst = max(thirst - 3/k, 0);
			
			// 体温调节
			//bodyTemp += (ambientTemp - bodyTemp)/k  ;
			
			// 健康检测
			if(thirst<=0) health = max(health - 4/k, 0);
			if(hunger<=0) health = max(health - 3/k, 0);
			if(bodyTemp > 39.0f || bodyTemp < 35.0f) health = max(health - 2/k, 0);
		}
	}
};

SurvivalStats g_stats; // 全局生存状态

/*废弃*/bool LoadHeightmapFromPNG(const char* filename);
void build_PerlinNoise();/*生成地形*/
void Playerdo();/*玩家操作*/
void DrawHUD();
/*+---------------------------------------------+*/
/*|祖传屎山，build_PerlinNoise()需要用的，勿动！|*/class PerlinNoise {private:int permutation[512];public:PerlinNoise(unsigned int seed) {/*初始化排列表*/iota(permutation, permutation + 256, 0);shuffle(permutation, permutation + 256, default_random_engine(seed));/* 重复排列表以避免越界*/for (int i = 0; i < 256; ++i)permutation[256 + i] = permutation[i];}/*生成二维柏林噪声*/double noise(double x, double y) {/*确定网格单元*/int X = (int)floor(x) & 255;int Y = (int)floor(y) & 255;/*网格内相对坐标*/x -= floor(x);y -= floor(y);/*计算缓和曲线*/double u = fade(x);double v = fade(y);/*哈希索引周围四个点*/ int A = permutation[X] + Y;int AA = permutation[A & 255];int AB = permutation[(A + 1) & 255];int B = permutation[X + 1] + Y;int BA = permutation[B & 255];int BB = permutation[(B + 1) & 255];/*混合梯度贡献*/ double gradAA = grad(AA, x, y);double gradBA = grad(BA, x - 1, y);double gradAB = grad(AB, x, y - 1);double gradBB = grad(BB, x - 1, y - 1);/* 双线性插值*/double lerp1 = lerp(u, gradAA, gradBA);double lerp2 = lerp(u, gradAB, gradBB);return lerp(v, lerp1, lerp2);}private:/*缓和曲线：6t^5 - 15t^4 + 10t^3*/ static double fade(double t) {return t * t * t * (t * (t * 6 - 15) + 10);}/*线性插值*/static double lerp(double t, double a, double b) {return a + t * (b - a);}/*计算梯度值（使用预定义的四个方向）*/static double grad(int hash, double x, double y) {int h = hash & 3;switch (h) {case 0: return x + y;    /*右上方*/case 1: return -x + y;   /*左上方*/case 2: return x - y;    /*右下方*/case 3: return -x - y;   /*左下方*/default: return 0; /* 不会执行*/}}};
/*+---------------------------------------------+*/
int main() {
	build_PerlinNoise();
	//LoadHeightmapFromPNG("global_heightmap_8192.png");
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(640, 480, "Ownderline");
	SetTargetFPS(60);
	DisableCursor();

	g_stats.ambientTemp = a[(int)playerX][(int)playerZ] * 0.5f;// 初始化生存状态
	Camera3D camera = { 0 };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	camera.up = {0, 1, 0};
	
	while (!WindowShouldClose()) {
	
		//g_stats.ambientTemp = a[(int)playerX][(int)playerZ];// 更新环境温度
		g_stats.Update(GetFrameTime());
		
		// 鼠标控制
		Vector2 mouseDelta = GetMouseDelta();
		playerRotation -= mouseDelta.x * mouseSensitivity;  // 水平旋转
		cameraPitch += mouseDelta.y * mouseSensitivity * 0.5f; // 垂直视角（降低灵敏度）
		cameraPitch = Clamp(cameraPitch, -MAX_PITCH, MAX_PITCH); // 限制角度
		// 更新摄像机
		Vector3 playerPos = {
			playerX,
			a[(int)playerX][(int)playerZ] ,
			playerZ
		};
		camera.position = {
			/*| cameraDistance        ：距离     |*/
			/*| DEG2RAD*cameraPitch   ：俯仰     |*/
			/*| DEG2RAD*playerRotation：偏航     |*/
			/*| cameraHeight          ：基础高度 |*/
			playerPos.x - cameraDistance * cosf(DEG2RAD*cameraPitch) * sinf(DEG2RAD*playerRotation),
			playerPos.y + cameraDistance * sinf(DEG2RAD*cameraPitch) + cameraHeight,
			playerPos.z - cameraDistance * cosf(DEG2RAD*cameraPitch) * cosf(DEG2RAD*playerRotation)
		};
		camera.target = playerPos;
		
		Playerdo();
		//flow();
		BeginDrawing();
		ClearBackground(SKYBLUE);
		BeginMode3D(camera);
		
		// 绘制地形
		for (int i = playerX-BeAbleSee; i <= playerX+BeAbleSee; ++i) {
			for (int j = playerZ-BeAbleSee; j <= playerZ+BeAbleSee; ++j) {
				Vector3 pos = { (float)i, a[i][j] * 0.5f, (float)j };
				DrawCubeV(pos, {1.0f, a[i][j], 1.0f}, Color{ 100, (unsigned char)a[i][j]*10, 100, 255 });
				
				if (w[i][j] > 0.0f) {
					Vector3 waterPos = { pos.x, pos.y + w[i][j]*0.5f, pos.z };
					DrawCubeV(waterPos, {0.8f, w[i][j], 0.8f}, BLUE);
				}
				
			}
		}
		
		// 绘制玩家
		DrawCubeV(playerPos, {0.8f, 1.6f, 0.8f}, RED);
		DrawCubeWiresV(playerPos, {0.8f, 1.6f, 0.8f}, DARKGRAY);
		
		EndMode3D();
		DrawHUD();
		
		EndDrawing();
	}
	
	CloseWindow();
	return 0;
}

void DrawHUD(){
	// 绘制生存状态HUD
	
	DrawRectangle(hudX-10, hudY-10, 300, 160, Color{0, 0, 0, 128});// 半透明背景
	
	
	DrawText("Health:", hudX, hudY, 20, RED);// 生命值
	DrawRectangle(hudX + 80, hudY, barWidth, barHeight, Color{50,50,50,200});
	DrawRectangle(hudX + 80, hudY, barWidth * g_stats.health / 100, barHeight, RED);
	
	
	DrawText("Hunger:", hudX, hudY+40, 20, ORANGE);// 饱食度
	DrawRectangle(hudX + 80, hudY+40, barWidth, barHeight, Color{50,50,50,200});
	DrawRectangle(hudX + 80, hudY+40, barWidth * g_stats.hunger / 100, barHeight, ORANGE);
	
	
	DrawText("Thirst:", hudX, hudY+80, 20, BLUE);// 口渴值
	DrawRectangle(hudX + 80, hudY+80, barWidth, barHeight, Color{50,50,50,200});
	DrawRectangle(hudX + 80, hudY+80, barWidth * g_stats.thirst / 100, barHeight, BLUE);
	
	
	char tempStr[20];// 体温
	sprintf(tempStr, "Temp: %.1fC", g_stats.bodyTemp);
	Color tempColor = (g_stats.bodyTemp >38 || g_stats.bodyTemp <36) ? RED : SKYBLUE;
	DrawText(tempStr, hudX, hudY+120, 20, tempColor);
	
	DrawText(TextFormat("Position: (%.1f,%.1f,%.1f)", playerX, a[(int)playerX][(int)playerZ],playerZ), 10, 10, 20, BLACK);// 调试信息
	DrawFPS(10, 40);
}
void build_PerlinNoise(){
	PerlinNoise pn(time(0));
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
				amplitude *= 0.4;  // 增加衰减速度
				frequency *= 1.8;  // 降低频率倍增
			}
			a[i][j] = (total / maxAmplitude + 1.0) / 2.0 * 255.0 / 10;
		}
	}
	
	// 添加平滑滤波
	for(int s = 0; s < 2; ++s) {  // 进行2次平滑
		for(int i = 2; i < n; ++i) {
			for(int j = 2; j < m; ++j) {
				a[i][j] = (a[i-1][j] + a[i+1][j] + a[i][j-1] + a[i][j+1] + 4*a[i][j]) / 8.0;
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
/*|-------废物部分-------|*/
/*| ！！！屎山部分！！！ |*/
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

bool LoadHeightmapFromPNG(const char* filename) {
	// 加载图像
	Image img = LoadImage(filename);
	if (!img.data) {
		TraceLog(LOG_WARNING, "无法加载高度图文件: %s", filename);
		return false;
	}
	
	// 转换为灰度格式（确保单通道）
	ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
	
	// 检查尺寸是否匹配
	if (img.width != n || img.height != m) {
		TraceLog(LOG_WARNING, "图像尺寸不匹配，需要 %dx%d，实际 %dx%d", 
			n, m, img.width, img.height);
		UnloadImage(img);
		return false;
	}
	
	// 提取灰度数据
	unsigned char *grayData = (unsigned char*)img.data;
	for (int y = 1; y < m; ++y) {
		for (int x = 1; x < n; ++x) {
			// 图像Y轴需要翻转（图像原点在左上，地形原点在左下）
			int imgY = m - 1 - y;
			a[x+1][y+1] = static_cast<double>(grayData[imgY * n + x]);
		}
	}
	
	UnloadImage(img);
	return true;
}
