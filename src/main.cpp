#include "ImageViewer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Image Viewer starting..." << std::endl;
    
    ImageViewer viewer;
    
    if (!viewer.Initialize()) {
        std::cerr << "Failed to initialize Image Viewer" << std::endl;
        return -1;
    }
    
    viewer.Run();
    viewer.Cleanup();
    
    std::cout << "Image Viewer closed." << std::endl;
    return 0;
}
