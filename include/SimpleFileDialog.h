#ifndef SIMPLE_FILE_DIALOG_H
#define SIMPLE_FILE_DIALOG_H

#include <string>

class SimpleFileDialog {
public:
    static std::string OpenFile();
    static std::string OpenFolder();
};

#endif // SIMPLE_FILE_DIALOG_H
