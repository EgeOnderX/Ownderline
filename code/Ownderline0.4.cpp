#include <raylib.h>
#include <raymath.h>
#include <bits/stdc++.h>
#include <rlgl.h>
#pragma GCC optimize(3)
#define min(_a,_b) (_a)<(_b)?(_a):(_b)
#define max(_a,_b) (_a)>(_b)?(_a):(_b)
using namespace std;

const int renderWidth = 640/4;   // 低分辨率宽度（数值越小像素越明显）
const int renderHeight = 480/4;  // 低分辨率高度
RenderTexture2D target;        // 离屏渲染目标
Shader ditherShader;           // 抖动着色器
// 地形参数
const int n = 512, m = 512;
float a[514][514];
float w[514][514];
float l[514][514];
float neww[514][514];
float scale = 0.005;
const int octaves = 3;
float BeAbleSee=64;

float flowRate = 0.8f;      // 水流速率
float waterViscosity = 0.15f; // 水流粘滞系数
float inertia[514][514][2];// 水流惯性矢量场

// 玩家参数
float playerX = n/ 2.0f;
float playerZ = m/ 2.0f;
float playerRotation = 0.0f;      // 玩家水平旋转角度
float moveSpeed; 

float QXX = n/ 2.0f;
float QXZ = m/ 2.0f;

float lastWPressedTime = 0.0;      // 上次按下W的时间
int wPressCount = 0;                // W键连续按压计数
bool isSprinting = false;           // 是否处于疾跑状态
const float floatPressThreshold = 0.7; // 双击判定时间阈值（秒）

// 摄像机参数
float cameraDistance = 10.0f;      // 摄像机跟随距离
float cameraHeight = 2.5f;        // 摄像机高度
float mouseSensitivity = 0.3f;    // 鼠标灵敏度
float cameraPitch = 0.0f;          // 新增垂直视角角度
const float MAX_PITCH = 89.0f;     // 最大俯仰角度
bool isFirstPerson = 1;

bool HideUI=0;
float Time=7;

//float hun=0.005;
float hun=0;
float PlayerHeight=1.8f;

bool isDoubleClickHeld = false;
bool shiftPressed = false;

enum MyCameraMode { FIRST_PERSON, THIRD_PERSON, ISOMETRIC };
MyCameraMode cameraMode = FIRST_PERSON;
float isoDistance = 25.0f;     // 等轴视角距离
float isoAngle = 45.0f;        // 等轴视角角度
char QX[40][29]={
	"        ##      ####",//0
	"     ###########....###",
	"    #.......##..#.....#",//2
	"   #...#...##......###",
	"   ##...####.#######",//4
	"    ####............##",
	"    #................#",//6
	"   #.................#",
	"  #..................#",//8
	" ############........#",
	" ###.................#",//10
	"    ##...............##",
	"    ##...............###",//12
	"   # #...............# ##",
	"  #  #................# #",//14
	" #   #................#  #",
	"##   #................#  ##",//16
	" #   #................#  #",
	"  #  #................# ##",//18
	"  # #.................##",
	"   ##.................#",//20
	"   ##.................#",
	"    #.................#",//22
	"    #.................#",
	"    #..................#",//24
	"    #..................#",
	"    #..................#",//26
	"   #..................#",
	"   #..................#",//28
	"   ####################",
	"   #                   #",//30
	"   #                   #",
	"   #                   #",//32
	"   #                   #",
	"   #                   #",//34
	"   #                   #",
	"   #                   #",//36
	"   #                  #",
	"   #                  #",//38
	"  ##                  ###"
};

struct DoubleClickDetector {
	unordered_map<int, float> lastPressTime;  // 按键最后按压时间
	unordered_map<int, int> pressCount;       // 连续按压计数
	
	bool Check(int key, float threshold = 0.3f) {
		if (IsKeyPressed(key)) {
			const float currentTime = GetTime();
			
			// 初始化新按键记录
			if (!lastPressTime.count(key)) {
				lastPressTime[key] = 0.0f;
				pressCount[key] = 0;
			}
			
			// 计算时间差并更新状态
			if (currentTime - lastPressTime[key] < threshold) {
				pressCount[key]++;
			} else {
				pressCount[key] = 1;  // 超时重新计数
			}
			
			lastPressTime[key] = currentTime;
			
			// 当达到双击时重置并返回true
			if (pressCount[key] >= 2) {
				pressCount[key] = 0;
				return true;
			}
		}
		return false;
	}
	
	// 可选：外部手动重置状态
	void Reset(int key) {
		lastPressTime[key] = 0.0f;
		pressCount[key] = 0;
	}
}g_doubleClick;
// 在全局变量区添加生存状态结构体
struct SurvivalStats {
	float health = 100;
	float hunger = 100;
	float bodyTemp = 37.0f;
	float ambientTemp = 25.0f;
	
