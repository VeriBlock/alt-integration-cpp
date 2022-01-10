// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef ALT_INTEGRATION_VISUALIZE_HPP
#define ALT_INTEGRATION_VISUALIZE_HPP

#include <fstream>

/// Contains utils to create DOT files

namespace altintegration {

struct DummyEndorsement;

//   std::ofstream f("file.txt");
//   WriteBlockTree(f, blockTree);
//   f.close();
//
// find file.txt and execute:
//   $ dot -Tsvg file.txt > file.svg
//
// merge multiple block trees:
//   $ cat file.txt | gvpack -u | dot -Tsvg > file.svg
//
// open svg file in browser or any viewer
//
template <typename Stream, typename BlockTree>
void WriteBlockTree(Stream& s,
                    const BlockTree& tree,
                    std::string name = "BLOCKCHAIN") {
  auto toNodeName = [](auto& blockIndex) {
    std::ostringstream ss;
    ss << "\"[";
    ss << blockIndex.height;
    ss << "] ";
    ss << blockIndex.getHash().toHex().substr(0, 8) << "\"";
  };

  s << "digraph " << name << " {\n";

  const auto& map = tree.getValidBlocks();
  auto& best = tree.getBestChain();
  auto* current = best.tip();
  while (current) {
    s << toNodeName(*current);
    s << R"( [fontcolor="green"];)";
    current = current->pprev;
  }

  for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
    auto& index = it->second;
    auto prev = index->pprev;
    if (prev) {
      s << toNodeName(*prev);
      s << "->";
    }

    s << toNodeName(*index);
    s << ";\n";

    if constexpr (!std::is_same<DummyEndorsement,
                                typename BlockTree::index_t::endorsement_t>{}) {
      if (!index->containingEndorsements.empty()) {
        for (const auto& kv : index->containingEndorsements) {
          auto& e = kv.second;
          if (!e) {
            continue;
          }

          auto* containing = tree.getBlockIndex(e->containingHash);
          auto* endorsed = tree.getBlockIndex(e->endorsedHash);

          s << toNodeName(*containing);
          s << "->";
          s << toNodeName(*endorsed);
          s << " [constraint=false color=\"green\"];\n";
        }
      }
    }
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

}  // namespace altintegration

#endif  // ALT_INTEGRATION_VISUALIZE_HPP
