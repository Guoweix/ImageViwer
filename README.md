# Image Viewer

这是一个使用SDL2库开发的图片查看器项目。

## 依赖项

- CMake 3.16+
- SDL2
- SDL2_image
- SDL2_ttf (用于字体渲染)
- GTK3 (用于文件对话框)
- fontconfig (用于系统字体检测)
- C++17 编译器

## 在Ubuntu/Debian上安装依赖

```bash
sudo apt update
sudo apt install cmake build-essential
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
sudo apt install libgtk-3-dev libfontconfig1-dev
```

## 编译和运行

1. 创建构建目录：
```bash
mkdir build
cd build
```

2. 生成Makefile：
```bash
cmake ..
```

3. 编译：
```bash
make
```

4. 运行：
```bash
./bin/image_viewer
```

## 项目结构

```
ImageViewer/
├── CMakeLists.txt          # CMake构建配置
├── README.md              # 项目说明
├── build.sh              # 构建脚本
├── include/               # 头文件目录
│   ├── ImageViewer.h     # 主类头文件
│   └── MenuBar.h         # 菜单栏类头文件
└── src/                  # 源文件目录
    ├── main.cpp          # 主程序入口
    ├── ImageViewer.cpp   # 主类实现
    └── MenuBar.cpp       # 菜单栏类实现
```

## 功能

当前版本包含：
- 带有"File"菜单的菜单栏，使用系统字体渲染
- 下拉菜单包含三个选项：
  - Open File：打开图片文件
  - Open Folder：打开文件夹
  - Open Archive：打开压缩包（暂未实现）
- GTK原生文件对话框支持
- 系统字体自动检测和回退机制
- 完整的鼠标交互和悬停效果
- 调试信息输出

### 控制键

- ESC键：退出程序
- 点击窗口关闭按钮：退出程序
- 点击"File"菜单：显示/隐藏下拉菜单
- 下拉菜单选项：
  - "Open File"：打开文件选择对话框
  - "Open Folder"：打开文件夹选择对话框
  - "Open Archive"：显示调试信息（功能待实现）

## 计划功能

- [ ] 图片加载和显示
- [ ] 支持常见图片格式 (PNG, JPG, BMP, etc.)
- [ ] 图片缩放
- [ ] 图片拖拽
- [ ] 文件夹浏览
- [ ] 键盘快捷键支持
