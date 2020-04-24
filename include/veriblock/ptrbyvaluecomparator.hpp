#ifndef ALTINTEGRATION_PTRCOMPARATOR_HPP
#define ALTINTEGRATION_PTRCOMPARATOR_HPP

namespace altintegration {

struct PtrByValueComparator {
  template <typename T>
  bool operator()(const std::vector<T*>& a, const std::vector<T*>& b) {
    if (a.size() != b.size()) {
      return false;
    }

    for (size_t i = 0, size = a.size(); i < size; i++) {
      if (a[i] == nullptr && b[i] == nullptr) {
        continue;
      }

      if (*a[i] != *b[i]) {
        return false;
      }
    }

    return true;
  }

  template <typename K, typename T>
  bool operator()(const std::unordered_map<K, std::shared_ptr<T>>& a,
                  const std::unordered_map<K, std::shared_ptr<T>>& b) {
    if (a.size() != b.size()) {
      return false;
    }

    for (const auto& p : a) {
      auto it = b.find(p.first);
      if (it == b.end()) {
        return false;
      }

      if (*it->second != *p.second) {
        return false;
      }
    }

    return true;
  }
};

}  // namespace altintegration

#endif  // ALTINTEGRATION_PTRCOMPARATOR_HPP
