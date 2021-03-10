
#include <vector>
#include <random>
#include<algorithm>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size);

int main() {
  std::vector<uint8_t> v{0x85,0x3f,0x3d};
//
//  srand(0);
//  std::generate_n(std::back_inserter(v), 10000, [](){
//    return rand() & 0xff;
//  });

  return LLVMFuzzerTestOneInput(v.data(), v.size());
}