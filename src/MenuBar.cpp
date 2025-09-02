#include "MenuBar.h"
#include "SimpleFileDialog.h"
#include <iostream>
#include <cstdlib>
#include <memory>
#include <array>

MenuBar::MenuBar() 
    : showDropdown(false), fontManager(nullptr), scaleFactor(1.0f), currentWindowWidth(800) {
    
    // 获取FontManager实例
    fontManager = &FontManager::GetInstance();
    
    // 设置颜色
    textColor = {50, 50, 50, 255};           // 深灰色文字
    textHoverColor = {0, 0, 0, 255};         // 黑色悬停文字
    dropdownBgColor = {240, 240, 240, 255};  // 浅灰色下拉背景
    dropdownBorderColor = {180, 180, 180, 255}; // 灰色边框
    
    // 初始化缩放参数
    UpdateScaledSizes();
    
    // 初始化File菜单项
    fileMenu.rect = {5, 0, menuItemWidth, menuHeight};
    fileMenu.text = "File";
    fileMenu.isHovered = false;
    fileMenu.isPressed = false;
    
    // 初始化下拉菜单项
    dropdownItems.resize(3);
    
    dropdownItems[0].text = "Open File";
    dropdownItems[0].callback = [this]() { OnOpenFile(); };
    
    dropdownItems[1].text = "Open Folder";
    dropdownItems[1].callback = [this]() { OnOpenFolder(); };
    
    dropdownItems[2].text = "Open Archive";
    dropdownItems[2].callback = [this]() { OnOpenArchive(); };
    
    // 设置下拉菜单项的位置
    for (size_t i = 0; i < dropdownItems.size(); ++i) {
        dropdownItems[i].rect = {
            fileMenu.rect.x,
            fileMenu.rect.y + menuHeight + (int)i * dropdownItemHeight,
            dropdownWidth,
            dropdownItemHeight
        };
        dropdownItems[i].isHovered = false;
    }
}

MenuBar::~MenuBar() {
    // FontManager是单例，不需要在这里清理
}

bool MenuBar::Initialize(SDL_Renderer* renderer) {
    // FontManager会自动初始化字体，这里不需要特殊处理
    return true;
}

void MenuBar::HandleEvent(const SDL_Event& event) {
    int mouseX, mouseY;
    
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                mouseX = event.button.x;
                mouseY = event.button.y;
                
                // 检查是否点击了File菜单
                if (IsPointInRect(mouseX, mouseY, fileMenu.rect)) {
                    fileMenu.isPressed = true;
                    showDropdown = !showDropdown;  // 切换下拉菜单显示状态
                }
                // 检查是否点击了下拉菜单项
                else if (showDropdown) {
                    for (auto& item : dropdownItems) {
                        if (IsPointInRect(mouseX, mouseY, item.rect)) {
                            item.callback();  // 调用回调函数
                            showDropdown = false;  // 关闭下拉菜单
                            break;
                        }
                    }
                    // 如果点击了下拉菜单外的区域，关闭下拉菜单
                    if (showDropdown) {
                        bool clickedOutside = true;
                        for (const auto& item : dropdownItems) {
                            if (IsPointInRect(mouseX, mouseY, item.rect)) {
                                clickedOutside = false;
                                break;
                            }
                        }
                        if (clickedOutside && !IsPointInRect(mouseX, mouseY, fileMenu.rect)) {
                            showDropdown = false;
                        }
                    }
                }
                else {
                    // 点击了其他地方，关闭下拉菜单
                    showDropdown = false;
                }
            }
            break;
            
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                fileMenu.isPressed = false;
            }
            break;
            
        case SDL_MOUSEMOTION:
            mouseX = event.motion.x;
            mouseY = event.motion.y;
            
            // 更新File菜单悬停状态
            fileMenu.isHovered = IsPointInRect(mouseX, mouseY, fileMenu.rect);
            
            // 更新下拉菜单项悬停状态
            if (showDropdown) {
                for (auto& item : dropdownItems) {
                    item.isHovered = IsPointInRect(mouseX, mouseY, item.rect);
                }
            }
            break;
    }
}

