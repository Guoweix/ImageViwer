#ifndef SIMPLE_FILE_DIALOG_H
#define SIMPLE_FILE_DIALOG_H

#include <string>
#include <future>

class SimpleFileDialog {
public:
    static std::string OpenFile();
    static std::string OpenFolder();
    static std::string OpenArchive();
    static std::future<std::string> OpenFileAsync();
    static std::future<std::string> OpenFolderAsync();
    static std::future<std::string> OpenArchiveAsync();
};

#endif // SIMPLE_FILE_DIALOG_H
