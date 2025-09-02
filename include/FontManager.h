#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <memory>
#include <vector>
#include <fstream>

class FontManager {
public:
    // 字体大小枚举
    enum FontSize {
        SMALL = 12,
        MEDIUM = 14,
        LARGE = 18,
        XLARGE = 24,
        XXLARGE = 32
    };
    
    // 单例模式
    static FontManager& GetInstance();
    
    // 初始化字体管理器
    bool Initialize();
    
    // 清理资源
    void Cleanup();
    
    // 获取指定大小的字体
    TTF_Font* GetFont(FontSize size = MEDIUM);
    
    // 渲染文字到表面
    SDL_Surface* RenderText(const std::string& text, SDL_Color color, FontSize size = MEDIUM);
    
    // 渲染文字到纹理
    SDL_Texture* RenderTextTexture(SDL_Renderer* renderer, const std::string& text, SDL_Color color, FontSize size = MEDIUM);
    
    // 计算文字尺寸
    void GetTextSize(const std::string& text, int* width, int* height, FontSize size = MEDIUM);
    
    // 渲染文字到指定位置
    void RenderTextAt(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, FontSize size = MEDIUM);
    
    // 渲染居中文字
    void RenderTextCentered(SDL_Renderer* renderer, const std::string& text, SDL_Rect rect, SDL_Color color, FontSize size = MEDIUM);
    
    // 检查是否成功加载字体
    bool IsInitialized() const { return initialized; }
    
private:
    FontManager() = default;
    ~FontManager();
    
    // 禁用拷贝构造和赋值
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    
    // 尝试加载系统字体
    std::string GetSystemFont();
    
    // 加载字体
    bool LoadFonts();
    
    // 字体路径和对象
    std::string fontPath;
    TTF_Font* fonts[5] = {nullptr}; // 对应5种字体大小
    bool initialized = false;
};
