// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_MERKLE_TREE_H_
#define VERIBLOCK_MERKLE_TREE_H_

#include <algorithm>
#include <deque>
#include <unordered_map>
#include <vector>
#include <veriblock/pop/arith_uint256.hpp>
#include <veriblock/pop/entities/atv.hpp>
#include <veriblock/pop/entities/merkle_path.hpp>

namespace altintegration {

//! @private
template <typename Specific, typename TxHash>
struct MerkleTree {
  using hash_t = TxHash;

  MerkleTree(Specific& instance, const std::vector<hash_t>& hashes)
      : instance(instance) {
    buildTree(hashes);
    for (int32_t i = 0, size = static_cast<int32_t>(hashes.size()); i < size;
         ++i) {
      hash_indices[hashes[i]] = i;
    }
  }

  std::vector<hash_t> getMerklePathLayers(const hash_t& hash) const {
    auto it = hash_indices.find(hash);
    VBK_ASSERT(it != hash_indices.end());
    size_t index = it->second;
    return getMerklePathLayers(index);
  }

  std::vector<hash_t> getMerklePathLayers(size_t index) const {
    VBK_ASSERT(!layers.empty());
    auto& leafs = layers[0];
    if (leafs.size() == 1) {
      VBK_ASSERT(layers.size() == 1);
      // no layers
      return {};
    }
    VBK_ASSERT(index < leafs.size());
    std::vector<hash_t> merklePath{};
    for (size_t i = 0, size = layers.size(); i < (size - 1); ++i) {
      auto& layer = layers[i];
      if (index % 2 == 0) {
        if (layer.size() == index + 1) {
          merklePath.push_back(layer[index]);
        } else {
          merklePath.push_back(layer[index + 1]);
        }
      } else {
        merklePath.push_back(layer[index - 1]);
      }

      index /= 2;
    }

    return merklePath;
  }

  hash_t getMerkleRoot() const {
    if (layers.empty()) {
      return hash_t{};
    }

    auto& root = layers.back();
    VBK_ASSERT(root.size() == 1);
    return root[0];
  }

  const std::vector<std::vector<hash_t>>& getLayers() const { return layers; }

  const std::unordered_map<hash_t, int32_t>& getHashIndices() const {
    return hash_indices;
  }

 protected:
  void buildTree(std::vector<hash_t> layer) {
    size_t n = layer.size();
    if (n == 0) {
      return;
    }
    if (n == 1) {
      layers = {layer};
      return;
    }

    layers.push_back(layer);

    layer.resize(n % 2 == 0 ? n : n + 1);
    while (n > 1) {
      if ((n % 2) != 0u) {
        layer[n] = layer[n - 1];
        ++n;
      }
      n /= 2;
      for (size_t i = 0; i < n; i++) {
        layer[i] = instance.hash(layer[2 * i], layer[2 * i + 1]);
      }
      layers.push_back({layer.begin(), layer.begin() + n});
    }
  }

  Specific& instance;
  std::vector<std::vector<hash_t>> layers;
  std::unordered_map<hash_t, int32_t> hash_indices;
};

//! @private
struct VbkMerkleTree {
  using hash_t = typename MerkleTree<VbkMerkleTree, uint256>::hash_t;

  enum class TreeIndex : int32_t {
    POP = 0,
    NORMAL = 1,
  };

  explicit VbkMerkleTree(const std::vector<hash_t>& normal_hashes,
                         const std::vector<hash_t>& pop_hashes)
      : pop_tree(*this, pop_hashes), normal_tree(*this, normal_hashes) {}

  hash_t hash(const hash_t& a, const hash_t& b) const { return sha256(a, b); }

  std::vector<hash_t> finalizePath(std::vector<hash_t> path,
                                   const TreeIndex treeIndex) const {
    switch (treeIndex) {
      case TreeIndex::POP: {
        // opposite tree merkle root
        if (path.empty() && normal_tree.getLayers().empty()) {
          return path;
        }

        path.emplace_back(normal_tree.getMerkleRoot());
        break;
      }
      case TreeIndex::NORMAL: {
        // opposite tree merkle root
        if (path.empty() && pop_tree.getLayers().empty()) {
          return path;
        }

        path.emplace_back(pop_tree.getMerkleRoot());
        break;
      }
    }

    // metapackage hash
    path.emplace_back();
    return path;
  }

  hash_t getMerkleRoot() const {
    if (pop_tree.getLayers().empty() && normal_tree.getLayers().empty()) {
      return hash_t{};
    }

    if ((pop_tree.getLayers().size() == 1) && normal_tree.getLayers().empty()) {
      // the only layer
      VBK_ASSERT(pop_tree.getLayers()[0].size() == 1);
      return pop_tree.getMerkleRoot();
    }

    if ((normal_tree.getLayers().size() == 1) && pop_tree.getLayers().empty()) {
      // the only layer
      VBK_ASSERT(normal_tree.getLayers()[0].size() == 1);
      return normal_tree.getMerkleRoot();
    }

    auto pop_hash = pop_tree.getMerkleRoot();
    auto normal_hash = normal_tree.getMerkleRoot();

    // add metapackage hash (also all zeroes) to the left subtree
    auto metapackageHash = hash_t();
    return hash(metapackageHash, hash(pop_hash, normal_hash));
  }

