import os
import pathlib
import subprocess
import platform
import re
import sys

from setuptools import Extension
from setuptools import setup
from distutils.version import LooseVersion
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

thisdir = pathlib.Path(__file__).parent
cwd = os.getcwd()

if cwd != str(thisdir):
    raise Exception("""
    
pypoptools is installable only from dir with setup.py!
    Current working dir: {}
    setup.py dir       : {}
    """.format(cwd, thisdir))

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: " +
                ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)',
                                                   out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))
        cfg = 'Debug' if self.debug else 'Release'

        sodir = pathlib.Path(extdir, 'pypoptools')
        sodir = pathlib.Path(sodir, 'pypopminer')
        cmake_args = [
            '-DPYPOPMINER_OUTPUT_DIR=' + str(sodir),
            '-DCMAKE_BUILD_TYPE=' + cfg,
            '-DWITH_PYPOPTOOLS=ON',
            '-DFUZZING=OFF',
            '-DTESTING=OFF',
            '-DBENCHMARKING=OFF',
            '-DWERROR=NO',
            '-DPYTHON_EXECUTABLE=' + sys.executable
        ]

        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), sodir)]
            if sys.maxsize > 2 ** 32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j4']

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args,
                              cwd=self.build_temp)
        subprocess.check_call(['cmake', '--build', '.'] + build_args,
                              cwd=self.build_temp)


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


packages = [
    'pypoptools',
    'pypoptools.pypoptesting',
    'pypoptools.pypoptesting.framework',
    'pypoptools.pypoptesting.tests',
    'pypoptools.pypopminer',
]

setup(
    name='pypoptools',
    version='0.0.0',
    packages=packages,
    url='https://github.com/VeriBlock/alt-integration-cpp',
    license='MIT',
    author='warchant',
    author_email='',
    description='',
    python_requires='>=3.6',
    install_requires=['requests', 'dataclasses'],
    ext_modules=[CMakeExtension('pypopminer', sourcedir='..')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
)
