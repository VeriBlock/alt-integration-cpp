// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#ifndef VERIBLOCK_POP_CPP_CONVERTERS_HPP
#define VERIBLOCK_POP_CPP_CONVERTERS_HPP

#include <boost/noncopyable.hpp>
#include <boost/python.hpp>
#include <boost/python/to_python_converter.hpp>

using namespace boost::python;
using namespace altintegration;

// Converts a C++ vector to a python list
template <class T>
boost::python::list toPythonList(std::vector<T> vector) {
  typename std::vector<T>::iterator iter;
  boost::python::list list;
  for (iter = vector.begin(); iter != vector.end(); ++iter) {
    list.append(*iter);
  }
  return list;
}

template <typename Container>
struct container_to_list {
  static PyObject* convert(Container const& s) {
    return boost::python::incref(boost::python::object(toPythonList(s)).ptr());
  }
};

/// @brief Type that allows for registration of conversions from
///        python iterable types.
struct iterable_converter {
  /// @note Registers converter from a python interable type to the
  ///       provided type.
  template <typename Container>
  iterable_converter& reg() {
    boost::python::converter::registry::push_back(
        &iterable_converter::convertible,
        &iterable_converter::construct<Container>,
        boost::python::type_id<Container>());

    boost::python::to_python_converter<Container,
                                       container_to_list<Container>>();

    // Support chaining.
    return *this;
  }

  /// @brief Check if PyObject is iterable.
  static void* convertible(PyObject* object) {
    return PyObject_GetIter(object) ? object : NULL;
  }

  /// @brief Convert iterable PyObject to C++ container type.
  ///
  /// Container Concept requirements:
  ///
  ///   * Container::value_type is CopyConstructable.
  ///   * Container can be constructed and populated with two iterators.
  ///     I.e. Container(begin, end)
  template <typename Container>
  static void construct(
      PyObject* object,
      boost::python::converter::rvalue_from_python_stage1_data* data) {
    namespace python = boost::python;
    // Object is a borrowed reference, so create a handle indicting it is
    // borrowed for proper reference counting.
    python::handle<> handle(python::borrowed(object));

    // Obtain a handle to the memory block that the converter has allocated
    // for the C++ type.
    typedef python::converter::rvalue_from_python_storage<Container>
        storage_type;
    void* storage = reinterpret_cast<storage_type*>(data)->storage.bytes;

    typedef python::stl_input_iterator<typename Container::value_type> iterator;

    // Allocate the C++ type into the converter's memory block, and assign
    // its handle to the converter's convertible variable.  The C++
    // container is populated by passing the begin and end iterators of
    // the python object to the container's constructor.
    new (storage) Container(iterator(python::object(handle)),  // begin
                            iterator());                       // end
    data->convertible = storage;
  }
};

template <typename Container>
struct container_to_pystring {
  static PyObject* convert(Container const& s) {
    return boost::python::incref(boost::python::object(HexStr(s)).ptr());
  }
};

/// @brief Type that allows for conversions of python strings to
//         vectors.
struct pystring_converter {
  /// @note Registers converter from a python interable type to the
  ///       provided type.
  template <typename Container>
  pystring_converter& reg() {
    boost::python::converter::registry::push_back(
        &pystring_converter::convertible,
        &pystring_converter::construct<Container>,
        boost::python::type_id<Container>());

    boost::python::to_python_converter<Container,
                                       container_to_pystring<Container>>();

    return *this;
  }

  /// @brief Check if PyObject is a string.
  static void* convertible(PyObject* object) {
    if (PyBytes_Check(object)) {
      return object;
    }
    if (PyUnicode_Check(object)) {
      return object;
    }

    return NULL;
  }

  /// @brief Convert PyString to Container.
  ///
  /// Container Concept requirements:
  ///
  ///   * Container::value_type is CopyConstructable from char.
  ///   * Container can be constructed and populated with two iterators.
  ///     I.e. Container(begin, end)
  template <typename Container>
  static void construct(
      PyObject* object,
      boost::python::converter::rvalue_from_python_stage1_data* data) {
    namespace python = boost::python;
    // Object is a borrowed reference, so create a handle indicting it is
    // borrowed for proper reference counting.
    python::handle<> handle(python::borrowed(object));

    // Obtain a handle to the memory block that the converter has allocated
    // for the C++ type.
    typedef python::converter::rvalue_from_python_storage<Container>
        storage_type;
    void* storage = reinterpret_cast<storage_type*>(data)->storage.bytes;

    // Allocate the C++ type into the converter's memory block, and assign
    // its handle to the converter's convertible variable.  The C++
    // container is populated by passing the begin and end iterators of
    // the python object to the container's constructor.
    std::vector<uint8_t> parsed;
    if (PyBytes_Check(object)) {
      const char* begin = PyBytes_AsString(object);
      const char* end = begin + PyBytes_Size(object);
      parsed = ParseHex(std::string(begin, end));
    } else if (PyUnicode_Check(object)) {
      Py_ssize_t size;
      const char* begin = PyUnicode_AsUTF8AndSize(object, &size);
      parsed = ParseHex(std::string(begin, begin + size));
    }

    data->convertible = new (storage) Container(parsed);
  }
};

#endif  // VERIBLOCK_POP_CPP_CONVERTERS_HPP
