# Modifying build setup {#integration_1_build}

[TOC]

# Overview

This section contains information about adding veriblock-pop C++ library to your fork of Bitcoin.

# 1. Build and install veriblock-pop-cpp library.

```sh
git clone https://github.com/VeriBlock/alt-integration-cpp.git
cd alt-integration-cpp
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j4
sudo make install
```

@note Building veriblock-pop-cpp library requires CMake 3.12 or newer

# 2. Add veriblock-pop-cpp library dependency.

@note We use Bitcoin source code as reference. Other integrations should adhere to another project structure, build system, programming language, etc.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/configure.ac](https://github.com/VeriBlock/vbk-ri-btc/blob/master/configure.ac)
```cpp
PKG_CHECK_MODULES([CRYPTO], [libcrypto],,[AC_MSG_ERROR(libcrypto not found.)])
+      # VeriBlock
+      export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib64/pkgconfig
+      export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
+      export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/lib64/pkgconfig
+      export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/lib/pkgconfig
+      PKG_CHECK_MODULES([VERIBLOCK_POP_CPP], [veriblock-pop-cpp],,[AC_MSG_ERROR(libveriblock-pop-cpp not found.)])
```
```cpp
else
+  # VeriBlock
+  AC_CHECK_HEADER([veriblock/pop.hpp],,AC_MSG_ERROR(veriblock-pop-cpp headers missing))
+  AC_CHECK_LIB([veriblock-pop-cpp],[main],[VERIBLOCK_POP_CPP_LIBS=" -lveriblock-pop-cpp"],AC_MSG_ERROR(veriblock-pop-cpp missing))
+  AC_ARG_VAR(VERIBLOCK_POP_CPP_LIBS, "linker flags for VERIBLOCK_POP_CPP")
+
   AC_CHECK_HEADER([openssl/crypto.h],,AC_MSG_ERROR(libcrypto headers missing))
```

After this, `VERIBLOCK_POP_CPP_LIBS` variable in Makefile will contain all link flags required to link `veriblock-pop-cpp` to your project.

Now, link each target in a project to `VERIBLOCK_POP_CPP_LIBS`. Note that some targets may list some libraries twice - to resolve circular dependencies.

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.am)
```cpp
bitcoind_LDADD = \
+  $(VERIBLOCK_POP_CPP_LIBS) \
   $(LIBBITCOIN_SERVER) \
   $(LIBBITCOIN_WALLET) \
   $(LIBBITCOIN_SERVER) \
   $(LIBBITCOIN_COMMON) \
   $(LIBUNIVALUE) \
   $(LIBBITCOIN_UTIL) \
   $(LIBBITCOIN_ZMQ) \
   $(LIBBITCOIN_CONSENSUS) \
   $(LIBBITCOIN_CRYPTO) \
   $(LIBLEVELDB) \
   $(LIBLEVELDB_SSE42) \
   $(LIBMEMENV) \
   $(LIBSECP256K1)
```
```cpp
bitcoin_cli_LDADD = \
+  $(VERIBLOCK_POP_CPP_LIBS) \
   $(LIBBITCOIN_CLI) \
   $(LIBUNIVALUE) \
   $(LIBBITCOIN_UTIL) \
   $(LIBBITCOIN_CRYPTO)
```
```cpp
bitcoin_tx_LDADD = \
+  $(VERIBLOCK_POP_CPP_LIBS) \
   $(LIBUNIVALUE) \
   $(LIBBITCOIN_COMMON) \
   $(LIBBITCOIN_UTIL) \
```
```cpp
-bitcoin_tx_LDADD += $(BOOST_LIBS)
+bitcoin_tx_LDADD += $(BOOST_LIBS) $(VERIBLOCK_POP_CPP_LIBS)
```
```cpp
bitcoin_wallet_LDADD = \
+  $(VERIBLOCK_POP_CPP_LIBS) \
   $(LIBBITCOIN_WALLET_TOOL) \
   $(LIBBITCOIN_WALLET) \
   $(LIBBITCOIN_COMMON) \
   $(LIBBITCOIN_CONSENSUS) \
   $(LIBBITCOIN_UTIL) \
   $(LIBBITCOIN_CRYPTO) \
   $(LIBBITCOIN_ZMQ) \
   $(LIBLEVELDB) \
   $(LIBLEVELDB_SSE42) \
   $(LIBMEMENV) \
   $(LIBSECP256K1) \
   $(LIBUNIVALUE)

-bitcoin_wallet_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(MINIUPNPC_LIBS) $(ZMQ_LIBS)
+bitcoin_wallet_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(MINIUPNPC_LIBS) $(ZMQ_LIBS) $(VERIBLOCK_POP_CPP_LIBS)
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.bench.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.bench.include)
```cpp
bench_bench_bitcoin_LDADD = \
   $(LIBBITCOIN_SERVER) \
   $(LIBBITCOIN_WALLET) \
   $(LIBBITCOIN_SERVER) \
   $(LIBBITCOIN_COMMON) \
   $(LIBBITCOIN_UTIL) \
   $(LIBBITCOIN_CONSENSUS) \
   $(LIBBITCOIN_CRYPTO) \
   $(LIBTEST_UTIL) \
   $(LIBLEVELDB) \
   $(LIBLEVELDB_SSE42) \
   $(LIBMEMENV) \
   $(LIBSECP256K1) \
   $(LIBUNIVALUE) \
   $(EVENT_PTHREADS_LIBS) \
