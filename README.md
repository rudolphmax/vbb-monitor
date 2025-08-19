# VBB Monitor

Monitoring VBB stops at home using the dedicated API.

Access to the VBB API is required for usage, refer to VBB for further information.

## Prerequisites

- clang / gcc (c++ compiler)
- CMake >= 3.23
- [conan](https://conan.io/)
- [pre-commit](https://pre-commit.com)


## Building from source

To build the project from source, run the following command.

```bash
$ make install
$ make build
```

The built executable can then be found under `build/Release/vbb_monitor`.

For development, use `make install-debug` and `make build-debug` to create a debug build. [Zed](https://zed.dev) users can use the provided debugger config.


## Running

The monitor is configured with environment variables.

| ENV Variable                | Description                                         | Example                       |
| --------------------------- | --------------------------------------------------- | ----------------------------- |
| `ROOT_CERT_BUNDLE_LOCATION` | (optional) Path to your SSL root certificate bundle | `"/path/to/certificates.pem"` |
| `VBBMON_ACCESS_ID`          | Your HAFAS Access-ID                                | Refer to HAFAS documentation  |
| `VBBMON_STOP_ID`            | The id of the stop to monitor                       | Refer to HAFAS documentation  |
| `VBBMON_REFRESH_INTERVAL`   | The data refresh interval in ms                     | 25000                         |
