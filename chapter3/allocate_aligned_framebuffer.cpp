#include <cstdlib>
#include <memory>

constexpr size_t WIDTH = 1920;
constexpr size_t HEIGHT = 1080;
constexpr size_t PIXEL_SIZE = 4;

int main(){
  size_t buffer_size = WIDTH * HEIGHT * PIXEL_SIZE;
  void* framebuffer = nullptr;

  // Allocating 32-byte aligned memory
  int res = posix_memalign(&framebuffer, 32, buffer_size);
  if(res != 0){
    // handle allocation failure
    return -1;
  }

  // Use framebuffer

  free(framebuffer);
  return 0;
}
