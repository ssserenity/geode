Geode: The Otherlab computational geometry library
==================================================

Geode is an open source library of computational geometry and associated mathematical utilities
together with a fast, lightweight python binding layer.  The license is standard three-clause BSD
(see the included `LICENSE` file or [LICENSE](https://github.com/otherlab/core/blob/master/LICENSE)).

### Dependencies

For C++:

* [boost >= 1.46](http://www.boost.org): Various C++ utility libraries (Boost Software License)
* [scons >= 2.0](http://www.scons.org): A build system (MIT license)
* [gmp >= 5.0](http://gmplib.org): Arbitrary precision arithmetic (LGPL)
* [cblas](http://www.netlib.org/blas/blast-forum/cblas.tgz): C wrappers for BLAS (BSD license)

For Python:

* [python >= 2.7](http://python.org): A scripting language (Python Software Foundation (PSF) license)
* [numpy >= 1.5](http://numpy.scipy.org): Efficient multidimensional arrays for Python (BSD license)
* [setuptools >= 0.6](http://pythonhosted.org/setuptools): A packaging system for Python (PSF license)

Optional dependencies (see below for how to disable these):

* [py.test >= 2.1](http://pytest.org): Simple python testing (MIT license)
* [scipy](http://www.scipy.org): Scientific computation for Python (BSD license)
* [openexr](http://www.openexr.com): High dynamic range floating point image format (BSD license)
* [libpng](http://www.libpng.org): Lossless image format (Custom noncopyleft license)
* [libjpeg](http://www.ijg.org): Lossy image format (Custom noncopyleft license)
* [openmesh](http://www.openmesh.org): Halfedge triangle mesh data structure (LGPL)

Geode makes extensive use of C++11 features, so a relatively recent C++ compiler is necessary.  So far the code has been tested on

* [gcc 4.6](http://gcc.gnu.org)
* [gcc 4.7](http://gcc.gnu.org)
* [clang 3.1](http://clang.llvm.org)
* [clang 3.2](http://clang.llvm.org)

### Installation

If necessary, dependencies can be installed via

    # Debian/Ubuntu
    sudo apt-get install python python-numpy scons libboost-dev \
      python-scipy python-py libpng-dev libjpeg-dev libopenexr-dev # optional

    # Homebrew
    TODO

    # MacPorts
    sudo port -v install python26 py26-numpy scons boost \
      py26-scipy py26-py libpng libjpeg openexr # optional

Geode can then be installed from source via

    git clone https//github.com/otherlab/geode.git
    cd geode

    # Install c++ headers and libraries to /usr/local
    scons -j 5
    sudo scons install

    # Install python bindings
    sudo python setup.py install

### Testing

Unit tests can be run via

    cd geode
    py.test

### Extra configuration

If additional build configuration is necessary, create a `config.py` file and set any desired options.  For example,

    # config.py
    CXX = 'clang++'
    cache = '/tmp/scons-cache'

These options can also be passed on the command line to scons.  Run `scons -h` for a complete list.
Use `type=debug` for a much slower build with many more assertions

    scons -j 5 type=debug

The following flags can be used to disable optional components:

    has_python = 0
    has_openexr = 0
    has_libpng = 0
    has_libjpeg = 0
    has_openmesh = 0

The libraries are built into `build/$arch/$type` (`build/native/release` by default) if you want to use
them without installing.

### Acknowledgements

Parts of geode come from the [PhysBAM simulation library](http://physbam.stanford.edu) developed by
Ron Fedkiw et al. at Stanford University.

For random numbers, we use the [Random123 library](http://www.deshawresearch.com/resources_random123.html) of
John Salmon et al. at D. E. Shaw Research.  Random123 is included inline in `core/random/random123`.

The interval arithmetic in `exact/Interval` is based on code by [Robert Bridson](http://www.cs.ubc.ca/~rbridson)
and [Tyson Brochu](http://www.cs.ubc.ca/~tbrochu).
