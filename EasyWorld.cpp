#include <bits/stdc++.h>
#include <windows.h>
#include <conio.h>
#define itn int
#define sc(s) SetConsoleTextAttribute(hOut,s)
using namespace std;
int maxx=0;
int mx,mn;
int n=25, m=25;
double a[1145][1145];
double w[1145][1145];
double l[1145][1145];
double scale=0.114514;
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
            
            // 将噪声值转换到0~99并存入数组
            a[i][j] = (total + 1.0) / 2.0 * 255.0;
            mx = max(mx, (int)a[i][j]);
            mn = min(mn, (int)a[i][j]);
        }
    }
}
/*** 隐藏光标 ***/
void HideCursor() {
CONSOLE_CURSOR_INFO cursor_info = {1, 0}; // 设置光标信息，大小为1，隐藏光标
SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info); // 设置控制台光标信息
}
void out() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hConsole, pos);
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
        	if (i == playerX && j == playerY){
        		sc(16*4);printf("  ");
			}
            else if (w[i][j] < 0.1){//land
				//sc(103); 
				/*
				sc(14*16+6);
				printf("%c%c", getHeightSymbol_land((a[i][j] + w[i][j])),getHeightSymbol_land((a[i][j]+ w[i][j])));
				*/
				int height=a[i][j]/2.55,xp=100/6;
    			if(height>=xp*5) sc(16*15);
    			else if(height>=xp*4) sc(16*7);
    			else if(height>=xp*3) sc(16*2);
    			else if(height>=xp*2) sc(16*10);
    			else if(height>=xp*1) sc(16*6);
    			else if(height>=xp*0) sc(16*14);
    			//cout<<a[i][j]<<a[i][j];
				printf("  ");
			}
            else{//water
            	int height=w[i][j]*2,xp=100/4;
    			if(height>=xp*3) sc(16);
    			else if(height>=xp*2) sc(16*9);
    			else if(height>=xp*1) sc(16*3);
    			else if(height>=xp*0) sc(16*11);
				printf("  ");
			}
            //printf("  ");
            //cout<<getHeightSymbol((a[i][j] + w[i][j]))<<getHeightSymbol((a[i][j] + w[i][j]));
        }
        sc(7);printf("\n");
    }
    //sc(7);printf("这是一个模拟水流的程序(by:upb)");
	sc(1*16);cout<<" ";sc(9*16);cout<<" ";sc(3*16);cout<<" ";sc(11*16);cout<<" ";sc(14*16);cout<<" ";sc(6*16);cout<<" ";sc(10*16);cout<<" ";sc(2*16);cout<<" ";sc(7*16);cout<<" ";sc(15*16);cout<<" ";sc(7);
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
void getMouseClick(int &x, int &y) {
    POINT pt;
    GetCursorPos(&pt);
    HWND hwnd = GetConsoleWindow();
    ScreenToClient(hwnd, &pt);

    // 将屏幕坐标转换为控制台窗口坐标
    CONSOLE_FONT_INFO fontInfo;
    GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &fontInfo);
    x = pt.y / fontInfo.dwFontSize.Y + 1;
    y = pt.x / fontInfo.dwFontSize.X / 2 + 1;
}
void Playerdo() {
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
        	int x, y;
            getMouseClick(x, y);
            if (x >= 1 && x <= n && y >= 1 && y <= m) {
                w[x][y] += 5.0; // 在点击位置放水
            }
        }
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
        	int x, y;
            getMouseClick(x, y);
            if (x >= 1 && x <= n && y >= 1 && y <= m) {
                w[x][y] += 5.0; // 在点击位置放水
            }
        }
		if (GetAsyncKeyState('W') & 0x8000) {
        	int newX = playerX;
    		int newY = playerY;
    		newX--;
    		if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
        		playerX = newX;
        		playerY = newY;
    		}
        }
        if (GetAsyncKeyState('S') & 0x8000) {
        	int newX = playerX;
    		int newY = playerY;
    		newX++;
    		if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
        		playerX = newX;
        		playerY = newY;
    		}
        }
        if (GetAsyncKeyState('A') & 0x8000) {
        	int newX = playerX;
    		int newY = playerY;
    		newY--;
    		if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
        		playerX = newX;
        		playerY = newY;
    		}
        }
        if (GetAsyncKeyState('D') & 0x8000) {
        	int newX = playerX;
    		int newY = playerY;
    		newY++;
    		if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
        		playerX = newX;
        		playerY = newY;
    		}
        }
}

int main() {
	DWORD prev_mode;
HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
GetConsoleMode(hStdin, &prev_mode);
SetConsoleMode(hStdin, ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_ECHO_INPUT));
	system("mode con cols=51 lines=26");
	HideCursor();
	build_PerlinNoise();
	/*
	cin>>n>>m;
	for(int i=1;i<=n;i++){
    	for(int j=1;j<=m;j++){
    		cin>>a[i][j];
		}
	}
	*/
	for(int i=1;i<=n;i++){
    	for(int j=1;j<=m;j++){
    		w[i][j]=max(maxx-a[i][j],0.0);
		}
	}
	system("cls");
	while(1){
		//Sleep(250);
		
		if(_kbhit()){
        char ch = _getch();
        if(ch == 't'){
        	string sss;
            cin>>sss;
            if(sss=="fa"){
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
    }
		for(int i=1;i<=n;i++){
    		for(int j=1;j<=m;j++){
    			l[i][j]=a[i][j]+w[i][j];
			}
		}
		Playerdo();
		flow();
		out();
		bool flag=1;
		for(int i=1;i<=n;i++){
    		for(int j=1;j<=m;j++){
    			if(a[i][j]+w[i][j]!=l[i][j]) flag=0;
			}
		}
		//if(flag==1) break;
	}
	int sum=0;
	for(int i=1;i<=n;i++){
    	for(int j=1;j<=m;j++){
    		sum+=w[i][j];
		}
	}
	cout<<sum;
    return 0;
}
