#ifndef ALT_INTEGRATION_VISUALIZE_HPP
#define ALT_INTEGRATION_VISUALIZE_HPP

#include <fstream>
#include <veriblock/fmt.hpp>

/// Contains utils to create DOT files

namespace AltIntegrationLib {

//   std::ofstream f("file.txt");
//   WriteBlockTree(f, blockTree);
//   f.close();
//
// find file.txt and execute:
//   $ dot -Tsvg file.txt > file.svg
//
// open svg file in browser or any viewer
//
template <typename Stream, typename BlockTree>
void WriteBlockTree(Stream& s,
                    const BlockTree& tree,
                    std::string name = "BLOCKCHAIN") {
  s << format("digraph %s {\n", name);

  const auto& map = tree.getAllBlocks();
  for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
    auto& index = it->second;
    auto prev = index->pprev;

    auto thisHash = index->getHash().toHex();
    auto to = thisHash.substr(0, 8);

    if (prev) {
      auto prevHash = prev->getHash().toHex();
      auto from = prevHash.substr(0, 8);

      s << format("\"[%d] %s\"", prev->height, from);
      s << "->";
    }

    s << format("\"[%d] %s\"", index->height, to);
    s << ";\n";
  }

  s << "}" << std::endl;
}

template <typename BlockTree>
void WriteBlockTreeToFile(const std::string& file,
                          const BlockTree& tree,
                          std::string name = "BLOCKCHAIN") {
  std::ofstream f(file.c_str());
  WriteBlockTree(f, tree, name);
  f.close();
}

}  // namespace AltIntegrationLib

#endif  // ALT_INTEGRATION_VISUALIZE_HPP
