#include <iostream>
#include <cstdint>

using namespace std;

bool isLittleEndian(){
  uint16_t value = 0x1;
  char* ptr = (char*)&value;
  return ptr[0] == 1;
}

int main(){

  #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  cout << "little endian detected at compile time\n";
  #elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  cout << "big endian detected at compile time\n";
  #endif

  if(isLittleEndian()){
    cout << "little endian detected at runtime\n";
  }
  else{
    cout << "big endian detected at runtime\n";
  }
  return 0;
}
