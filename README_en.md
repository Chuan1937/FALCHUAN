# Introduce

This program is based on the [Visual Lossless Seismic Data Compression](http://ieco.upc.edu.cn/2024/0426/c20746a256826/page.htm) track of the 3rd Dongfang Cup National College Student Exploration Geophysics Development Competition. It has already won the third prize. 

With this, I am making it open-source and improving it based on the original, aiming to go beyond the constraints of the competition's problem so that other types of data can also be compressed.

The original program was designed for binary floating-point seismic data, so the current version can only compress and decompress binary files of two-dimensional pure numerical data. However, it is possible to specify the range of the two-dimensional matrix that needs to be compressed or decompressed. The program already includes a method for controllable precision visual lossless compression. Under visual lossless conditions, the highest compression rate is 90.61%, and the speed is about 7 seconds (Windows 10 Enterprise, AMD Ryzen 7 5800U, 16GB RAM).

# Installation
The program can only run on Windows platform at present, and Linux and Mac will be considered in the future.
The program depends on [zfp](https://github.com/LLNL/zfp) and [armadillo](https://arma.sourceforge.net/), which need to be configured first.
However, in this example, the required library files have been placed in include, and the corresponding library files have been placed in lib. You can configure the visual stdio project, and then add include and lib to the VC++ include directory and library directory in project->properties.


# Future work:
1. Add various lossless and lossy algorithms.
2. Support sgy, segy, and other seismic data formats.

# Citation
The core algorithm uses LLNL's [zfp](https://github.com/LLNL/zfp)

# Author
旗正飘飘--do your best and do your job

# License
BSD-3-Clause
Not for commercial use



I hope you can provide your valuable feedback, including the issues with the program and suggestions for future improvements or additional features that may be needed.