+  $(VERIBLOCK_POP_CPP_LIBS) \
   $(EVENT_LIBS)
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.test.include)
```cpp
test_test_bitcoin_LDADD += $(BDB_LIBS) $(MINIUPNPC_LIBS) $(RAPIDCHECK_LIBS)
test_test_bitcoin_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) -static

+ test_test_bitcoin_LDADD += $(VERIBLOCK_POP_CPP_LIBS)
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.qt.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.qt.include)
```cpp
-qt_bitcoin_qt_LDADD = qt/libbitcoinqt.a $(LIBBITCOIN_SERVER)
+qt_bitcoin_qt_LDADD = qt/libbitcoinqt.a $(VERIBLOCK_POP_CPP_LIBS) $(LIBBITCOIN_SERVER)
if ENABLE_WALLET
qt_bitcoin_qt_LDADD += $(LIBBITCOIN_UTIL) $(LIBBITCOIN_WALLET)
endif
if ENABLE_ZMQ
qt_bitcoin_qt_LDADD += $(LIBBITCOIN_ZMQ) $(ZMQ_LIBS)
endif
qt_bitcoin_qt_LDADD += $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBBITCOIN_UTIL) $(LIBBITCOIN_CONSENSUS) $(LIBBITCOIN_CRYPTO) $(LIBUNIVALUE) $(LIBLEVELDB) $(LIBLEVELDB_SSE42) $(LIBMEMENV) \
  $(BOOST_LIBS) $(QT_LIBS) $(QT_DBUS_LIBS) $(QR_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(LIBSECP256K1) \
-  $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS)
+  $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(VERIBLOCK_POP_CPP_LIBS)
qt_bitcoin_qt_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(QT_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)
qt_bitcoin_qt_LIBTOOLFLAGS = $(AM_LIBTOOLFLAGS) --tag CXX
```

[https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.qttest.include](https://github.com/VeriBlock/vbk-ri-btc/blob/master/src/Makefile.qttest.include)
```cpp
 qt_test_test_bitcoin_qt_LDADD += $(LIBBITCOIN_CLI) $(LIBBITCOIN_COMMON) $(LIBBITCOIN_UTIL) $(LIBBITCOIN_CONSENSUS) $(LIBBITCOIN_CRYPTO) $(LIBUNIVALUE) $(LIBLEVELDB) \
   $(LIBLEVELDB_SSE42) $(LIBMEMENV) $(BOOST_LIBS) $(QT_DBUS_LIBS) $(QT_TEST_LIBS) $(QT_LIBS) \
-  $(QR_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(NATPMP_LIBS) $(LIBSECP256K1) \
-  $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(SQLITE_LIBS)
-qt_test_test_bitcoin_qt_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(QT_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)
+  $(QR_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(LIBSECP256K1) \
+  $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(VERIBLOCK_POP_CPP_LIBS)
```


[Next Section](./integration_2_popdata.md)
