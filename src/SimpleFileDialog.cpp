#include "SimpleFileDialog.h"
#include <iostream>
#include <memory>
#include <array>
#include <cstdlib>
#include <future>

std::string SimpleFileDialog::OpenFile() {
    std::string result;
    
    // 尝试使用zenity (GNOME)
    std::string command = "zenity --file-selection --title='Open Image File' "
                         "--file-filter='Image Files | *.jpg *.jpeg *.png *.bmp *.gif *.tiff' "
                         "--file-filter='All Files | *' 2>/dev/null";
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    
    if (pipe) {
        std::array<char, 512> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        // 移除换行符
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
    }
    
    // 如果zenity失败，尝试kdialog (KDE)
    if (result.empty()) {
        command = "kdialog --getopenfilename ~ '*.jpg *.jpeg *.png *.bmp *.gif *.tiff|Image Files' 2>/dev/null";
        pipe.reset(popen(command.c_str(), "r"));
        
        if (pipe) {
            std::array<char, 512> buffer;
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            
            if (!result.empty() && result.back() == '\n') {
                result.pop_back();
            }
        }
    }
    
    // 如果都失败，回退到命令行输入
    if (result.empty()) {
        std::cout << "请输入文件路径: ";
        std::getline(std::cin, result);
    }
    
    return result;
}

std::string SimpleFileDialog::OpenFolder() {
    std::string result;
    
    // 尝试使用zenity (GNOME)
    std::string command = "zenity --file-selection --directory --title='Open Folder' 2>/dev/null";
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    
    if (pipe) {
        std::array<char, 512> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        // 移除换行符
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
    }
    
    // 如果zenity失败，尝试kdialog (KDE)
    if (result.empty()) {
        command = "kdialog --getexistingdirectory ~ 2>/dev/null";
        pipe.reset(popen(command.c_str(), "r"));
        
        if (pipe) {
            std::array<char, 512> buffer;
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            
            if (!result.empty() && result.back() == '\n') {
                result.pop_back();
            }
        }
    }
    
    // 如果都失败，回退到命令行输入
    if (result.empty()) {
        std::cout << "请输入文件夹路径: ";
        std::getline(std::cin, result);
    }
    
    return result;
}

std::future<std::string> SimpleFileDialog::OpenFileAsync() {
    return std::async(std::launch::async, []() {
        return OpenFile();
    });
}

std::future<std::string> SimpleFileDialog::OpenFolderAsync() {
    return std::async(std::launch::async, []() {
        return OpenFolder();
    });
}
