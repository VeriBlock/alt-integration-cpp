// Copyright (c) 2019-2022 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <veriblock/pop/value_sorted_map.hpp>

#include "FuzzedDataProvider.hpp"

enum Op : uint8_t { Insert = 0, Find = 1, Erase = 2, kMaxValue = 2 };

using namespace altintegration;

auto sort_map(const std::unordered_map<int, int>& a) {
  std::multiset<int, std::function<bool(const int&, const int&)>> v;
  for (auto& p : a) {
    v.insert(p.second);
  }
  return v;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
  std::unordered_map<int, int> m;
  ValueSortedMap<int, int> s{std::less<int>{}};

  FuzzedDataProvider p(Data, Size);

  int key = p.ConsumeIntegralInRange(0, 1000);
  int value = p.ConsumeIntegralInRange(0, 1000);

  Op operation = p.ConsumeEnum<Op>();
  switch (operation) {
    case Insert: {
      m.insert({key, value});
      s.insert(key, value);
      break;
    }
    case Find: {
      auto it1 = m.find(key);
      auto it2 = s.find(key);

      if (it1 == m.end()) {
        VBK_ASSERT(it2 == s.end());
        break;
      }

      if (it2 == m.end()) {
        VBK_ASSERT(it1 == m.end());
        break;
      }

      VBK_ASSERT(*it1 == *it2);
      break;
    }
    case Erase: {
      m.erase(key);
      s.erase(key);
      break;
    }
  }

  VBK_ASSERT(m.size() == s.size());

  auto v1 = sort_map(m);
  auto v2 = s.getSortedValues();

  VBK_ASSERT(v1 == v2);

  return 0;
}