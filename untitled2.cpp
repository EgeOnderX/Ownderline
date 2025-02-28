#include <bits/stdc++.h>
#include <raylib.h>
#define itn int
#define sc(s) SetConsoleTextAttribute(hOut,s)
using namespace std;
int maxx=0;
int mx,mn;
int n=100, m=100;
double a[1145][1145];
double w[1145][1145];
double l[1145][1145];
double scale=0.05;
// 人物位置
int playerX = n / 2; // 人物的初始X坐标
int playerY = m / 2; // 人物的初始Y坐标
// 柏林噪声生成器类
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
			mx = max(mx, (int)a[i][j]);
			mn = min(mn, (int)a[i][j]);
		}
	}
}

void flow(){
	// 创建临时数组存储新水量
	double neww[114][114] = {0};
	
	for(int i=1;i<=n;i++){
		for(int j=1;j<=m;j++){
			if(w[i][j] < 0.1) continue;
			
			// 边界消减
			if((i==1||i==n||j==1||j==m) && w[i][j]>=1){
				neww[i][j] -= 0.5;
			}
			
			// 四方向流动
			int dx[] = {-1, 0, 1, 0};
			int dy[] = {0, -1, 0, 1};
			for(int k=0; k<4; k++){
				int ni = i + dx[k];
				int nj = j + dy[k];
				
				if(ni>=1 && ni<=n && nj>=1 && nj<=m){
					if(l[i][j] > l[ni][nj]){
						double flowAmount = min(w[i][j], (l[i][j] - l[ni][nj])*0.1);
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
			if(w[i][j] < 0) w[i][j] = 0;
		}
	}
}

void Playerdo() {
	/*
	if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
		int x, y;
		//getMouseClick(x, y);
		if (x >= 1 && x <= n && y >= 1 && y <= m) {
			w[x][y] += 10.0; // 在点击位置放水
		}
	}
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
		int x, y;
		//getMouseClick(x, y);
		if (x >= 1 && x <= n && y >= 1 && y <= m) {
			w[x][y] += 10.0; // 在点击位置放水
		}
	}
	*/
	if (IsKeyDown(KEY_SPACE)) {
		int x = playerX;
		int y = playerY;
		//getMouseClick(x, y);
		if (x >= 1 && x <= n && y >= 1 && y <= m) {
			w[x][y] += 10.0; // 在点击位置放水
		}
	} 
	if (IsKeyDown(KEY_W)) {
		int newX = playerX;
		int newY = playerY;
		newX++;
		if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
			playerX = newX;
			playerY = newY;
		}
	} 
	if (IsKeyDown(KEY_S)) {
		int newX = playerX;
		int newY = playerY;
		newX--;
		if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
			playerX = newX;
			playerY = newY;
		}
	}
	if (IsKeyDown(KEY_A)) {
		int newX = playerX;
		int newY = playerY;
		newY--;
		if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
			playerX = newX;
			playerY = newY;
		}
	}
	if (IsKeyDown(KEY_D)) {
		int newX = playerX;
		int newY = playerY;
		newY++;
		if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
			playerX = newX;
			playerY = newY;
		}
	}
	if (IsKeyDown(KEY_T)) {
		
		w[playerX-1][playerY-1]=0;
		w[playerX-1][playerY]=0;
		w[playerX-1][playerY+1]=0;
		w[playerX][playerY-1]=0;
		w[playerX][playerY]=0;
		w[playerX][playerY+1]=0;
		w[playerX+1][playerY-1]=0;
		w[playerX+1][playerY]=0;
		w[playerX+1][playerY+1]=0;
		
		
	}
}
void DrawBox(int p1x,int p1y,int p1z,int p2x,int p2y,int p2z, Color color) {
	// 计算尺寸
	Vector3 size = {
		.x = fabsf(p2x - p1x),
		.y = fabsf(p2y - p1y),
		.z = fabsf(p2z - p1z)
	};
	
	// 计算中心点
	Vector3 center = {
		.x = (p1x + p2x) / 2,
		.y = (p1y + p2y) / 2,
		.z = (p1z + p2z) / 2
	};
	
	// 绘制实体和线框
	DrawCube(center, size.x, size.y, size.z, color);
	DrawCubeWires(center, size.x, size.y, size.z, BLACK);
}
float cameraDistance = 15.0f;    // 摄像机跟随距离
float cameraHeight = 10.0f;      // 摄像机基础高度
float cameraLookAtHeight = 2.0f; // 摄像机注视点高度偏移
int main() {
	build_PerlinNoise();
	
	const int screenWidth = 640;
	const int screenHeight = 480;
	
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(screenWidth, screenHeight, "3D Terrain with Camera Follow");
	
	Camera3D camera = { 0 };
	// 初始化摄像机参数
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 70.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	
	SetTargetFPS(30);
	
	while (!WindowShouldClose()) {
		Playerdo();
		flow();
		
		// 更新摄像机位置和方向----------------------------------------
		// 计算玩家实际三维坐标
		Vector3 playerPos = {
			(float)playerX, 
			a[playerX][playerY] * 0.1f,  // 地形高度
			(float)playerY
		};
		
		// 计算摄像机位置（玩家后方上方）
		camera.position = {
			playerPos.x - cameraDistance,
			playerPos.y + cameraHeight,
			playerPos.z - cameraDistance
		};
		
		// 计算摄像机注视点（玩家位置稍上方）
		camera.target = {
			playerPos.x,
			playerPos.y + cameraLookAtHeight,
			playerPos.z
		};
		//-----------------------------------------------------------
		
		BeginDrawing();
		ClearBackground(WHITE);
		BeginMode3D(camera);
		
		// 绘制地形
		for(int i=1;i<=n;i++){
			for(int j=1;j<=m;j++){
				// 地形方块
				DrawCube(
					(Vector3){i, a[i][j]*0.05f, j}, // 调整高度为5%避免地形过高
					1.0f, a[i][j]*0.1f, 1.0f,
					Color{ 100, (unsigned char)(a[i][j]), 100, 255 } // 根据高度着色
					);
				
				// 水位绘制
				if(w[i][j] > 0.1f) {
					DrawCube(
						(Vector3){i, (a[i][j]+w[i][j])*0.05f, j},
						0.8f, w[i][j]*0.1f, 0.8f,
						BLUE
						);
				}
			}
		}
		
		// 绘制玩家（红色立方体）
		DrawCube(playerPos, 0.8f, 1.6f, 0.8f, RED);
		DrawCubeWires(playerPos, 0.8f, 1.6f, 0.8f, DARKGRAY);
		
		// 绘制辅助网格
		DrawGrid(n, 1.0f);
		EndMode3D();
		
		// 显示调试信息
		DrawText(TextFormat("Player Position: (%d, %d)", playerX, playerY), 10, 10, 20, BLACK);
		DrawFPS(10, 30);
		
		EndDrawing();
	}
	
	//关闭窗口
	CloseWindow();
	return 0;
}
//wprintf(L"\033[48;2;%d;%d;%dm\x1b[38;2;%d;%d;%dmLG\n", r, g, b, r, g, b);
