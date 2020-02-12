#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

template <typename InStream, typename OutStream>
void write(InStream& in, OutStream& out, const std::string& var) {
  out << "namespace generated {" << std::endl;
  out << "extern const char " << var << "[] = {" << std::endl;
  std::string line;
  while (std::getline(in, line)) {
    for (char c : line) {
      out << static_cast<int>(c) << ", ";
    }
    out << static_cast<int>('\n') << ", " << std::endl;
  }
  out << 0 << std::endl;     // null terminator
  out << "};" << std::endl;  // end of array
  out << "};" << std::endl;  // end of namespace
}

// NOLINTNEXTLINE
int main(int argc, char** argv) {
  if (argc != 4) {
    std::cout << R"(  Usage:
   stringtobytes variable_name infile outfile
)";
    return 1;
  }

  std::string var = argv[1]; // NOLINT
  std::string in = argv[2];  // NOLINT
  std::string out = argv[3]; // NOLINT
  std::fstream fout(out, std::fstream::out);
  std::fstream fin(in, std::fstream::in);

  write(fin, fout, var);

  return 0;
}