void MenuBar::Render(SDL_Renderer* renderer) {
    // 获取当前渲染器输出尺寸
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    
    // 绘制菜单栏背景（白色背景）
    SDL_SetRenderDrawColor(renderer, 0xF8, 0xF8, 0xF8, 0xFF); // 浅灰白色
    SDL_Rect menuBarRect = {0, 0, windowWidth, menuHeight};
    SDL_RenderFillRect(renderer, &menuBarRect);
    
    // 绘制File菜单项
    DrawMenuItem(renderer, fileMenu);
    
    // 绘制下拉菜单
    if (showDropdown) {
        DrawDropdown(renderer);
    }
    
    // 绘制底部分隔线（深一点的灰色）
    SDL_SetRenderDrawColor(renderer, 0xD0, 0xD0, 0xD0, 0xFF);
    SDL_RenderDrawLine(renderer, 0, menuHeight - 1, windowWidth, menuHeight - 1);
}

void MenuBar::DrawMenuItem(SDL_Renderer* renderer, const MenuItem& item) {
    // 设置背景颜色
    if (item.isPressed) {
        SDL_SetRenderDrawColor(renderer, 0xE0, 0xE0, 0xE0, 0xFF); // 浅灰色按下
    } else if (item.isHovered) {
        SDL_SetRenderDrawColor(renderer, 0xF0, 0xF0, 0xF0, 0xFF); // 很浅的灰色悬停
    } else {
        SDL_SetRenderDrawColor(renderer, 0xF8, 0xF8, 0xF8, 0xFF); // 白色默认背景
    }
    
    SDL_RenderFillRect(renderer, &item.rect);
    
    // 绘制边框（如果需要）
    if (item.isHovered || item.isPressed) {
        SDL_SetRenderDrawColor(renderer, 0xC0, 0xC0, 0xC0, 0xFF);
        SDL_RenderDrawRect(renderer, &item.rect);
    }
    
    // 绘制文字
    SDL_Color color = item.isHovered ? textHoverColor : textColor;
    
    if (fontManager) {
        // 计算文字居中位置
        int textW, textH;
        fontManager->GetTextSize(item.text, &textW, &textH, FontManager::FontSize::SMALL);
        int textX = item.rect.x + (item.rect.w - textW) / 2;
        int textY = item.rect.y + (item.rect.h - textH) / 2;
        
        DrawText(renderer, item.text, textX, textY, color);
    } else {
        // 如果没有字体，绘制简单的矩形占位符
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_Rect textRect = {
            item.rect.x + 10,
            item.rect.y + 10,
            item.rect.w - 20,
            item.rect.h - 20
        };
        SDL_RenderDrawRect(renderer, &textRect);
    }
}

void MenuBar::DrawDropdown(SDL_Renderer* renderer) {
    // 绘制下拉菜单背景
    SDL_Rect dropdownBg = {
        fileMenu.rect.x,
        fileMenu.rect.y + menuHeight,
        dropdownWidth,
        (int)dropdownItems.size() * dropdownItemHeight
    };
    
    SDL_SetRenderDrawColor(renderer, dropdownBgColor.r, dropdownBgColor.g, dropdownBgColor.b, dropdownBgColor.a);
    SDL_RenderFillRect(renderer, &dropdownBg);
    
    // 绘制下拉菜单边框
    SDL_SetRenderDrawColor(renderer, dropdownBorderColor.r, dropdownBorderColor.g, dropdownBorderColor.b, dropdownBorderColor.a);
    SDL_RenderDrawRect(renderer, &dropdownBg);
    
    // 绘制每个下拉菜单项
    for (const auto& item : dropdownItems) {
        // 设置背景颜色
        if (item.isHovered) {
            SDL_SetRenderDrawColor(renderer, 0xE8, 0xE8, 0xE8, 0xFF); // 浅灰色悬停
            SDL_RenderFillRect(renderer, &item.rect);
        }
        
        // 绘制分隔线
        if (&item != &dropdownItems[0]) { // 不在第一项上绘制分隔线
            SDL_SetRenderDrawColor(renderer, dropdownBorderColor.r, dropdownBorderColor.g, dropdownBorderColor.b, dropdownBorderColor.a);
            SDL_RenderDrawLine(renderer, item.rect.x, item.rect.y, item.rect.x + item.rect.w, item.rect.y);
        }
        
        // 绘制文字
        SDL_Color color = item.isHovered ? textHoverColor : textColor;
        
        if (fontManager) {
            int textW, textH;
            fontManager->GetTextSize(item.text, &textW, &textH, FontManager::FontSize::SMALL);
            int textX = item.rect.x + 10; // 左对齐，稍微缩进
            int textY = item.rect.y + (item.rect.h - textH) / 2;
            
            DrawText(renderer, item.text, textX, textY, color);
        }
    }
}

