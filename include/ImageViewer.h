#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include "MenuBar.h"
#include "FontManager.h"

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
    
    // 图片相关
    SDL_Texture* currentImage;
    std::string currentImagePath;
    int imageWidth, imageHeight;
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
    void OnFileOpened(const std::string& filename);
    void OnFolderOpened(const std::string& foldername);
    
    // 图片相关方法
    bool LoadImage(const std::string& imagePath);
    void ClearImage();
    void FitImageToWindow();
    void CenterImage();
    
    // 缩放相关方法
    void HandleWindowResize(int newWidth, int newHeight);
    void UpdateScaleFactor();
    float GetCurrentScaleFactor() const { return scaleFactor; }
    
    // 节能相关方法
    void MarkForRedraw() { needsRedraw = true; }
};