	void Update(float deltaTime) {
		static float timer = 0;
		timer += deltaTime;
		
		// 更新一次状态
		if(timer >= 0.01f) {
			timer = 0;
			
			// 基础消耗
			hunger = max(hunger-hun,0);
			
			// 体温调节
			//bodyTemp += (ambientTemp - bodyTemp)/k  ;
			
			// 健康检测
			if(hunger<=0) health = max(health - 0.5, 0);
			//if(bodyTemp > 39.0f || bodyTemp < 35.0f) health = max(health - 0.04, 0);
		}
	}
}g_stats;
struct HitInfo {
	int i;
	int j;
	bool isTerrain;
	Vector3 position;
	Vector3 normal;
	Vector3 v1, v2, v3; // 新增三个顶点坐标
	bool isFirstTriangle; // 标识是否是第一个三角形
};
static HitInfo lastHit = {-1, -1, false, false, {0}, {0}};
void flow();
/*废弃*/bool LoadHeightmapFromPNG(const char* filename);
void build_PerlinNoise();/*生成地形*/
void Playerdo(Camera3D camera);/*玩家操作*/
void DrawHUD();
void InitFile();
float GetTerrainHeight(float x, float z);
HitInfo Raycast(Vector3 start, Vector3 dir, float maxDist);
/*+---------------------------------------------+*/
/*|祖传屎山，build_PerlinNoise()需要用的，勿动！|*/class PerlinNoise {private:int permutation[512];public:PerlinNoise(unsigned int seed) {/*初始化排列表*/iota(permutation, permutation + 256, 0);shuffle(permutation, permutation + 256, default_random_engine(seed));/* 重复排列表以避免越界*/for (int i = 0; i < 256; ++i)permutation[256 + i] = permutation[i];}/*生成二维柏林噪声*/float noise(float x, float y) {/*确定网格单元*/int X = (int)floor(x) & 255;int Y = (int)floor(y) & 255;/*网格内相对坐标*/x -= floor(x);y -= floor(y);/*计算缓和曲线*/float u = fade(x);float v = fade(y);/*哈希索引周围四个点*/ int A = permutation[X] + Y;int AA = permutation[A & 255];int AB = permutation[(A + 1) & 255];int B = permutation[X + 1] + Y;int BA = permutation[B & 255];int BB = permutation[(B + 1) & 255];/*混合梯度贡献*/ float gradAA = grad(AA, x, y);float gradBA = grad(BA, x - 1, y);float gradAB = grad(AB, x, y - 1);float gradBB = grad(BB, x - 1, y - 1);/* 双线性插值*/float lerp1 = lerp(u, gradAA, gradBA);float lerp2 = lerp(u, gradAB, gradBB);return lerp(v, lerp1, lerp2);}private:/*缓和曲线：6t^5 - 15t^4 + 10t^3*/ static float fade(float t) {return t * t * t * (t * (t * 6 - 15) + 10);}/*线性插值*/static float lerp(float t, float a, float b) {return a + t * (b - a);}/*计算梯度值（使用预定义的四个方向）*/static float grad(int hash, float x, float y) {int h = hash & 3;switch (h) {case 0: return x + y;    /*右上方*/case 1: return -x + y;   /*左上方*/case 2: return x - y;    /*右下方*/case 3: return -x - y;   /*左下方*/default: return 0; /* 不会执行*/}}};
/*+---------------------------------------------+*/
int main() {
	InitFile();
	build_PerlinNoise();
	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI );
	
	InitWindow(640, 480, "Ownderline0.4");
	target = LoadRenderTexture(renderWidth, renderHeight); // 创建离屏画布
	SetTextureFilter(target.texture, TEXTURE_FILTER_POINT); // 关键：设置点过滤模式
	ditherShader = LoadShader(0, "dither.fs");
	SetTargetFPS(114514);
	DisableCursor();
	
	g_stats.ambientTemp = a[(int)round(playerX)][(int)round(playerZ)] * 0.5f;// 初始化生存状态
	Camera3D camera = { 0 };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	camera.up = {0, 1, 0};
	
	
	for (int i = 0+QXX-5; i < 40+QXX+5; i++) for (int j = 0+QXZ-5; j < 27+QXZ+5; j++) a[i][j]=1;
	//for (int i = 1; i <=n; i++) {for (int j = 1; j <=m; j++) {a[i][j]=1;
	while (!WindowShouldClose()) {
		if(w[256][256]<0.1) w[256][256]=0.1;
		w[256][256]+=10.0/w[256][256];
		moveSpeed=g_stats.hunger*0.06f+1;
		moveSpeed=moveSpeed/1.8f*PlayerHeight;
		Time+=0.005;
		
		//for(int i = 1; i <= n; i++) for(int j = 1; j <= m; j++) if(w[i][j]<max(Time-a[i][j],0))w[i][j]+=max(Time-a[i][j],0);
		
		/*
		int i=600,j=600;
		w[i][j]=max(Time*100-a[i][j],0);
		*/
		//g_stats.ambientTemp = a[(int)playerX][(int)playerZ];// 更新环境温度
		g_stats.Update(GetFrameTime());
		
		// 鼠标控制
		Vector2 mouseDelta = GetMouseDelta();
		playerRotation -= mouseDelta.x * mouseSensitivity;  // 水平旋转
		cameraPitch += mouseDelta.y * mouseSensitivity * 0.5f; // 垂直视角（降低灵敏度）
		cameraPitch = Clamp(cameraPitch, -MAX_PITCH, MAX_PITCH); // 限制角度
		
		// 鼠标滚轮控制
		float wheel = GetMouseWheelMove();
		if (cameraMode == THIRD_PERSON) {
			cameraDistance = cameraDistance - wheel * 2.0f;
		}
		
		if ((IsKeyDown(KEY_LEFT_CONTROL)||IsKeyDown(KEY_RIGHT_CONTROL))&&(IsKeyDown(KEY_KP_0)||IsKeyDown(KEY_ZERO))&&!isFirstPerson) cameraDistance=10.0f;
		
		Vector3 playerPos = {
			playerX,
			GetTerrainHeight(playerX, playerZ),
			playerZ
		};
		
		switch (cameraMode) {
			case FIRST_PERSON: {
				camera.position = {
					playerPos.x,
					playerPos.y + PlayerHeight/1.8f*1.6f,
					playerPos.z
				};
				
				float yaw = DEG2RAD * playerRotation;
				float pitch = -DEG2RAD * cameraPitch;
				Vector3 front = {
					sinf(yaw) * cosf(pitch),
					sinf(pitch),
					cosf(yaw) * cosf(pitch)
				};
				front = Vector3Normalize(front);
				camera.target = {
					camera.position.x + front.x,
					camera.position.y + front.y,
					camera.position.z + front.z
				};
				camera.projection = CAMERA_PERSPECTIVE;
				break;
			}
			case THIRD_PERSON: {
				camera.position = {
					playerPos.x - cameraDistance * cosf(DEG2RAD*cameraPitch) * sinf(DEG2RAD*playerRotation),
					playerPos.y + cameraDistance * sinf(DEG2RAD*cameraPitch) + cameraHeight,
					playerPos.z - cameraDistance * cosf(DEG2RAD*cameraPitch) * cosf(DEG2RAD*playerRotation)
				};
				camera.target = playerPos;
				camera.projection = CAMERA_PERSPECTIVE;
				break;
			}
			case ISOMETRIC: {
				const float baseHeight = 5.0f;
				camera.position = {
					playerPos.x + isoDistance * cosf(DEG2RAD*(isoAngle)),
					playerPos.y + baseHeight + isoDistance,
					playerPos.z + isoDistance * sinf(DEG2RAD*(isoAngle))
				};
				camera.target = playerPos;
				camera.up = (Vector3){0, 1, 0};
				camera.projection = CAMERA_ORTHOGRAPHIC;
				camera.fovy = 50.0f;
				break;
			}
		}
		
		if (cameraMode != ISOMETRIC) {
			Vector2 mouseDelta = GetMouseDelta();
			playerRotation -= mouseDelta.x * mouseSensitivity;
			cameraPitch += mouseDelta.y * mouseSensitivity * 0.5f;
			cameraPitch = Clamp(cameraPitch, -MAX_PITCH, MAX_PITCH);
		}
		Playerdo(camera);
		static float simAccumulator = 0.0f;
		simAccumulator += GetFrameTime();
		if (simAccumulator >= 1.0f/24.0f) { // 物理更新
			flow();
			simAccumulator -= 1.0f/24.0f;
		}
		BeginTextureMode(target);
		ClearBackground(SKYBLUE);
		BeginMode3D(camera);
		BeginScissorMode(0,0,GetScreenWidth(),GetScreenHeight());
		// 绘制地形
		BeginBlendMode(BLEND_ALPHA);
		
		for (int i = 0; i < 40; i++) {
			for (int j = 0; j < 27; j++) {
				Vector3 pos = { (float)i+QXX, 2, (float)j+QXZ };
				if(QX[i][j]=='#') DrawCubeV(pos, {1.0f, 1.0f, 1.0f}, BLACK);
				else if(QX[i][j]=='.') DrawCubeV(pos, {1.0f, 1.0f, 1.0f}, WHITE);
			}
		}
	//	for (int i = max(playerX-BeAbleSee,1); i <= min(playerX+BeAbleSee,n); i++) {
	//		for (int j =max( playerZ-BeAbleSee,1); j <= min(playerZ+BeAbleSee,m); j++) {
		for (int i = playerX-BeAbleSee; i <= playerX+BeAbleSee; i++) {
			for (int j =playerZ-BeAbleSee; j <= playerZ+BeAbleSee; j++) {
				if (1) {
					// 定义四边形四个顶点（Y轴向上）
					Vector3 v1 = { i, a[i][j], j };
					Vector3 v2 = { i+1,a[i+1][j], j };
					Vector3 v3 = { i,a[i][j+1], j+1 };
					Vector3 v4 = { i+1,a[i+1][j+1], j+1 };
					
					// 绘制第一个三角形（逆时针顺序）
					DrawTriangle3D(v1, v3, v2, Color{
						100, 
						(unsigned char)Clamp((v1.y+ v3.y +v2.y)*0.6*2, 0, 255),
						100, 
						255
					});
					
					// 绘制第二个三角形（逆时针顺序）
					DrawTriangle3D(v2, v3, v4, Color{
						100,
						(unsigned char)Clamp((v2.y+ v3.y +v2.y)*0.6*2, 0, 255),
						100,
						255
					});
				}
				// 定义四边形四个顶点（Y轴向上）
				Vector3 v1 = { i, a[i][j]+w[i][j], j };
				Vector3 v2 = { i+1,a[i+1][j]+w[i+1][j], j };
				Vector3 v3 = { i,a[i][j+1]+w[i][j+1], j+1 };
				Vector3 v4 = { i+1,a[i+1][j+1]+w[i+1][j+1], j+1 };
				if (w[i][j]+w[i+1][j]+w[i][j+1]>0.0001) {
					const float depth =w[i][j]+w[i+1][j]+w[i][j+1]*0.6;
					const unsigned char alpha = (unsigned char)Clamp(80 + depth * 20, 100, 200);
					const Color waterColor = {
						(unsigned char)Clamp(50 + depth * 5, 50, 100),   // R
						(unsigned char)Clamp(180 - depth * 3, 100, 220), // G 
						(unsigned char)Clamp(220 + depth * 15, 220, 255),// B
						(unsigned char)Clamp(100 + depth * 10, 120, 200) // Alpha
					};
					// 绘制第一个三角形（逆时针顺序）
					DrawTriangle3D(v1, v3, v2,ColorAlpha(BLUE,0.5f));
					
				}
				if (w[i+1][j]+w[i][j+1]+w[i+1][j+1]>0.0001) {
					const float depth =w[i][j]+w[i+1][j]+w[i][j+1]*0.6;
					const unsigned char alpha = (unsigned char)Clamp(80 + depth * 20, 100, 200);
					const Color waterColor = {
						(unsigned char)Clamp(50 + depth * 5, 50, 100),   // R
						(unsigned char)Clamp(180 - depth * 3, 100, 220), // G 
						(unsigned char)Clamp(220 + depth * 15, 220, 255),// B
						(unsigned char)Clamp(100 + depth * 10, 120, 200) // Alpha
					};
					// 绘制第二个三角形（逆时针顺序）
					DrawTriangle3D(v2, v3, v4,ColorAlpha(BLUE, 0.5f));
				}
				
			}
		}
		
		if(cameraMode != FIRST_PERSON){
			// 计算立方体中心位置（考虑玩家高度）
			Vector3 cubeCenter = {
				playerPos.x,
				playerPos.y + PlayerHeight/2.0f, // 将立方体垂直居中
				playerPos.z
			};
			
			// 应用旋转和平移变换
			rlPushMatrix();
			rlTranslatef(cubeCenter.x, cubeCenter.y, cubeCenter.z);
			rlRotatef(playerRotation, 0.0f, 1.0f, 0.0f); // 绕Y轴旋转（注意符号）
			
			// 绘制旋转后的立方体（原点位于中心）
			DrawCube(Vector3Zero(), 0.8f, PlayerHeight, 0.8f, RED);
			DrawCubeWiresV(Vector3Zero(), (Vector3){0.8f, PlayerHeight, 0.8f}, DARKGRAY);
			
			rlPopMatrix();
		}
		if(lastHit.i != -1) {
			Vector3 v1, v2, v3;
			
			if(lastHit.isFirstTriangle) {
				v1 = {lastHit.i, a[lastHit.i][lastHit.j], lastHit.j};
				v2 = {lastHit.i, a[lastHit.i][lastHit.j+1], lastHit.j+1};
				v3 = {lastHit.i+1, a[lastHit.i+1][lastHit.j], lastHit.j};
			} else {
				v1 = {lastHit.i+1, a[lastHit.i+1][lastHit.j], lastHit.j};
				v2 = {lastHit.i, a[lastHit.i][lastHit.j+1], lastHit.j+1};
				v3 = {lastHit.i+1, a[lastHit.i+1][lastHit.j+1], lastHit.j+1};
			}
			
			if(lastHit.i != -1) {
				// 直接使用存储的三个顶点
				DrawLine3D(lastHit.v1, lastHit.v2, YELLOW);
				DrawLine3D(lastHit.v2, lastHit.v3, YELLOW);
				DrawLine3D(lastHit.v3, lastHit.v1, YELLOW);
				
				// 在交点处绘制小立方体
				DrawCubeWires(lastHit.position, 0.1f, 0.1f, 0.1f, RED);
			}
			
			// 在交点处绘制小立方体
			DrawCubeWires(lastHit.position, 0.1f, 0.1f, 0.1f, RED);
		}
		EndBlendMode();
		EndScissorMode();
		EndMode3D();
		EndTextureMode();
		
		// ==== 第二步：渲染到主窗口 ====
		BeginDrawing();
		ClearBackground(BLACK); // 背景设为黑色防止边缘露馅
		
		// 应用像素化效果
		//BeginShaderMode(ditherShader); // 开启着色器
		DrawTexturePro(
			target.texture, // 离屏纹理
			(Rectangle){ 0, 0, renderWidth, -renderHeight }, // 源区域
			(Rectangle){ 0, 0, GetScreenWidth(), GetScreenHeight() }, // 目标区域
			(Vector2){ 0, 0 }, 
			0, 
			WHITE
			);
		//EndShaderMode();
		
		// 绘制UI（保持清晰）
		if(!HideUI) DrawHUD();
		
		EndDrawing();
	}
	UnloadRenderTexture(target); // 释放离屏目标
	UnloadShader(ditherShader);  // 释放着色器
	CloseWindow();
	return 0;
}

