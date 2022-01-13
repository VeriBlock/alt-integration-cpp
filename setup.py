import os
import platform
import subprocess
import sys
import multiprocessing
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import distutils

thisdir = os.path.abspath(os.path.dirname(__file__))
cwd = os.getcwd()

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

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(
            os.path.dirname(self.get_ext_fullpath(ext.name)))
        cfg = 'Debug' if self.debug else 'Release'

        sodir = os.path.join(extdir, 'pypoptools', 'pypopminer')
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
            build_args += ['--', '-j{}'.format(multiprocessing.cpu_count())]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        cmake_cmd = ['cmake', str(thisdir)] + cmake_args

        print("""
        {cmake}
        cwd       = {cwd}
        src       = {src}
        build dir = {build}
        """.format(
            cmake=' '.join(cmake_cmd),
            cwd=cwd,
            src=thisdir,
            build=self.build_temp
        ))

        subprocess.check_call(cmake_cmd, cwd=self.build_temp)
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
    'pypoptools.pypoptesting.tests.node',
    'pypoptools.pypoptesting.tests.rpc',
    'pypoptools.pypoptesting.tests.pop',
    'pypoptools.pypoptesting.benchmarks',
    'pypoptools.pypopminer',
]

setup(
    name='pypoptools',
    version='0.1',
    packages=packages,
    url='https://github.com/VeriBlock/alt-integration-cpp',
    license='MIT',
    author='warchant',
    author_email='warchantua@gmail.com',
    description='POP-tools for testing',
    python_requires='>=3.6',
    install_requires=[
        'requests',
        'dataclasses',
        'cmake'
    ],
    ext_modules=[CMakeExtension('pypopminer', sourcedir=str(thisdir))],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
    setup_requires=[
        'pytest-runner',
        'wheel',
        'cmake'
    ],
    tests_require=[
        'pytest'
    ],
)
