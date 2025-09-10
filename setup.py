from setuptools import setup, find_packages, Extension
from distutils.command.build_ext import build_ext as build_ext_orig
import sys

# Read the long description from README.md
with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()
    

compile_args = ['-Wall', '-O3', '-fPIC'] # Common warnings

class CTypesExtension(Extension):
    pass

class build_ext(build_ext_orig):
    def build_extension(self, ext):
        self._ctypes = isinstance(ext, CTypesExtension)
        return super().build_extension(ext)

    def get_export_symbols(self, ext):
        if self._ctypes:
            return ext.export_symbols
        return super().get_export_symbols(ext)

    def get_ext_filename(self, ext_name):
        if self._ctypes:
            return ext_name + ".so"
        return super().get_ext_filename(ext_name)

setup(
    name="fastchess-engine",
    version="0.1.0",
    author="ThefCraft", # Replace with your actual name
    author_email="sisodiyalaksh@gmail.com", # Replace with your actual email
    description="A high-performance chess engine with a C backend and Python API.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/thefcraft/FastChess", # Replace with your GitHub repository URL
    packages=find_packages(where='src'),
    package_dir={'': 'src'},
    ext_modules=[CTypesExtension(
            "fastchess._FastChessC",
            sources=["src/fastchess/FastChess.c"],
            extra_compile_args=compile_args
    )],
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Intended Audience :: Developers",
        "Topic :: Games/Entertainment :: Board Games",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
    ],
    python_requires='>=3.8',
    install_requires=[],
    cmdclass={"build_ext": build_ext},
)