# Copyright (c) 2019-2020 Xenios SEZC
# https://www.veriblock.org
# Distributed under the MIT software license, see the accompanying
# file LICENSE or http://www.opensource.org/licenses/mit-license.php.

from distutils.core import setup, Extension

pypopminer_module = Extension('pypopminer',
                        sources=['pypopminer/miner.cpp'],
                        include_dirs=['/usr/local/include', '../include'],
                        library_dirs=['/usr/local/lib/boost', '/usr/local/lib'],
                        runtime_library_dirs=['/usr/local/lib/boost'],
                        libraries=['boost_python', 'veriblock-pop-cpp'])

setup(name='pypopminer',
      version='0.1',
      description='Python bindings for MockMiner',
      packages=['pypopminer'],
      ext_package='pypopminer',
      ext_modules=[pypopminer_module])