#ifndef SIMPLE_FILE_DIALOG_H
#define SIMPLE_FILE_DIALOG_H

#include <string>
#include <future>

class SimpleFileDialog {
public:
    static std::string OpenFile();
    static std::string OpenFolder();
    static std::future<std::string> OpenFileAsync();
    static std::future<std::string> OpenFolderAsync();
};

#endif // SIMPLE_FILE_DIALOG_H
