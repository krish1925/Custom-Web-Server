# High Performance Web Server

A high-performance, configurable web server built with C++ and CMake. This project implements a robust HTTP server with support for static file serving, configurable handlers, and comprehensive test coverage.

## Features

- HTTP/1.1 compliant web server
- Configurable request handlers
- Static file serving
- Logging system with Boost.Log
- Comprehensive unit and integration tests
- Docker support for containerized deployment
- Code coverage reporting
- CMake-based build system

## Prerequisites

- CMake (version 3.10.0 or higher)
- C++ compiler with C++11 support
- Boost libraries (version 1.50 or higher)
  - system
  - log
  - log_setup
  - regex
- Google Test framework
- Docker (optional, for containerized deployment)

## Building the Project

### Local Build

1. Create a build directory:
```bash
mkdir build && cd build
```

2. Configure and build:
```bash
cmake ..
make
```

### Docker Build

Build and run using Docker:
```bash
cd docker
./docker-test.sh
```

## Project Structure

```
.
├── src/                # Source files
├── include/           # Header files
├── tests/             # Test files
│   ├── integration/   # Integration tests
│   └── unit/         # Unit tests
├── config/            # Configuration files
├── docker/            # Docker configuration
├── docs/              # Documentation
├── cmake/             # CMake configuration
└── build/             # Build directory
```

## Testing

The project includes comprehensive test coverage:

- Unit tests for individual components
- Integration tests for end-to-end functionality
- Code coverage reports

To run tests:
```bash
cd build
ctest
```

## Configuration

The server can be configured using the configuration files in the `config/` directory. See the documentation in `docs/` for detailed configuration options.

## Docker Deployment

The project includes Docker support for containerized deployment:

- `Dockerfile`: Main production image
- `base.Dockerfile`: Base image with dependencies
- `coverage.Dockerfile`: Image for running coverage tests
- `cloudbuild.yaml`: Google Cloud Build configuration

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

[Add your license information here]

## Authors

[Add author information here] 