void DrawHUD(){
	
	DrawRectangle(0,GetScreenHeight()-20, g_stats.health / 100*GetScreenWidth()/2, 20, RED);
	DrawText("Health:", 0+10, GetScreenHeight()-20, 20, WHITE);// 生命值
	
	
	DrawRectangle(GetScreenWidth()/2, GetScreenHeight()-20, g_stats.hunger / 100*GetScreenWidth()/2, 20, ORANGE);
	DrawText("Hunger:",GetScreenWidth()/2+10, GetScreenHeight()-20, 20, WHITE);// 饱食度
	/*
	char tempStr[20];// 体温
	sprintf(tempStr, "Temp: %.1fC", g_stats.bodyTemp);
	Color tempColor = (g_stats.bodyTemp >38 || g_stats.bodyTemp <36) ? RED : SKYBLUE;
	DrawText(tempStr, hudX, hudY+120, 20, tempColor);
	//其他
	*/
	
	DrawText(TextFormat("%.1f,%.1f,%.1f", playerX, GetTerrainHeight(playerX, playerZ),playerZ), 10, 10, 20, BLACK);// 调试信息
	DrawFPS(10, 40);
	
	if (cameraMode == FIRST_PERSON) {
		int centerX = GetScreenWidth() / 2;
		int centerY = GetScreenHeight() / 2;
		DrawLine(centerX - 10, centerY, centerX + 10, centerY, WHITE);
		DrawLine(centerX, centerY - 10, centerX, centerY + 10, WHITE);
	}
}

