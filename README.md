## aggrocow: over-engineered solution for cow allocation

aggrocow is a sample project for solving the [Aggressive Cows
problem](https://www.spoj.com/problems/AGGRCOW/).

Although the problem can be easily solved via a single application source file,
this project aims to produce an exaggerated solution for the purpose of
demonstrating my C programming style and allow me to endulge myself in writing a
bit of [Meson](https://mesonbuild.com/) along the way.

The project provides a library, `libaggrocow`, and a utility `aggrocow` that
uses the library to process the described input.

### Development

The library and executable are both written in compliance with C17 (ISO/IEC 9899:2018), although some POSIX.1-2008 functions (strdup(3)) and OpenBSD functions (reallocarray(3)) are used, which are provided by glibc.

#### Build dependencies

* The build system used is [Meson](https://mesonbuild.com/).
* The build backend is [Ninja](https://ninja-build.org/).
* The preferred compiler suite is [LLVM](https://llvm.org), with [clang](https://clang.llvm.org) as the front-end driver.
* On glibc systems, glibc version 2.29 or later is expected.
* If you want to build the pretty library documentation, you'll need [cldoc](https://jessevdk.github.io/cldoc/).

Consult the relevant project pages for how to obtain these tools for your operating system.

#### Building

Building the project is pretty straight forward. First we need to set up a build directory:
```sh
$ meson setup build
```

The above will create a directory called `build` in the project root directory, for the most part mirroring the project directory structure (give or take a few entries).

Then build the project:
```sh
$ meson compile -C build
```

The final artifacts can be found in the `build` directory, in the `src` directory for the `aggrocow` executable and `src/libaggrocow` directory for the `libaggrocow` library. The above will also produce a `pkg-config` database file for the library and executable.

If you wish to build the documentation for the library, build the `doc` target:
```sh
$ meson compile -C build doc
```

To view the generated documentation, run:
```sh
$ cldoc serve build/doc
```

#### TODOs and Great Ideas™

In no particular order...

* Write man-pages for `aggrocow` and `libaggrocow`
* Write library unit-tests
* Write some benchmarks
* Properly version the library and give it a proper `SONAME`
* Make Meson query the VCS for version information
* Find or write a simple logging facade to use
* Make the library MT-safe
* Make the executable multithreaded
* Write a web service based on the library to help allocate remote cows
* Write a lexer and a parser for the input data grammar

#### Support

The project is known to build on Fedora 36 and OpenBSD 7.1 (with -current changes, from a snapshot dated July 18, 2022) using Meson 0.52.2 and Ninja 1.10.2 as the backend.

### Installing

To install the artifacts for staging, run:
```sh
$ meson install -C build --destdir fakeroot
```

This will create a staging directory `fakeroot` under the `build` directory, which can then be carved up for packages.

### Licensing

The project is under an ISC license, a copy of which is provided in the [LICENSE](LICENSE) file.
