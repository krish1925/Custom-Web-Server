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

The project includes Docker support for containerized deployment with a multi-stage build process:

### Local Docker Operations

1. Build the base image:
```bash
docker build -f docker/base.Dockerfile -t vibe-code-only:base .
```

2. Build the production image:
```bash
docker build -f docker/Dockerfile -t vibe-code-only:latest .
```

3. Run the container:
```bash
docker run --rm -p 80:80 vibe-code-only:latest
```

4. Check running containers:
```bash
docker ps
```

5. View container logs:
```bash
docker logs <container_id>
```

### Google Cloud Deployment

The project is configured for Google Cloud Build and Container Registry:

1. Set up Google Cloud project:
```bash
gcloud config set project <your-project-id>
```

2. Enable required APIs:
```bash
gcloud services enable cloudbuild.googleapis.com
gcloud services enable containerregistry.googleapis.com
```

3. Configure Cloud Build triggers:
   - Connect your GitHub repository to Cloud Build
   - Set up triggers for:
     - Push to main branch
     - Pull request creation
     - Tag creation

4. Build and deploy using Cloud Build:
```bash
gcloud builds submit --config=docker/cloudbuild.yaml
```

### Container Configuration

The production container includes:
- Web server binary at `/usr/local/bin/webserver`
- Configuration file at `/etc/webserver/config`
- Static assets at `/var/www/static`
- Log directory at `/var/log/webserver`

### Environment Variables

- `LOG_DIR`: Directory for log files (default: `/var/log/webserver`)
- Configuration can be overridden at runtime by mounting a custom config file

### Cloud Build Pipeline

The Cloud Build pipeline (`cloudbuild.yaml`) includes:
1. Pulling cached base image
2. Building and caching base image
3. Building production image
4. Building coverage test image
5. Pushing images to Container Registry

### Monitoring and Logging

1. View container logs:
```bash
docker logs <container_id>
docker cp <container_id:/logs ./logs_from_container
cd <path to logs_from_container>
cat logs/<date of log>.log
```

2. Access logs in production:
```bash
kubectl logs <pod-name> -n <namespace>
```

3. Monitor container metrics:
```bash
kubectl top pod <pod-name>
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request


## Authors

Alexander West
Krish Patel 
William Wu
Guanhua Ji
