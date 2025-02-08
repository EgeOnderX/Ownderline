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
// ????
int playerX = n / 2; // ?????X??
int playerY = m / 2; // ?????Y??
// ????????
class PerlinNoise {
private:
    int permutation[512];
    
public:
    PerlinNoise(unsigned int seed) {
        // ??????
        iota(permutation, permutation + 256, 0);
        shuffle(permutation, permutation + 256, default_random_engine(seed));
        // ??????????
        for (int i = 0; i < 256; ++i)
            permutation[256 + i] = permutation[i];
    }
    
    // ????????
    double noise(double x, double y) {
        // ??????
        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        
        // ???????
        x -= floor(x);
        y -= floor(y);
        
        // ??????
        double u = fade(x);
        double v = fade(y);
        
        // ?????????
        int A = permutation[X] + Y;
        int AA = permutation[A & 255];
        int AB = permutation[(A + 1) & 255];
        int B = permutation[X + 1] + Y;
        int BA = permutation[B & 255];
        int BB = permutation[(B + 1) & 255];
        
        // ??????
        double gradAA = grad(AA, x, y);
        double gradBA = grad(BA, x - 1, y);
        double gradAB = grad(AB, x, y - 1);
        double gradBB = grad(BB, x - 1, y - 1);
        
        // ?????
        double lerp1 = lerp(u, gradAA, gradBA);
        double lerp2 = lerp(u, gradAB, gradBB);
        return lerp(v, lerp1, lerp2);
    }
    
private:
    // ????:6t^5 - 15t^4 + 10t^3
    static double fade(double t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }
    
    // ????
    static double lerp(double t, double a, double b) {
        return a + t * (b - a);
    }
    
    // ?????(??????????)
    static double grad(int hash, double x, double y) {
        int h = hash & 3;
        switch (h) {
            case 0: return x + y;    // ???
            case 1: return -x + y;   // ???
            case 2: return x - y;    // ???
            case 3: return -x - y;   // ???
            default: return 0; // ????
        }
    }
};


void build_PerlinNoise(){
	PerlinNoise pn(time(0)); // ??????????
	 // ????????
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            // ?????????????
            double x = i * scale;
            double y = j * scale;
            
            // ??????(??????)
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
            total /= maxAmplitude; // ???
            
            // ???????0~99?????
            a[i][j] = (total + 1.0) / 2.0 * 255.0;
            mx = max(mx, (int)a[i][j]);
            mn = min(mn, (int)a[i][j]);
        }
    }
}
/*** ???? ***/
void HideCursor() {
CONSOLE_CURSOR_INFO cursor_info = {1, 0}; // ??????,???1,????
SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info); // ?????????
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
    //sc(7);printf("???????????(by:upb)");
	sc(1*16);cout<<" ";sc(9*16);cout<<" ";sc(3*16);cout<<" ";sc(11*16);cout<<" ";sc(14*16);cout<<" ";sc(6*16);cout<<" ";sc(10*16);cout<<" ";sc(2*16);cout<<" ";sc(7*16);cout<<" ";sc(15*16);cout<<" ";sc(7);
}
/*
void flow(){
	for(int i=1;i<=n;i++){
    	for(int j=1;j<=m;j++){
    		if(w[i][j]==0) continue;
    		if(w[i][j]>=1&&(i==1||i==n||j==1||j==m)) w[i][j]--;
    		int dx[] = {-1, 0, 1, 0};
            int dy[] = {0, -1, 0, 1};
            for (int k = 0; k < 4; k++) {
                int ni=i+dx[k];
                int nj=j+dy[k];
                
                if (ni>=1&&ni<=n&&nj>=1&&nj<=m){
                    if(w[i][j]>0.1&&l[i][j]>l[ni][nj]){
                        w[i][j]--;
                        w[ni][nj]++;
                	}
                	
            	}
            	
			}
		}
	}
}
*/

void flow(){
    // ???????????
    double neww[114][114] = {0};
    
    for(int i=1;i<=n;i++){
        for(int j=1;j<=m;j++){
            if(w[i][j] < 0.1) continue;
            
            // ????
            if((i==1||i==n||j==1||j==m) && w[i][j]>=1){
                neww[i][j] -= 0.5;
            }
            
            // ?????
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
    
    // ????
    for(int i=1;i<=n;i++){
        for(int j=1;j<=m;j++){
            w[i][j] += neww[i][j];
            if(w[i][j] < 0) w[i][j] = 0;
        }
    }
}
void movePlayer(char direction) {
    int newX = playerX;
    int newY = playerY;
    
    if (direction == 'w') newX--;
    if (direction == 's') newX++;
    if (direction == 'a') newY--;
    if (direction == 'd') newY++;

    // ??????
    if(newX >= 1 && newX <= n && newY >= 1 && newY <= m){
        playerX = newX;
        playerY = newY;
    }
}
int main() {
	DWORD prev_mode;
HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
GetConsoleMode(hStdin, &prev_mode);
SetConsoleMode(hStdin, ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_ECHO_INPUT));
	system("mode con cols=50 lines=26");
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
	
	w[15][15]=9800;
	system("cls");
	while(1){
		//Sleep(250);
		 if(_kbhit()){
        char ch = _getch();
        if(ch == 'w' || ch == 'a' || ch == 's' || ch == 'd'){
            movePlayer(ch);
        }
        else if(ch == 27) break; // ESC??
    }
		for(int i=1;i<=n;i++){
    		for(int j=1;j<=m;j++){
    			l[i][j]=a[i][j]+w[i][j];
			}
		}
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
