# Big Tips
按**ESC**退出。
尽量本地编译，```.exe```不保证最新 or 可运行。
# Ownderline
## Ownderline0.5（working）
- 优化**地形**生成。
- 优化**准心显示**。
- 更丰富的**细节**（working）
- 更**漂亮的水**（working）
- 更**大的地图**（working）
  -**无限**的地图（移到Ownderline0.6）
- 生成**植物**（working）
- 生成**QX**（作为添加实体的前瞻）（working）
## Ownderline0.4
- **帧率**是原来的**3.5倍**！
- 使用了着色器，把显示**像素化**了。
- 把渲染改为了**三角形网格**。
- 添加了**鼠标交互**。
- 优化水体显示（部分完成，剩下的移到Ownderline0.5）
## Ownderline0.3
- 添加**QX**。
- 可以**蹲下**、**趴下**了。
## Ownderline0.2
- 添加三种**视角**。
- 添加**截屏**
- 添加**地形编辑**操作
## Ownderline
继承于**EasyWorld2.0**。
- 添加**生存状态**。
# EasyWorld（停止更新）
## EasyWorld3.0 （不可运行！！！）
raylib仿MINECRAFT小游戏，需要[raylib](https://www.raylib.com/)库，或使用[小熊猫C++](http://royqh.net/redpandacpp/download/)。
## EasyWorld2.0 （BUG！！！）
raylib小游戏，需要[raylib](https://www.raylib.com/)库，或使用[小熊猫C++](http://royqh.net/redpandacpp/download/)。
用长方体的**高**和**颜色**来代表高度。```DrawCube(Vector3{i,a[i][j]*0.5,j},1,a[i][j],1,Color{ 100, (unsigned char)a[i][j], 100, 255 });```在 $(i,j)$ 位置画一个高 a_{ij} ，颜色越高越绿的长方体。
## EasyWorld1.0
控制台小游戏，使用WindowsAPI（```SetConsoleTextAttribute(hOut,s)```）显示高度。色彩单一，有较大局限性。
Windows**10** **可用**，Windows**7** 、Windows**11** **不**可用，不保证本地可运行。（放弃）