void Playerdo(Camera3D camera) {
	
	if (IsKeyPressed(KEY_F1)) HideUI = !HideUI;
	if (IsKeyPressed(KEY_F2)) {
		time_t now = time(NULL);
		struct tm *tm = localtime(&now);
		char filename[256];
		strftime(filename, sizeof(filename), "screenshots_%Y%m%d_%H%M%S.png", tm);
		TakeScreenshot(filename); 
	}
	
	// F5切换视角模式
	if (IsKeyPressed(KEY_F5)) {
		cameraMode = static_cast<MyCameraMode>((static_cast<int>(cameraMode) + 1));
		if(static_cast<int>(cameraMode) > ISOMETRIC) cameraMode = FIRST_PERSON;
		
		if(cameraMode == ISOMETRIC) {
			isoDistance = 25.0f;
			isoAngle = 45.0f;
		}
	}
	
	// 等轴视角滚轮控制
	if (cameraMode == ISOMETRIC) {
		float wheel = GetMouseWheelMove();
		isoDistance = Clamp(isoDistance - wheel * 2.0f, 10.0f, 100.0f);
	}
	if (g_doubleClick.Check(KEY_LEFT_SHIFT)) {
		isDoubleClickHeld = true;
	}
	
	// 按下状态处理
	if (IsKeyDown(KEY_LEFT_SHIFT)) {
		if (isDoubleClickHeld) {
			PlayerHeight = 0.4f;  // 双击后按住
		} else {
			PlayerHeight = 0.9f;   // 普通按住
		}
		shiftPressed = true;
	} 
	// 释放状态处理
	else if (IsKeyReleased(KEY_LEFT_SHIFT)) {
		PlayerHeight = 1.8f;
		isDoubleClickHeld = false;
		shiftPressed = false;
	}
	
	// 防止在未双击时保持状态
	if (!shiftPressed) {
		isDoubleClickHeld = false;
	}
	if (IsKeyReleased(KEY_LEFT_SHIFT)) PlayerHeight=1.8f;
	if (IsKeyDown(KEY_SPACE)) w[(int)round(playerX)][(int)round(playerZ)] += 100.0;
	if (IsKeyDown(KEY_T)) {
		for (int dx = -5; dx <= 5; ++dx)
			for (int dy = -5; dy <= 5; ++dy)
				w[(int)round(playerX)+dx][(int)round(playerZ)+dy] = 0;
	}
	if (IsKeyDown(KEY_Q)) {
		for (int dx = -2; dx <= 2; ++dx)
			for (int dy = -2; dy <= 2; ++dy)
				a[(int)round(playerX)+dx][(int)round(playerZ)+dy] -=0.25;
	}
	if (IsKeyDown(KEY_E)) {
		for (int dx = -2; dx <= 2; ++dx)
			for (int dy = -2; dy <= 2; ++dy)
				a[(int)round(playerX)+dx][(int)round(playerZ)+dy] +=0.25;
	}
	if (g_doubleClick.Check(KEY_W))isSprinting = true;// 检测双击W逻辑
	if (IsKeyReleased(KEY_W)) isSprinting = false;// 松开W时重置疾跑状态
	Vector2 inputDir = {0};
	if (IsKeyDown(KEY_W)) {inputDir.y = 1;g_stats.hunger = max(g_stats.hunger - hun*2, 0);}
	if (IsKeyDown(KEY_S)) {inputDir.y = -1;g_stats.hunger = max(g_stats.hunger - hun*2, 0);}
	if (IsKeyDown(KEY_A)) {inputDir.x = 1;g_stats.hunger = max(g_stats.hunger - hun*2, 0);}
	if (IsKeyDown(KEY_D)) {inputDir.x = -1;g_stats.hunger = max(g_stats.hunger -hun*2, 0);}
	float currentSpeed = moveSpeed;
	if (isSprinting && inputDir.y == 1) { // 只有向前时加速
		currentSpeed *= 2.0f; // 疾跑速度翻倍
		// 这里可以添加体力消耗逻辑
		g_stats.hunger = max(g_stats.hunger - 2.5*hun, 0);
	}
	
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
	playerX = Clamp(playerX + moveDir.x * currentSpeed * deltaTime, 1.0f, n);
	playerZ = Clamp(playerZ + moveDir.z * currentSpeed * deltaTime, 1.0f, m);
	
	
	
	// 交互操作
	if (cameraMode == FIRST_PERSON) {
		// 计算射线方向
		float yaw = DEG2RAD * playerRotation;
		float pitch = -DEG2RAD * cameraPitch;
		Vector3 direction = {
			sinf(yaw) * cosf(pitch),
			sinf(pitch),
			cosf(yaw) * cosf(pitch)
		};
		direction = Vector3Normalize(direction);
		HitInfo hit = Raycast(camera.position, direction, 10.0f);
		lastHit = hit; // 存储当前帧的命中信息
		// 破坏方块（鼠标左键）
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			HitInfo hit = Raycast(camera.position, direction, 10.0f);
			if (hit.i != -1) {
				//if (hit.isTerrain) {
				a[(int)hit.v1.x][(int)hit.v1.z]--;
				a[(int)hit.v2.x][(int)hit.v2.z]--;
				a[(int)hit.v3.x][(int)hit.v3.z]--;
				
				
				//} 
			}
		}
		
		// 放置方块（鼠标右键）
		if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
			HitInfo hit = Raycast(camera.position, direction, 10.0f);
			if (hit.i != -1) {
				a[(int)hit.v1.x][(int)hit.v1.z]++;
				a[(int)hit.v2.x][(int)hit.v2.z]++;
				a[(int)hit.v3.x][(int)hit.v3.z]++;
				/*
				// 根据法线方向确定放置位置
				int placeI = hit.i + round(hit.normal.x);
				int placeJ = hit.j + round(hit.normal.z);
				
				// 确保位置有效
				if (placeI >= 1 && placeI <= n && placeJ >= 1 && placeJ <= m) {
				// 放置地形方块
				if (Vector3Length(hit.normal) > 0.5f) {
				a[placeI][placeJ] += 1.0f;
				}
				}
				*/
				
			}
		}
	}
	
}
/*------------------*/
void build_PerlinNoise() {
	PerlinNoise pn(time(0));
	
	// 噪声参数配置
	const float baseScale = 0.03f;    // 基础噪声尺度
	const int octaves = 5;            // 噪声层数
	const float persistence = 0.45f;  // 振幅衰减率
	const float lacunarity = 2.0f;    // 频率倍增率
	
	
	// 第一阶段：生成基础地形
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= m; j++) {
			float x = i * baseScale;
			float y = j * baseScale;
			
			float total = 0.0;
			float amplitude = 1.0;
			float frequency = 1.0;
			float maxAmplitude = 0.0;
			
			// 多层噪声叠加
			for (int k = 0; k < octaves; ++k) {
				total += pn.noise(x * frequency, y * frequency) * amplitude;
				maxAmplitude += amplitude;
				amplitude *= persistence;
				frequency *= lacunarity;
			}
			
			// 高度归一化与曲线调整
			float height = (total / maxAmplitude) * 0.5 + 0.5; // [-1,1] -> [0,1]
			height = pow(height, 1.5f);  // 增强中间过渡
			
			// 基础高度映射
			a[i][j] = height * 50.0f;    // 最大高度50单位
		}
	}
	
	// 第二阶段：地形平滑（5x5高斯模糊）
	for(int s = 0; s < 3; ++s) {
		for(int i = 3; i < n-3; ++i) {
			for(int j = 3; j < m-3; ++j) {
				float sum = 
				a[i-2][j] * 0.05f + a[i-1][j] * 0.1f +
				a[i+1][j] * 0.1f + a[i+2][j] * 0.05f +
				a[i][j-2] * 0.05f + a[i][j-1] * 0.1f +
				a[i][j+1] * 0.1f + a[i][j+2] * 0.05f +
				a[i][j] * 0.4f;
				a[i][j] = sum;
			}
		}
	}
	
	// 第三阶段：侵蚀模拟
	const float erosionStrength = 0.3f;
	const int erosionRadius = 2;
	for(int i = erosionRadius; i < n-erosionRadius; ++i) {
		for(int j = erosionRadius; j < m-erosionRadius; ++j) {
			float maxSlope = 0;
			
			// 检测周围最大坡度
			for(int dx = -1; dx <= 1; ++dx) {
				for(int dz = -1; dz <= 1; ++dz) {
					if(dx == 0 && dz == 0) continue;
					float slope = a[i+dx][j+dz] - a[i][j];
					maxSlope = max(maxSlope, slope);
				}
			}
			
			// 坡度侵蚀处理
			if(maxSlope > 1.5f) {
				float erosion = maxSlope * erosionStrength;
				for(int dx = -erosionRadius; dx <= erosionRadius; ++dx) {
					for(int dz = -erosionRadius; dz <= erosionRadius; ++dz) {
						float weight = 1.0f / (1 + abs(dx) + abs(dz));
						a[i+dx][j+dz] -= erosion * weight;
					}
				}
			}
		}
	}
	
	// 第四阶段：添加细节噪声
	const float detailScale = 0.2f;
	for(int i = 1; i <= n; i++) {
		for(int j = 1; j <= m; j++) {
			float x = i * detailScale;
			float y = j * detailScale;
			
			// 生成细节噪声（范围-2.5~+2.5）
			float detail = pn.noise(x, y) * 2.5f; 
			
			// 根据地势调整细节强度
			float heightFactor = 1.0f - (a[i][j] / 50.0f);
			detail *= heightFactor * 0.8f;  // 低处细节更强
			
			a[i][j] += detail;
		}
	}
	
	// 最终高度限制
	for(int i = 1; i <= n; i++) {
		for(int j = 1; j <= m; j++) {
			a[i][j] =a[i][j]*2;
			w[i][j]=max(12.5-a[i][j],0.1);
		}
	}
}
HitInfo Raycast(Vector3 start, Vector3 dir, float maxDist) {
	HitInfo result = {-1, -1, false, {0}, {0}, {0}, {0}, {0}, false};
	dir = Vector3Normalize(dir);
	
	// 初始化DDA参数
	int stepX = (dir.x > 0) ? 1 : -1;
	int stepZ = (dir.z > 0) ? 1 : -1;
	
	float tDeltaX = (dir.x != 0) ? 1.0f/fabs(dir.x) : FLT_MAX;
	float tDeltaZ = (dir.z != 0) ? 1.0f/fabs(dir.z) : FLT_MAX;
	
	float tMaxX = (dir.x != 0) ? ((dir.x > 0 ? ceil(start.x) : floor(start.x)) - start.x)/dir.x : FLT_MAX;
	float tMaxZ = (dir.z != 0) ? ((dir.z > 0 ? ceil(start.z) : floor(start.z)) - start.z)/dir.z : FLT_MAX;
	
	int mapX = (int)floor(start.x);
	int mapZ = (int)floor(start.z);
	float traveled = 0.0f;
	
	while(traveled < maxDist) {
		// 检查当前网格是否有效
		if(mapX >= 1 && mapX <= n && mapZ >= 1 && mapZ <= m) {
			// 计算当前射线位置
			Vector3 currentPos = Vector3Add(start, Vector3Scale(dir, traveled));
			
			// 获取当前网格的四个顶点
			Vector3 v1 = { (float)mapX, a[mapX][mapZ], (float)mapZ };
			Vector3 v2 = { (float)(mapX+1), a[mapX+1][mapZ], (float)mapZ };
			Vector3 v3 = { (float)mapX, a[mapX][mapZ+1], (float)(mapZ+1) };
			Vector3 v4 = { (float)(mapX+1), a[mapX+1][mapZ+1], (float)(mapZ+1) };
			
			// 检测第一个三角形 (v1, v3, v2)
			RayCollision tri1Collision = GetRayCollisionTriangle(
				Ray{start, dir},
				v1, v3, v2
				);
			
			// 检测第二个三角形 (v2, v3, v4)
			RayCollision tri2Collision = GetRayCollisionTriangle(
				Ray{start, dir},
				v2, v3, v4
				);
			
			// 选择最近的碰撞
			bool hitTri1 = tri1Collision.hit && tri1Collision.distance <= maxDist;
			bool hitTri2 = tri2Collision.hit && tri2Collision.distance <= maxDist;
			
			if(hitTri1 || hitTri2) {
				result.i = mapX;
				result.j = mapZ;
				result.position = hitTri1 ? tri1Collision.point : tri2Collision.point;
				result.normal = hitTri1 ? tri1Collision.normal : tri2Collision.normal;
				
				if(hitTri1) {
					result.v1 = v1;
					result.v2 = v3;
					result.v3 = v2;
					result.isFirstTriangle = true;
				} else {
					result.v1 = v2;
					result.v2 = v3;
					result.v3 = v4;
					result.isFirstTriangle = false;
				}
				
				// 判断是地形还是水体
				float waterHeight = w[mapX][mapZ];
				result.isTerrain = (result.position.y <= a[mapX][mapZ] + 0.1f);
				
				return result;
			}
		}
		
		// 步进到下一个网格
		if(tMaxX < tMaxZ) {
			traveled = tMaxX;
			tMaxX += tDeltaX;
			mapX += stepX;
		} else {
			traveled = tMaxZ;
			tMaxZ += tDeltaZ;
			mapZ += stepZ;
		}
	}
	return result;
}
/*|-------废物部分-------|*/
/*| ！！！屎山部分！！！ |*/


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
			a[x+1][y+1] = static_cast<float>(grayData[imgY * n + x]);
		}
	}
	
	UnloadImage(img);
	return true;
}
float GetTerrainHeight(float x, float z) {
	// 转换为网格坐标
	int i = static_cast<int>(floor(x));
	int j = static_cast<int>(floor(z));
	
	// 边界保护
	i = Clamp(i, 1, n); // 确保i+1不越界
	j = Clamp(j, 1, m); // 确保j+1不越界
	
	// 计算相对位置
	float dx = x - i;
	float dz = z - j;
	
	// 双线性插值
	return a[i][j]     * (1-dx)*(1-dz) +
	a[i+1][j]   * dx*(1-dz) +
	a[i][j+1]   * (1-dx)*dz +
	a[i+1][j+1] * dx*dz;
}
void flow() {
	static const int dx[] = {-1, 0, 1, 0}; // 四邻域方向
	static const int dy[] = {0, -1, 0, 1};
	const float flowRate = 2.5f;           // 提升基础流动速率
	const float inertiaFactor = 0.35f;     // 增强惯性影响
	const float viscosity = 0.75f;         // 降低粘滞衰减
	const float maxFlowRatio = 0.7f;       // 增加最大流量比例
	
	// 动态模拟范围（BeAbleSee）
	const int start_i = max(2, (int)playerX - BeAbleSee);
	const int end_i = min(n-1, (int)playerX + BeAbleSee);
	const int start_j = max(2, (int)playerZ - BeAbleSee);
	const int end_j = min(m-1, (int)playerZ + BeAbleSee);
	
	memset(neww, 0, sizeof(neww));
	
	// 第一遍：计算动态水流
	for(int i = start_i; i <= end_i; ++i) {
		for(int j = start_j; j <= end_j; ++j) {
			const float currentWater = w[i][j];
			if(currentWater < 0.0001f){
				w[i][j]=0;
				continue;
			}
			
			// 当前总高度（地形+水位）
			const float totalHeight = a[i][j] + currentWater;
			
			// 边界消减（保持自然流失）
			if(i <= 2 || i >= n-1 || j <= 2 || j >= m-1) {
				neww[i][j] -= min(0.8f, currentWater * 0.4f);
				continue;
			}
			
			// 动态压力计算（考虑周围平均高度）
			float pressureSum = 0.0f;
			int validNeighbors = 0;
			for(int dir = 0; dir < 4; ++dir) {
				const int ni = i + dx[dir];
				const int nj = j + dy[dir];
				if(ni < 1 || ni > n || nj < 1 || nj > m) continue;
				pressureSum += a[ni][nj] + w[ni][nj];
				validNeighbors++;
			}
			const float pressure = totalHeight - (validNeighbors > 0 ? pressureSum/validNeighbors : 0);
			
			// 四方向流动处理
			for(int dir = 0; dir < 4; ++dir) {
				const int ni = i + dx[dir];
				const int nj = j + dy[dir];
				if(ni < 1 || ni > n || nj < 1 || nj > m) continue;
				
				// 惯性矢量合成
				Vector2 inertiaVec = {
					inertia[i][j][0] * viscosity,
					inertia[i][j][1] * viscosity
				};
				
				// 动态高度差计算
				const float neighborHeight = a[ni][nj] + w[ni][nj];
				const float heightDiff = totalHeight - neighborHeight;
				if(heightDiff < 0.01f) continue;
				
				// 基础流动量（压力驱动）
				float flowAmount = heightDiff * flowRate * 0.1f;
				
				// 增强惯性影响（方向一致性）
				const Vector2 dirVec = { (float)dx[dir], (float)dy[dir] };
				const float inertiaEffect = 1.0f + Vector2DotProduct(
					Vector2Normalize(inertiaVec), 
					Vector2Normalize(dirVec)
					);
				flowAmount *= inertiaEffect;
				
				// 流量动态限制
				flowAmount = min(flowAmount, currentWater * maxFlowRatio);
				flowAmount = Clamp(flowAmount, 0.01f, currentWater * 0.8f);
				
				// 更新流动量
				neww[i][j] -= flowAmount;
				neww[ni][nj] += flowAmount;
				
				// 更新惯性系统（动量守恒）
				inertia[ni][nj][0] += flowAmount * dx[dir] * inertiaFactor;
				inertia[ni][nj][1] += flowAmount * dy[dir] * inertiaFactor;
			}
			
			// 粘滞衰减（保持动量持续性）
			inertia[i][j][0] = Lerp(inertia[i][j][0], 0.0f, 0.1f);
			inertia[i][j][1] = Lerp(inertia[i][j][1], 0.0f, 0.1f);
		}
	}
	
	// 第二遍：应用变化并处理特殊地形
	for(int i = start_i; i <= end_i; ++i) {
		for(int j = start_j; j <= end_j; ++j) {
			// 应用水流变化
			w[i][j] = max(w[i][j] + neww[i][j], 0.0f);
			if(a[i][j] < 0.0f) {
				w[i][j]+=a[i][j];
			}
		}
	}
}

void InitFile(){
	freopen("dither.fs","w",stdout);
	cout<<R"(#version 330

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
	})";
fclose(stdout);
}
