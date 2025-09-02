#include "FontManager.h"
#include <iostream>
#include <array>
#include <memory>

FontManager& FontManager::GetInstance() {
    static FontManager instance;
    return instance;
}

FontManager::~FontManager() {
    Cleanup();
}

bool FontManager::Initialize() {
    if (initialized) {
        return true;
    }
    
    // 初始化SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    
    // 获取系统字体路径
    fontPath = GetSystemFont();
    
    if (fontPath.empty()) {
        std::cerr << "No suitable font found!" << std::endl;
        TTF_Quit();
        return false;
    }
    
    // 加载不同大小的字体
    if (!LoadFonts()) {
        std::cerr << "Failed to load fonts!" << std::endl;
        Cleanup();
        return false;
    }
    
    initialized = true;
    std::cout << "FontManager initialized successfully with font: " << fontPath << std::endl;
    return true;
}

void FontManager::Cleanup() {
    for (int i = 0; i < 5; ++i) {
        if (fonts[i]) {
            TTF_CloseFont(fonts[i]);
            fonts[i] = nullptr;
        }
    }
    
    if (initialized) {
        TTF_Quit();
        initialized = false;
    }
}

std::string FontManager::GetSystemFont() {
    // 首先尝试使用fontconfig获取系统默认字体
    const char* fontconfigCmd = "fc-match -f '%{file}' sans-serif";
    
    std::array<char, 512> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(fontconfigCmd, "r"), pclose);
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        // 移除可能的换行符
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        
        // 验证文件是否存在
        if (!result.empty()) {
            std::ifstream file(result);
            if (file.good()) {
                std::cout << "System default font detected: " << result << std::endl;
                return result;
            }
        }
    }
    
    // 如果fontconfig失败，回退到常见字体路径
    const char* fallbackFonts[] = {
        // Ubuntu/Debian 系统字体
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        
        // Fedora/CentOS 系统字体
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
        
        // Arch Linux 系统字体
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/LiberationSans-Regular.ttf",
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        
        // openSUSE
        "/usr/share/fonts/truetype/DejaVuSans.ttf",
        
        // 其他可能的位置
        "/System/Library/Fonts/Arial.ttf", // macOS
        "/usr/share/fonts/corefonts/arial.ttf", // Windows字体包
        
        nullptr
    };
    
    for (int i = 0; fallbackFonts[i] != nullptr; ++i) {
        std::ifstream file(fallbackFonts[i]);
        if (file.good()) {
            std::cout << "Found fallback font: " << fallbackFonts[i] << std::endl;
            return std::string(fallbackFonts[i]);
        }
    }
    
    return "";
}

bool FontManager::LoadFonts() {
    int sizes[] = {SMALL, MEDIUM, LARGE, XLARGE, XXLARGE};
    
    for (int i = 0; i < 5; ++i) {
        fonts[i] = TTF_OpenFont(fontPath.c_str(), sizes[i]);
        if (!fonts[i]) {
            std::cerr << "Failed to load font size " << sizes[i] << ": " << TTF_GetError() << std::endl;
            return false;
        }
    }
    
    return true;
}

TTF_Font* FontManager::GetFont(FontSize size) {
    if (!initialized) {
        return nullptr;
    }
    
    int index = 0;
    switch (size) {
        case SMALL:   index = 0; break;
        case MEDIUM:  index = 1; break;
        case LARGE:   index = 2; break;
        case XLARGE:  index = 3; break;
        case XXLARGE: index = 4; break;
    }
    
    return fonts[index];
}

SDL_Surface* FontManager::RenderText(const std::string& text, SDL_Color color, FontSize size) {
    TTF_Font* font = GetFont(size);
    if (!font) {
        return nullptr;
    }
    
    return TTF_RenderText_Solid(font, text.c_str(), color);
}

SDL_Texture* FontManager::RenderTextTexture(SDL_Renderer* renderer, const std::string& text, SDL_Color color, FontSize size) {
    SDL_Surface* surface = RenderText(text, color, size);
    if (!surface) {
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    return texture;
}

void FontManager::GetTextSize(const std::string& text, int* width, int* height, FontSize size) {
    TTF_Font* font = GetFont(size);
    if (!font) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    
    TTF_SizeText(font, text.c_str(), width, height);
}

void FontManager::RenderTextAt(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color, FontSize size) {
    SDL_Texture* texture = RenderTextTexture(renderer, text, color, size);
    if (!texture) {
        return;
    }
    
    int textWidth, textHeight;
    SDL_QueryTexture(texture, nullptr, nullptr, &textWidth, &textHeight);
    
    SDL_Rect destRect = {x, y, textWidth, textHeight};
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);
    
    SDL_DestroyTexture(texture);
}

void FontManager::RenderTextCentered(SDL_Renderer* renderer, const std::string& text, SDL_Rect rect, SDL_Color color, FontSize size) {
    int textWidth, textHeight;
    GetTextSize(text, &textWidth, &textHeight, size);
    
    int x = rect.x + (rect.w - textWidth) / 2;
    int y = rect.y + (rect.h - textHeight) / 2;
    
    RenderTextAt(renderer, text, x, y, color, size);
}
