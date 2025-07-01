#include <iostream>
#include <cstdint>
#include <bit>

using namespace std;

uint32_t swapEndian(uint32_t val){
  return std::byteswap(val);
}

int main(){
  uint32_t valueBefore = 0xAB12CD34;
  cout << "Value before swapping: " << std::hex << std::uppercase << valueBefore << std::endl << "\n";
  uint32_t valueAfter = swapEndian(valueBefore);
  cout << "Value after swapping: " << valueAfter << "\n";
}