void MenuBar::DrawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    fontManager->RenderTextAt(renderer, text, x, y, color, FontManager::FontSize::SMALL);
}

bool MenuBar::IsPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x < rect.x + rect.w && 
           y >= rect.y && y < rect.y + rect.h;
}

void MenuBar::OnOpenFile() {
    std::cout << "[DEBUG] Open File clicked" << std::endl;
    std::string filename = OpenFileDialog();
    if (!filename.empty()) {
        std::cout << "[DEBUG] File selected: " << filename << std::endl;
        if (onFileOpened) {
            onFileOpened(filename);
        }
    } else {
        std::cout << "[DEBUG] No file selected" << std::endl;
    }
}

void MenuBar::OnOpenFolder() {
    std::cout << "[DEBUG] Open Folder clicked" << std::endl;
    std::string foldername = OpenFolderDialog();
    if (!foldername.empty()) {
        std::cout << "[DEBUG] Folder selected: " << foldername << std::endl;
        if (onFolderOpened) {
            onFolderOpened(foldername);
        }
    } else {
        std::cout << "[DEBUG] No folder selected" << std::endl;
    }
}

void MenuBar::OnOpenArchive() {
    std::cout << "[DEBUG] Open Archive clicked" << std::endl;
    std::cout << "[DEBUG] Archive functionality not yet implemented" << std::endl;
}

std::string MenuBar::OpenFileDialog() {
    return SimpleFileDialog::OpenFile();
}

std::string MenuBar::OpenFolderDialog() {
    return SimpleFileDialog::OpenFolder();
}

void MenuBar::SetOnFileOpened(std::function<void(const std::string&)> callback) {
    onFileOpened = callback;
}

void MenuBar::SetOnFolderOpened(std::function<void(const std::string&)> callback) {
    onFolderOpened = callback;
}

void MenuBar::SetScaleFactor(float scale) {
    scaleFactor = scale;
    UpdateScaledSizes();
    
    std::cout << "MenuBar scale factor set to: " << scaleFactor << std::endl;
}

void MenuBar::UpdateLayout(int windowWidth, int windowHeight) {
    currentWindowWidth = windowWidth;
    
    // 更新菜单项位置和大小
    fileMenu.rect = {5, 0, menuItemWidth, menuHeight};
    
    // 更新下拉菜单项的位置
    for (size_t i = 0; i < dropdownItems.size(); ++i) {
        dropdownItems[i].rect = {
            fileMenu.rect.x,
            fileMenu.rect.y + menuHeight + static_cast<int>(i) * dropdownItemHeight,
            dropdownWidth,
            dropdownItemHeight
        };
    }
    
    std::cout << "MenuBar layout updated for window size: " << windowWidth << "x" << windowHeight << std::endl;
}

void MenuBar::UpdateScaledSizes() {
    menuHeight = static_cast<int>(baseMenuHeight * scaleFactor);
    menuItemWidth = static_cast<int>(baseMenuItemWidth * scaleFactor);
    dropdownItemHeight = static_cast<int>(baseDropdownItemHeight * scaleFactor);
    dropdownWidth = static_cast<int>(baseDropdownWidth * scaleFactor);
}