  std::vector<hash_t> getMerklePathLayers(const size_t index,
                                          const TreeIndex treeIndex) const {
    switch (treeIndex) {
      case TreeIndex::POP:
        return finalizePath(pop_tree.getMerklePathLayers(index), treeIndex);
      case TreeIndex::NORMAL:
        return finalizePath(normal_tree.getMerklePathLayers(index), treeIndex);
      default:
        return {};
    }
  }

  VbkMerklePath getMerklePath(const hash_t& hash,
                              const TreeIndex treeIndex) const {
    VbkMerklePath merklePath;
    merklePath.subject = hash;
    merklePath.treeIndex = static_cast<int32_t>(treeIndex);
    switch (treeIndex) {
      case TreeIndex::POP: {
        const auto it = pop_tree.getHashIndices().find(hash);
        VBK_ASSERT(it != pop_tree.getHashIndices().end());
        const int32_t index = it->second;

        merklePath.index = index;
        merklePath.layers = finalizePath(
            pop_tree.getMerklePathLayers(static_cast<size_t>(index)),
            treeIndex);
        break;
      }
      case TreeIndex::NORMAL: {
        const auto it = normal_tree.getHashIndices().find(hash);
        VBK_ASSERT(it != normal_tree.getHashIndices().end());
        const int32_t index = it->second;

        merklePath.index = index;
        merklePath.layers = finalizePath(
            normal_tree.getMerklePathLayers(static_cast<size_t>(index)),
            treeIndex);
        break;
      }
      default:
        return merklePath;
    }

    return merklePath;
  }

 private:
  MerkleTree<VbkMerkleTree, uint256> pop_tree;
  MerkleTree<VbkMerkleTree, uint256> normal_tree;
};

// Calculates an approximate amount of PopTxs in the POP merkle subtree
inline uint32_t estimateNumberOfPopTxs(
    const std::vector<VbkMerklePath>& paths) {
  // validate that we have a continugous indexes set
  std::set<int32_t> indexes;
  int32_t maxIndex = -1;
  for (const auto& path : paths) {
    if (path.treeIndex == (int32_t)VbkMerkleTree::TreeIndex::POP &&
        maxIndex < path.index) {
      maxIndex = path.index;
    }
  }

  // find the latest index and determine amount of the leaves
  uint32_t aproximate_leaves_number = 0;
  for (const auto& path : paths) {
    if (path.treeIndex == (int32_t)VbkMerkleTree::TreeIndex::POP &&
        maxIndex == path.index) {
      if (path.layers.empty()) {
        aproximate_leaves_number = 1;
        break;
      }
      auto vec = path.equalLayerIndexes();
      if (vec.empty()) {
        aproximate_leaves_number = (1 << (path.layers.size() - 2));
        break;
      } else if (vec.front() == 0) {
        aproximate_leaves_number = path.index + 1;
        break;
      } else {
        aproximate_leaves_number = (1 << (path.layers.size() - 2));
        for (const auto& i : vec) {
          aproximate_leaves_number -= (1 << i);
        }
        break;
      }
    }
  }

  return aproximate_leaves_number;
}

inline bool isPopSubTreeFull(const std::vector<VbkMerklePath>& paths) {
  return (uint32_t)paths.size() == estimateNumberOfPopTxs(paths);
}

//! @private
struct BtcMerkleTree : public MerkleTree<BtcMerkleTree, uint256> {
  using base = MerkleTree<BtcMerkleTree, uint256>;

  explicit BtcMerkleTree(const std::vector<hash_t>& txes) : base(*this, txes) {}

  hash_t hash(const hash_t& a, const hash_t& b) const {
    return sha256twice(a, b);
  }

  MerklePath getMerklePath(const hash_t& hash) const {
    auto it = hash_indices.find(hash);
    VBK_ASSERT(it != hash_indices.end());
    size_t index = it->second;

    MerklePath merklePath;
    merklePath.index = (int32_t)index;
    merklePath.subject = hash;
    merklePath.layers = getMerklePathLayers(index);
    return merklePath;
  }

  hash_t getMerkleRoot() { return base::getMerkleRoot().reverse(); }
};

//! @private
template <typename pop_t>
struct PayloadsMerkleTree
    : public MerkleTree<PayloadsMerkleTree<pop_t>, typename pop_t::id_t> {
  using base = MerkleTree<PayloadsMerkleTree<pop_t>, typename pop_t::id_t>;
  using hash_t = typename base::hash_t;

  explicit PayloadsMerkleTree(const std::vector<hash_t>& hashes)
      : base(*this, hashes) {}

  hash_t hash(const hash_t& a, const hash_t& b) const {
    return sha256twice(a, b).template trimLE<hash_t::size()>();
  }

  hash_t getMerkleRoot() { return base::getMerkleRoot().reverse(); }
};

}  // namespace altintegration
#endif  // !VERIBLOCK_MERKLE_TREE_H_
