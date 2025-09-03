#pragma once

#include <SDL2/SDL.h>
// #include <gtk/gtk.h>
#include <string>
#include <functional>
#include <vector>
#include <future>
#include "FontManager.h"

struct MenuItem {
    SDL_Rect rect;
    std::string text;
    bool isHovered;
    bool isPressed;
};

struct DropdownItem {
    std::string text;
    SDL_Rect rect;
    bool isHovered;
    std::function<void()> callback;
};

class MenuBar {
public:
    MenuBar();
    ~MenuBar();
    
    // 初始化菜单栏
    bool Initialize(SDL_Renderer* renderer);
    
    // 处理事件
    void HandleEvent(const SDL_Event& event);
    
    // 渲染菜单栏
    void Render(SDL_Renderer* renderer);
    
    // 设置回调函数
    void SetOnFileOpened(std::function<void(const std::string&)> callback);
    void SetOnFolderOpened(std::function<void(const std::string&)> callback);
    
    // 获取菜单栏高度（支持缩放）
    int GetHeight() const { return static_cast<int>(baseMenuHeight * scaleFactor); }
    
    // 缩放相关方法
    void SetScaleFactor(float scale);
    void UpdateLayout(int windowWidth, int windowHeight);
    
private:
    static const int baseMenuHeight = 35;
    static const int baseMenuItemWidth = 60;
    static const int baseDropdownItemHeight = 25;
    static const int baseDropdownWidth = 150;
    
    // 当前缩放参数
    float scaleFactor;
    int menuHeight;
    int menuItemWidth;
    int dropdownItemHeight;
    int dropdownWidth;
    int currentWindowWidth;
    
    MenuItem fileMenu;
    bool showDropdown;
    
    std::vector<DropdownItem> dropdownItems;
    
    // 字体管理器
    FontManager* fontManager;
    SDL_Color textColor;
    SDL_Color textHoverColor;
    SDL_Color dropdownBgColor;
    SDL_Color dropdownBorderColor;
    
    // 回调函数
    std::function<void(const std::string&)> onFileOpened;
    std::function<void(const std::string&)> onFolderOpened;
    
    // 辅助函数
    void DrawMenuItem(SDL_Renderer* renderer, const MenuItem& item);
    void DrawDropdown(SDL_Renderer* renderer);
    void DrawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);
    bool IsPointInRect(int x, int y, const SDL_Rect& rect);
    void UpdateScaledSizes();
    
    // 下拉菜单项回调
    void OnOpenFile();
    void OnOpenFolder();
    void OnOpenArchive();
    
    // 系统文件对话框
    std::string OpenFileDialog();
    std::string OpenFolderDialog();

    // 异步文件对话框
    std::future<std::string> openFileFuture;
    bool openFilePending = false;
};
