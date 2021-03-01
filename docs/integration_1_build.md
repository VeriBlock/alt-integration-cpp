# POP Integration. Modifying build setup. {#integration_1_build}

[TOC]

# Overview

This section contains information about adding veriblock-pop C++ library to your project.

# 1. Build and install veriblock-pop-cpp library

```sh
git clone https://github.com/VeriBlock/alt-integration-cpp.git
cd alt-integration-cpp
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_PYPOPTOOLS=ON
make
sudo make install
cd ..
python3 setup.py install --user
```

@note Building veriblock-pop-cpp library requires CMake 3.12 or newer

# 2. Add veriblock-pop-cpp library dependency

@note We use Bitcoin source code as reference. Other integrations should adhere to another project structure, build system, programming language, etc.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/configure.ac](https://github.com/VeriBlock/vbk-ri-btc/blob/master/configure.ac)
```diff
PKG_CHECK_MODULES([CRYPTO], [libcrypto],,[AC_MSG_ERROR(libcrypto not found.)])
+      # VeriBlock
+      echo "pkg-config is used..."
+      export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib64/pkgconfig
+      export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
+      export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/lib64/pkgconfig
+      export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/lib/pkgconfig
+      PKG_CHECK_MODULES([VERIBLOCK_POP_CPP], [veriblock-pop-cpp],,[AC_MSG_ERROR(libveriblock-pop-cpp not found.)])
```
```diff
else
+  # VeriBlock
+  AC_CHECK_HEADER([veriblock/pop_context.hpp],,AC_MSG_ERROR(veriblock-pop-cpp headers missing))
+  AC_CHECK_LIB([veriblock-pop-cpp],[main],[VERIBLOCK_POP_CPP_LIBS=" -lveriblock-pop-cpp"],AC_MSG_ERROR(veriblock-pop-cpp missing))
+
+  AC_ARG_VAR(VERIBLOCK_POP_CPP_LIBS, "linker flags for VERIBLOCK_POP_CPP")
+
   AC_CHECK_HEADER([openssl/crypto.h],,AC_MSG_ERROR(libcrypto headers missing))
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am)
```diff
bitcoind_LDADD = \
+  $(VERIBLOCK_POP_CPP_LIBS) \
+  $(LIBBITCOIN_SERVER) \
   $(LIBBITCOIN_WALLET) \
   $(LIBBITCOIN_COMMON) \
-  $(LIBBITCOIN_UTIL) \
   $(LIBUNIVALUE) \
+  $(LIBBITCOIN_UTIL) \
```
```diff
+bitcoind_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS) $(VERIBLOCK_POP_CPP_LIBS)
```
```diff
bitcoin_cli_LDADD = \
   $(LIBBITCOIN_CLI) \
   $(LIBUNIVALUE) \
   $(LIBBITCOIN_UTIL) \
-  $(LIBBITCOIN_CRYPTO)
+  $(LIBBITCOIN_CRYPTO) \
+  $(VERIBLOCK_POP_CPP_LIBS)
```
```diff
bitcoin_tx_LDADD = \
+  $(VERIBLOCK_POP_CPP_LIBS) \
   $(LIBUNIVALUE) \
   $(LIBBITCOIN_COMMON) \
   $(LIBBITCOIN_UTIL) \
```
```diff
+bitcoin_tx_LDADD += $(BOOST_LIBS) $(VERIBLOCK_POP_CPP_LIBS)
```
```diff
bitcoin_wallet_LDADD = \
+  $(LIBBITCOIN_WALLET_TOOL) \
+  $(LIBBITCOIN_WALLET) \
   $(LIBBITCOIN_COMMON) \
-  $(LIBBITCOIN_UTIL) \
-  $(LIBUNIVALUE) \
   $(LIBBITCOIN_CONSENSUS) \
+  $(LIBBITCOIN_UTIL) \
   $(LIBBITCOIN_CRYPTO) \
-  $(LIBSECP256K1)
+  $(LIBBITCOIN_ZMQ) \
+  $(LIBLEVELDB) \
+  $(LIBLEVELDB_SSE42) \
+  $(LIBMEMENV) \
+  $(LIBSECP256K1) \
+  $(LIBUNIVALUE) \
+  $(VERIBLOCK_POP_CPP_LIBS)
```
```diff
+bitcoin_wallet_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(MINIUPNPC_LIBS) $(ZMQ_LIBS)
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.bench.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.bench.include)
```diff
bench_bench_bitcoin_LDADD = \
   $(LIBSECP256K1) \
   $(LIBUNIVALUE) \
   $(EVENT_PTHREADS_LIBS) \
-  $(EVENT_LIBS)
+  $(EVENT_LIBS) \
+  $(VERIBLOCK_POP_CPP_LIBS)
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include)
```diff
-test_test_bitcoin_LDADD += $(LIBBITCOIN_SERVER) $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBBITCOIN_UTIL) $(LIBBITCOIN_CONSENSUS) $(LIBBITCOIN_CRYPTO) $(LIBUNIVALUE) \
-  $(LIBLEVELDB) $(LIBLEVELDB_SSE42) $(LIBMEMENV) $(BOOST_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(LIBSECP256K1) $(EVENT_LIBS) $(EVENT_PTHREADS_LIBS)
-test_test_bitcoin_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
+test_test_bitcoin_LDADD += $(LIBSECP256K1) $(VERIBLOCK_POP_CPP_LIBS) $(LIBBITCOIN_SERVER) $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBBITCOIN_UTIL) $(LIBBITCOIN_CONSENSUS) $(LIBBITCOIN_CRYPTO) $(LIBUNIVALUE) \
+  $(LIBLEVELDB) $(LIBLEVELDB_SSE42) $(LIBMEMENV) $(BOOST_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(EVENT_LIBS) $(EVENT_PTHREADS_LIBS)
+test_test_bitcoin_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS) -g -Og
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.qt.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.qt.include)
```diff
-bitcoin_qt_ldadd = qt/libbitcoinqt.a $(LIBBITCOIN_SERVER)
+bitcoin_qt_LDADD = qt/libbitcoinqt.a $(VERIBLOCK_POP_CPP_LIBS) $(LIBBITCOIN_SERVER)
```
```diff
+bitcoin_qt_LDADD += $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBBITCOIN_UTIL) $(LIBBITCOIN_CONSENSUS) $(LIBBITCOIN_CRYPTO) $(LIBUNIVALUE) $(LIBLEVELDB) $(LIBLEVELDB_SSE42) $(LIBMEMENV) \
+  $(BOOST_LIBS) $(QT_LIBS) $(QT_DBUS_LIBS) $(QR_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(LIBSECP256K1) \
+  $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(VERIBLOCK_POP_CPP_LIBS)
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.qttest.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.qttest.include)
```diff
 qt_test_test_bitcoin_qt_LDADD += $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBBITCOIN_UTIL) $(LIBBITCOIN_CONSENSUS) $(LIBBITCOIN_CRYPTO) $(LIBUNIVALUE) $(LIBLEVELDB) \
   $(LIBLEVELDB_SSE42) $(LIBMEMENV) $(BOOST_LIBS) $(QT_DBUS_LIBS) $(QT_TEST_LIBS) $(QT_LIBS) \
-  $(QR_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(NATPMP_LIBS) $(LIBSECP256K1) \
-  $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(SQLITE_LIBS)
-qt_test_test_bitcoin_qt_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(QT_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)
+  $(QR_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(LIBSECP256K1) \
+  $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(VERIBLOCK_POP_CPP_LIBS)
```