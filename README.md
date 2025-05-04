# Big Tips
尽量本地编译，```.exe```不保证最新 or 可运行。
# Ownderline
## Ownderline0.4
- 使用了着色器，把显示像素化了。
- 把渲染改为了三角形网格
- 优化了水体显示
- 添加了鼠标交互
## Ownderline0.3
记不到了。。。
## Ownderline0.2
记不到了。。。
## Ownderline0.1
记不到了。。。
# EasyWorld（停止更新）
## EasyWorld3.0 （不可运行！！！）
### Tips
按ESC退出。

raylib仿MINECRAFT小游戏，需要[raylib](https://www.raylib.com/)库，或使用[小熊猫C++](http://royqh.net/redpandacpp/download/)。
## EasyWorld2.0 （BUG！！！）
### Tips
按ESC退出。

raylib小游戏，需要[raylib](https://www.raylib.com/)库，或使用[小熊猫C++](http://royqh.net/redpandacpp/download/)。

用长方体的**高**和**颜色**来代表高度。```DrawCube(Vector3{i,a[i][j]*0.5,j},1,a[i][j],1,Color{ 100, (unsigned char)a[i][j], 100, 255 });```在 $(i,j)$ 位置画一个高 a_{ij} ，颜色越高越绿的长方体。
## EasyWorld1.0
### Tips
控制台小游戏，使用WindowsAPI（```SetConsoleTextAttribute(hOut,s)```）显示高度。色彩单一，有较大局限性。

Windows10 可用，Windows7 无法使用鼠标，Windows11 色彩错误。（不保证本地可运行）
