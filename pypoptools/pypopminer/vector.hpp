// Copyright (c) 2019-2021 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_VECTOR_HPP
#define VERIBLOCK_POP_CPP_VECTOR_HPP

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <vector>

template <typename T>
std::string vector_repr(std::vector<T>& v) {
  return format("vector{size={}}", v.size());
}

template <typename T>
void reg_vector(const std::string& name) {
  using namespace boost::python;
  class_<std::vector<T>>(name.c_str(), no_init)
      .def("__repr__", &vector_repr<T>)
      .def(vector_indexing_suite<std::vector<T>>());
}

#endif  // VERIBLOCK_POP_CPP_VECTOR_HPP
