# Documentation

This directory is here to host the meson.build file with the documentation run
target. This is done out of convenience for placing the generated documentation
in the mirror project directory tree of the build.

## Building

The documentation target is not built by default. To generate the documentation,
set up a build directory and build the 'doc' target explicitly:

```bash
$ meson setup build
$ meson compile -C build doc
```

where `build` is the build directory you want to set up and `doc` is the target.
