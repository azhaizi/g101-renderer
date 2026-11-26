# Games101 汇总

## 1. 总体概览
### 1.1 项目说明
该项目为 games101 作业的个人汇总，其中：
- Homework 文件夹里为个人原始作业
- 其余为个人根据作业构建出来的一个渲染器仓库

### 1.2目录结构

## 2. 作业说明
docs 文件夹中包含对于 games101 8 个作业的说明。 

说明分为两部分，第一部分是作业的解法，第二部分是关于作业怎么成为该渲染器仓库的一份子的说明。 

|作业   |文档   |作业   |文档|
|---|---|---|---|
|1  |[作业1](/docs/1.md)|2  |[作业2](/docs/2.md)|
|3  |[作业3](/docs/3.md)|4  |[作业4](/docs/4.md)|
|5  |[作业5](/docs/5.md)|6  |[作业6](/docs/6.md)|
|7  |[作业7](/docs/7.md)|8  |[作业8](/docs/8.md)|

## 3.构建说明
- Homework 采用作业文档中说明的构建方法，在对应作业的目录下运行即可
- 下面介绍渲染器的构建与运行方法

### 3.1 构建命令
```powershell
cd C:\Users\15956\Desktop\Gams101
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### 3.2 执行命令参数说明
```powershell
.\build\Release\renderer.exe --mode <模式> [参数...]
```

| 参数 | 取值 | 作用 |
|---|---|---|
| `--mode` | `raster` / `raytrace` / `pathtrace` / `bezier` / `rope` | 选哪个渲染器 |
| `--scene` | `cornell` / `bunny` | 场景（raytrace/pathtrace 用） |
| `--out` | 路径 | 输出 PNG；`rope` 模式是**文件名前缀** |
| `--width` `--height` | 整数 | 覆盖默认分辨率 |
| `--spp` | 整数 | 每像素采样数（pathtrace 默认 64） |
| `--depth` | 整数 | 递归深度上限（默认 5） |
| `--gamma` | 浮点 | 伽马校正（pathtrace 默认 0.6） |
| `--bvh` | `naive` / `sah` | BVH 划分方式 |
| `--integrator` | `euler` / `verlet` | 绳索积分器 |
| `--angle` | 角度 | 光栅化模型旋转角 |
| `--frames` | 整数 | 绳索帧数 |
| `--dt` `--substeps` | 浮点/整数 | 绳索每帧时长 / 子步数 |
| `--asset-dir` | 路径 | 资源根目录 |

查看帮助：`.\build\Release\renderer.exe --help`

### 3.3 每个作业的执行命令
> 所有命令都在**仓库根目录**执行（用 `.\build\Release\renderer.exe` 调用，不依赖当前目录）。

#### 作业 1：变换（MVP 矩阵）

```powershell
.\build\Release\renderer.exe --mode raster --out results\raster\a1_transform.png
```

#### 作业 2：三角形光栅化 + 抗锯齿

```powershell
.\build\Release\renderer.exe --mode raster --out results\raster\a2_raster.png
.\build\Release\renderer.exe --mode raster --angle 45 --out results\raster\a2_raster_rot45.png
```

#### 作业 3：纹理与 Blinn-Phong 着色

```powershell
.\build\Release\renderer.exe --mode raster --out results\raster\a3_textured.png
```
> 当前状态：`rasterizer` 已支持 `set_texture()` / `set_fragment_shader()`，但 app 侧还没接 OBJ 与贴图，所以这条命令走的还是顶点色回退路径。
> 
#### 作业 4：贝塞尔曲线

```powershell
.\build\Release\renderer.exe --mode bezier --out results\geometry\a4_bezier.png
```

#### 作业 5：光线追踪（Whitted）

```powershell
.\build\Release\renderer.exe --mode raytrace --scene cornell --spp 4 --width 400 --height 400 --out results\raytrace\a5_whitted.png
```
> 当前状态：走的是 spp=1~4 的路径追踪，独立 Whitted 积分器尚未实现。

#### 作业 6：BVH 加速

```powershell
.\build\Release\renderer.exe --mode pathtrace --scene bunny --bvh naive --spp 8 --width 200 --height 200 --out results\bench\a6_bvh_naive.png
.\build\Release\renderer.exe --mode pathtrace --scene bunny --bvh sah   --spp 8 --width 200 --height 200 --out results\bench\a6_bvh_sah.png
```

#### 作业 7：路径追踪

```powershell
.\build\Release\renderer.exe --mode pathtrace --scene cornell --spp 16  --width 300 --height 300 --out results\pathtrace\a7_pt_16spp.png
.\build\Release\renderer.exe --mode pathtrace --scene cornell --spp 128 --width 300 --height 300 --out results\pathtrace\a7_pt_128spp.png
.\build\Release\renderer.exe --mode pathtrace --scene cornell --depth 1 --spp 8 --width 200 --height 200 --out results\bench\a7_depth1.png
.\build\Release\renderer.exe --mode pathtrace --scene cornell --depth 5 --spp 8 --width 200 --height 200 --out results\bench\a7_depth5.png
```

#### 作业 8：绳索模拟

```powershell
.\build\Release\renderer.exe --mode rope --integrator euler  --frames 60 --out results\physics\a8_rope_euler
.\build\Release\renderer.exe --mode rope --integrator verlet --frames 60 --out results\physics\a8_rope_verlet
ffmpeg -y -framerate 30 -i "results\physics\a8_rope_euler_%04d.png"  results\physics\a8_rope_euler.gif
ffmpeg -y -framerate 30 -i "results\physics\a8_rope_verlet_%04d.png" results\physics\a8_rope_verlet.gif
```

### 3.4 一键运行脚本
```powershell
powershell -ExecutionPolicy Bypass -File "C:\Users\15956\Desktop\deepseek harness\GAME101\render-all.ps1"
```