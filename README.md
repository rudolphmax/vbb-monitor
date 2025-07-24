# VBB Monitor

## Prerequisites

- clang / gcc (c++ compiler)
- CMake >= 3.23
- [conan](https://conan.io/)


## Building from source

To build the project from source, run the following command.

```
$ make install
$ make build
```

The built executable can then be found under `build/vbb_monitor`.

If needed, specify the path to your SSL root certificate bundle by setting the `ROOT_CERT_BUNDLE_LOCATION` environment variable (e.g. `export ROOT_CERT_BUNDLE_LOCATION=/path/to/certificates.pem`).


## Developing

To create a debug build (i.e. for debuggers), run

```
$ make install-debug
$ make build-debug
```

The built executable can then be found under `build/vbb_monitor`.

If needed, specify the path to your SSL root certificate bundle by setting the `ROOT_CERT_BUNDLE_LOCATION` environment variable (e.g. `export ROOT_CERT_BUNDLE_LOCATION=/path/to/certificates.pem`).

For [zed](https://zed.dev) users, a debugger config is provided.
