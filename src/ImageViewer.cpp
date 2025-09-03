#include "ImageViewer.h"
#include <iostream>
#include <algorithm>
#include <filesystem>

ImageViewer::ImageViewer() 
    : window(nullptr), renderer(nullptr), isRunning(false), isFullscreen(false), hasOpenedFile(false), needsRedraw(true),
      imageScale(1.0f), imageOffsetX(0), imageOffsetY(0),
      scaleFactor(1.0f), windowWidth(800), windowHeight(600), lastWindowWidth(800), lastWindowHeight(600),
      currentImageIndex(-1) {
}

ImageViewer::~ImageViewer() {
    Cleanup();
}

bool ImageViewer::Initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    lastWindowWidth = width;
    lastWindowHeight = height;
    
    // 设置缩放插值方式为最近邻，避免图片缩放模糊
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // 初始化SDL_image
    int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
        return false;
    }
    
    // 创建窗口
    window = SDL_CreateWindow("Image Viewer - Welcome",
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             windowWidth, windowHeight,
                             SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // 创建渲染器 - 启用VSync以节能
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // 设置渲染器颜色
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    
 
    // 初始化字体管理器
    FontManager& fontManager = FontManager::GetInstance();
    if (!fontManager.Initialize()) {
        std::cerr << "Failed to initialize font manager" << std::endl;
        return false;
    }
    
    // 初始化菜单栏
    if (!menuBar.Initialize(renderer)) {
        std::cerr << "Failed to initialize menu bar" << std::endl;
        return false;
    }
    
    // 设置菜单栏回调
    menuBar.SetOnFileOpened([this](const std::string& filepath) {
        OnFileOpened(filepath);
    });

    menuBar.SetOnFolderOpened([this](const std::string& folderpath) {
        OnFolderOpened(folderpath);
    });
    
    // 初始化缩放
    UpdateScaleFactor();
    menuBar.SetScaleFactor(scaleFactor);
    menuBar.UpdateLayout(windowWidth, windowHeight);
    
    isRunning = true;
    std::cout << "SDL initialized successfully!" << std::endl;
    return true;
}

void ImageViewer::Run() {
    while (isRunning) {
        HandleEvents();
        
        // 只在需要时重绘
        if (needsRedraw) {
            Render();
            needsRedraw = false;
        }
        
        // 使用更长的延迟以节能，或者等待事件
        SDL_WaitEventTimeout(nullptr, 16); // 最多等待16ms，如果有事件则立即返回
    }
}

void ImageViewer::HandleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        // 先让菜单栏处理事件
        menuBar.HandleEvent(e);
        switch (e.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        if (isFullscreen) {
                            ToggleFullscreen();
                        } else {
                            isRunning = false;
                        }
                        break;
                    case SDLK_F11:
                        ToggleFullscreen();
                        break;
                    case SDLK_m:
                        if (e.key.keysym.mod & KMOD_CTRL) {
                            MinimizeWindow();
                        }
                        break;
                    case SDLK_q:
                        if (e.key.keysym.mod & KMOD_CTRL) {
                            isRunning = false;
                        }
                        break;
                    case SDLK_LEFT:
                        // 上一张
                        if (images.size() > 1 && currentImageIndex > 0) {
                            --currentImageIndex;
                            FitImageToWindow();
                            CenterImage();
                            MarkForRedraw();
                        }
                        break;
                    case SDLK_RIGHT:
                        // 下一张
                        if (images.size() > 1 && currentImageIndex < (int)images.size() - 1) {
                            ++currentImageIndex;
                            FitImageToWindow();
                            CenterImage();
                            MarkForRedraw();
                        }
                        break;
                }
                MarkForRedraw(); // 键盘事件后标记重绘
                break;
                
            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
                    isRunning = false;
                } else if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    HandleWindowResize(e.window.data1, e.window.data2);
                    MarkForRedraw(); // 窗口大小改变后标记重绘
                } else if (e.window.event == SDL_WINDOWEVENT_EXPOSED) {
                    MarkForRedraw(); // 窗口暴露事件后标记重绘
                }
                break;
                
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEMOTION:
                MarkForRedraw(); // 鼠标事件后标记重绘
                break;
        }
    }
}

