#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include "VulkansEye.hpp"

const int WIDTH = 1024;
const int HEIGHT = 768;

int main() {
  CoInitializeEx(NULL, COINIT_MULTITHREADED);

  tat::VulkansEye app;
  try {
    app.init(WIDTH, HEIGHT);
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  CoUninitialize();
  return EXIT_SUCCESS;
}