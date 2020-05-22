// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>
#include <veriblock/mock_miner.hpp>

#include "converters.hpp"

using namespace boost::python;
using namespace altintegration;

template <size_t N>
boost::shared_ptr<Blob<N>> makeBlob(const object& obj) {
  std::string hex = extract<std::string>(obj);
  auto b = Blob<N>::fromHex(hex);
  return boost::shared_ptr<Blob<N>>(new Blob<N>(std::move(b)));
}

template <size_t N>
void blob(std::string name) {
  using blob_t = Blob<N>;
  class_<blob_t, boost::noncopyable, boost::shared_ptr<blob_t>>(name.c_str())
      .def("__init__", make_constructor(&makeBlob<N>))
      .def("__str__", &blob_t::toHex)
      .def("__len__", &blob_t::size)
      .def("__repr__", &blob_t::toHex)
      .def("toHex", &blob_t::toHex);

  pystring_converter().reg<blob_t>();
}

std::string vecToHex(std::vector<uint8_t>& v) { return HexStr(v); }

void init_primitives() {
  class_<std::vector<uint8_t>,
         boost::noncopyable,
         boost::shared_ptr<std::vector<uint8_t>>>("ByteVector", no_init)
      .def("__str__", &vecToHex)
      .def("toHex", &vecToHex)
      .def("__len__", &std::vector<uint8_t>::size)
      .def("__repr__", &vecToHex);
  blob<256 / 8>("uint256");
  blob<192 / 8>("uint192");
  blob<128 / 8>("uint128");
  blob<96 / 8>("uint96");
  blob<72 / 8>("uint72");

  pystring_converter().reg<std::vector<uint8_t>>();
}