void ImageViewer::Render() {
    // 清除屏幕
    if (hasOpenedFile) {
        SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xFF); // 深灰色背景
    } else {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // 白色背景
    }
    SDL_RenderClear(renderer);
    
    if (hasOpenedFile && currentImageIndex >= 0 && currentImageIndex < (int)images.size() && images[currentImageIndex].texture) {
        // 渲染图片
        RenderImage();
    } else if (hasOpenedFile) {
        // 如果文件打开但图片加载失败，显示错误信息
        int menuHeight = menuBar.GetHeight();
        int centerX = windowWidth / 2;
        int centerY = (windowHeight + menuHeight) / 2;
        
        FontManager& fontManager = FontManager::GetInstance();
        SDL_Color errorColor = {255, 100, 100, 255}; // 红色
        SDL_Rect errorRect = {centerX - 150, centerY - 10, 300, 20};
        fontManager.RenderTextCentered(renderer, "Failed to load image", errorRect, errorColor, FontManager::FontSize::MEDIUM);
    } else {
        // 显示欢迎界面
        RenderWelcomeScreen();
    }
    // 最后渲染菜单栏，确保它在最上层
    menuBar.Render(renderer);
    // 更新屏幕
    SDL_RenderPresent(renderer);
}

void ImageViewer::Cleanup() {
    ClearImage();
    
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    IMG_Quit();
    SDL_Quit();
    std::cout << "SDL cleaned up." << std::endl;
}

void ImageViewer::OnFileOpened(const std::string& filename) {
    ClearAllImages();
    std::cout << "File opened: " << filename << std::endl;
    if (LoadImage(filename)) {
        hasOpenedFile = true;
        currentImageIndex = 0;
        // 获取文件名（不包含路径）
        size_t pos = filename.find_last_of("/\\");
        std::string displayName = (pos != std::string::npos) ? filename.substr(pos + 1) : filename;
        SDL_SetWindowTitle(window, ("Image Viewer - " + displayName).c_str());
        FitImageToWindow();
        CenterImage();
        MarkForRedraw(); // 加载新图片后标记重绘
    } else {
        hasOpenedFile = true; // 仍然设置为true以显示错误信息
        MarkForRedraw(); // 显示错误信息也需要重绘
        std::cerr << "Failed to load image: " << filename << std::endl;
    }
}

void ImageViewer::OnFolderOpened(const std::string& folderpath) {
    ClearAllImages(); // 先清除之前的图片
    std::cout << "Folder opened: " << folderpath << std::endl;
    hasOpenedFile = true;
    SDL_SetWindowTitle(window, ("Image Viewer - " + folderpath).c_str());
    // TODO: 在这里添加浏览文件夹中图片的逻辑

    //将图片添加到images
    MarkForRedraw(); // 打开文件夹后标记重绘
    //遍历folderpath下的图片文件，加载到images中
    for (const auto& entry : std::filesystem::directory_iterator(folderpath)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".png"|| entry.path().extension() == ".jpg" || entry.path().extension() == ".jpeg" || entry.path().extension() == ".bmp" || entry.path().extension() == ".tif" || entry.path().extension() == ".tiff")) {
            LoadImage(entry.path().string());
        }
    }

    if (!images.empty()) {
        currentImageIndex = 0;
        FitImageToWindow();
        CenterImage();
    } else {
        currentImageIndex = -1; // 没有图片
    }

}

