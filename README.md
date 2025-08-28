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

The built executable can then be found under `build/vbb_monitor`.

For development, use `make install-debug` and `make build-debug` to create a debug build. [Zed](https://zed.dev) users can use the provided debugger config.


## Running

Simply run the built executable:

```bash
$ ./build/vbb_monitor
```

Necessary information has to specified via command line options (recommended, for a list run with `--help`) or environment variables (see below).

### Environment Variables

| ENV Variable                | Description                                         | Example                           |
| --------------------------- | --------------------------------------------------- | --------------------------------- |
| `ROOT_CERT_BUNDLE_LOCATION` | (optional) Path to your SSL root certificate bundle | `"/path/to/certificates.pem"`     |
| `VBBMON_API_HOST`           | The hostname of the HAFAS API                       | hafas.example.com"                |
| `VBBMON_API_PORT`           | The port of the HAFAS API                           | 443 for SSL or 80 for plain http. |
| `VBBMON_API_BASE`           | The base url of API                                 | /api/info/v2                      |
| `VBBMON_ACCESS_ID`          | Your HAFAS Access-ID                                | Refer to HAFAS documentation      |
| `VBBMON_STOP_ID`            | The id of the stop to monitor                       | Refer to HAFAS documentation      |
| `VBBMON_REFRESH_INTERVAL`   | The data refresh interval in ms                     | 25000                             |
