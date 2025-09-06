#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <vector>
#include "MenuBar.h"
#include "FontManager.h"

struct ImageData {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    std::string path;
};

class ImageViewer {
public:
    ImageViewer();
    ~ImageViewer();
    
    bool Initialize(int width = 800, int height = 600);
    void Run();
    void Cleanup();
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool isRunning;
    bool isFullscreen;
    bool hasOpenedFile;
    bool needsRedraw;  // 添加重绘标志
    MenuBar menuBar;

    // 多图相关
    std::vector<ImageData> images;
    int currentImageIndex = -1;
    float imageScale;
    int imageOffsetX, imageOffsetY;

    // 缩放相关
    float scaleFactor;
    int windowWidth, windowHeight;
    int lastWindowWidth, lastWindowHeight;

    void HandleEvents();
    void Render();
    void RenderWelcomeScreen();
    void RenderImage();
    void ToggleFullscreen();
    void MinimizeWindow();
    void OnFileOpened(const std::string& filepath);
    void OnFolderOpened(const std::string& folderpath);
    void OnArchiveOpened(const std::string& archivename); // 新增：处理打开归档文件

    // 图片相关方法
    bool LoadImage(const std::string& imagePath); // 可扩展为批量加载
    void ClearImage();
    void FitImageToWindow();
    void CenterImage();
    void ClearAllImages(); // 新增：释放所有图片

    // 缩放相关方法
    void HandleWindowResize(int newWidth, int newHeight);
    void UpdateScaleFactor();
    float GetCurrentScaleFactor() const { return scaleFactor; }

    // 节能相关方法
    void MarkForRedraw() { needsRedraw = true; }
};