void ImageViewer::RenderWelcomeScreen() {
    int menuHeight = menuBar.GetHeight();
    
    // 计算居中位置
    int centerX = windowWidth / 2;
    int centerY = (windowHeight + menuHeight) / 2;
    
    FontManager& fontManager = FontManager::GetInstance();
    
    // 根据缩放因子调整文字区域大小
    int titleWidth = static_cast<int>(300 * scaleFactor);
    int titleHeight = static_cast<int>(40 * scaleFactor);
    int subtitleWidth = static_cast<int>(200 * scaleFactor);
    int subtitleHeight = static_cast<int>(20 * scaleFactor);
    int hintWidth = static_cast<int>(350 * scaleFactor);
    int hintHeight = static_cast<int>(15 * scaleFactor);
    
    // 渲染主标题
    SDL_Color titleColor = {255, 255, 255, 255}; // 白色
    SDL_Rect titleRect = {centerX - titleWidth/2, centerY - 60, titleWidth, titleHeight};
    fontManager.RenderTextCentered(renderer, "Image Viewer", titleRect, titleColor, FontManager::FontSize::XLARGE);
    
    // 渲染副标题
    SDL_Color subtitleColor = {200, 200, 200, 255}; // 浅灰色
    SDL_Rect subtitleRect = {centerX - subtitleWidth/2, centerY - 10, subtitleWidth, subtitleHeight};
    fontManager.RenderTextCentered(renderer, "Open File to Begin", subtitleRect, subtitleColor, FontManager::FontSize::MEDIUM);
    
    // 渲染提示信息
    SDL_Color hintColor = {160, 160, 160, 255}; // 更浅的灰色
    SDL_Rect hintRect = {centerX - hintWidth/2, centerY + 30, hintWidth, hintHeight};
    fontManager.RenderTextCentered(renderer, "Use File menu or press F11 for fullscreen", hintRect, hintColor, FontManager::FontSize::SMALL);
    
    // 渲染缩放信息
    SDL_Color scaleColor = {100, 100, 100, 255}; // 深灰色
    SDL_Rect scaleRect = {10, windowHeight - 30, 200, 20};
    std::string scaleText = "Scale: " + std::to_string(static_cast<int>(scaleFactor * 100)) + "%";
    fontManager.RenderTextAt(renderer, scaleText, scaleRect.x, scaleRect.y, scaleColor, FontManager::FontSize::SMALL);
    
  
    
   
    // 快捷键提示的小矩形
    const char* shortcuts[] = {"F11: Fullscreen", "Ctrl+M: Minimize", "Ctrl+Q: Quit", "ESC: Exit"};
    int shortcutWidth = (windowWidth - 80) / 4;
    
}

void ImageViewer::ToggleFullscreen() {
    isFullscreen = !isFullscreen;
    
    if (isFullscreen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        std::cout << "Switched to fullscreen mode" << std::endl;
    } else {
        SDL_SetWindowFullscreen(window, 0);
        std::cout << "Switched to windowed mode" << std::endl;
    }
    
    MarkForRedraw(); // 全屏切换后标记重绘
}

void ImageViewer::MinimizeWindow() {
    SDL_MinimizeWindow(window);
    std::cout << "Window minimized" << std::endl;
}

void ImageViewer::HandleWindowResize(int newWidth, int newHeight) {
    windowWidth = newWidth;
    windowHeight = newHeight;
    
    std::cout << "Window resized to: " << newWidth << "x" << newHeight << std::endl;
    
    // 更新缩放因子
    UpdateScaleFactor();
    
    // 更新菜单栏缩放
    menuBar.SetScaleFactor(scaleFactor);
    menuBar.UpdateLayout(windowWidth, windowHeight);
    
    // 如果有图片，重新调整其位置和大小
    if (currentImageIndex >= 0 && currentImageIndex < (int)images.size()) {
        FitImageToWindow();
        CenterImage();
    }
}

void ImageViewer::UpdateScaleFactor() {
    // 基于窗口大小计算缩放因子
    // 使用800x600作为基准分辨率
    float widthScale = static_cast<float>(windowWidth) / 800.0f;
    float heightScale = static_cast<float>(windowHeight) / 600.0f;
    
    // 取较小的缩放比例以保持图片比例
    scaleFactor = std::min(widthScale, heightScale);
    
    // 限制缩放范围
    scaleFactor = std::max(0.5f, std::min(scaleFactor, 3.0f));
    
    std::cout << "Scale factor updated to: " << scaleFactor << std::endl;
}

bool ImageViewer::LoadImage(const std::string& imagePath) {
    // 加载图片
    SDL_Surface* loadedSurface = IMG_Load(imagePath.c_str());
    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image " << imagePath << "! IMG_Error: " << IMG_GetError() << std::endl;
        return false;
    }
    // 创建纹理
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (tex == nullptr) {
        std::cerr << "Unable to create texture from " << imagePath << "! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(loadedSurface);
        return false;
    }
    // 获取图片尺寸
    int w = loadedSurface->w;
    int h = loadedSurface->h;
    // 释放表面
    SDL_FreeSurface(loadedSurface);
    // 存入images
    images.push_back({tex, w, h, imagePath});
    std::cout << "Image loaded successfully: " << w << "x" << h << std::endl;
    return true;
}

void ImageViewer::ClearImage() {
    if (currentImageIndex >= 0 && currentImageIndex < (int)images.size()) {
        if (images[currentImageIndex].texture) {
            SDL_DestroyTexture(images[currentImageIndex].texture);
            images[currentImageIndex].texture = nullptr;
        }
        images.erase(images.begin() + currentImageIndex);
        if (images.empty()) {
            currentImageIndex = -1;
        } else if (currentImageIndex >= (int)images.size()) {
            currentImageIndex = (int)images.size() - 1;
        }
    }
    imageScale = 1.0f;
    imageOffsetX = 0;
    imageOffsetY = 0;
}

void ImageViewer::ClearAllImages() {
    for (auto& img : images) {
        if (img.texture) SDL_DestroyTexture(img.texture);
    }
    images.clear();
    currentImageIndex = -1;
    imageScale = 1.0f;
    imageOffsetX = 0;
    imageOffsetY = 0;
}

void ImageViewer::FitImageToWindow() {
    if (currentImageIndex < 0 || currentImageIndex >= (int)images.size() || !images[currentImageIndex].texture) return;
    int menuHeight = menuBar.GetHeight();
    int availableWidth = windowWidth;
    int availableHeight = windowHeight - menuHeight;
    int imageWidth = images[currentImageIndex].width;
    int imageHeight = images[currentImageIndex].height;
    // 计算缩放比例以适应窗口
    float scaleX = static_cast<float>(availableWidth) / static_cast<float>(imageWidth);
    float scaleY = static_cast<float>(availableHeight) / static_cast<float>(imageHeight);
    // 选择较小的缩放比例以保持圖片比例
    imageScale = std::min(scaleX, scaleY);
    // 限制最小缩放比例
    imageScale = std::max(0.1f, imageScale);
    std::cout << "Image scale set to: " << imageScale << std::endl;
}

void ImageViewer::CenterImage() {
    if (currentImageIndex < 0 || currentImageIndex >= (int)images.size() || !images[currentImageIndex].texture) return;
    int menuHeight = menuBar.GetHeight();
    int scaledWidth = static_cast<int>(images[currentImageIndex].width * imageScale);
    int scaledHeight = static_cast<int>(images[currentImageIndex].height * imageScale);
    // 计算居中位置
    imageOffsetX = (windowWidth - scaledWidth) / 2;
    imageOffsetY = menuHeight + (windowHeight - menuHeight - scaledHeight) / 2;
    std::cout << "Image centered at: " << imageOffsetX << ", " << imageOffsetY << std::endl;
}

void ImageViewer::RenderImage() {
    if (currentImageIndex < 0 || currentImageIndex >= (int)images.size() || !images[currentImageIndex].texture) return;
    int scaledWidth = static_cast<int>(images[currentImageIndex].width * imageScale);
    int scaledHeight = static_cast<int>(images[currentImageIndex].height * imageScale);
    SDL_Rect destRect = {
        imageOffsetX,
        imageOffsetY,
        scaledWidth,
        scaledHeight
    };
    SDL_RenderCopy(renderer, images[currentImageIndex].texture, nullptr, &destRect);
}
