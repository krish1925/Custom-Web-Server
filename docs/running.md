# Running and Using the Web Server

This document explains how to build, run, and use the web server provided in the project. The server utilizes CMake for configuration, Boost for asynchronous I/O, and GoogleTest for testing. It accepts HTTP requests and echoes them back in plain text.

## Prerequisites

- **CMake** version 3.10.0 or later
- **Boost Library** (version 1.50 or later) with static libraries
- **GoogleTest** (installed in `/usr/src/googletest`) for running unit tests
- A C++ compiler with C++11 support or later
- Docker (optional) for containerized building and deployment

## Building the Server

### 1. Create a Build Directory
To avoid cluttering the source directory with build artifacts, create and switch to a new build directory:

```bash
mkdir build
cd build
```

### 2. Configure the Project
Run CMake from the build directory to set up the build configuration:

```bash
cmake ..
```

### 3. Build the Project
Compile the source code by executing:

```bash
make
```

### 4. Run Tests (Optional)
If tests are configured, run them to ensure your build is working as expected:

```bash
ctest --output-on_failure
```

## Running the Server

The compiled server executable (e.g., `webserver`) requires a configuration file as an argument. The configuration file should specify the server port, among other settings.

### Run the Server with a Configuration File
Execute the server binary, providing the path to your configuration file:

```bash
./bin/webserver /path/to/config/file
```

**Configuration File Guidelines:**

- The file must include at least one statement specifying the port using a format similar to:
  
  ```
  port 8080;
  ```

- If a valid port is not provided, the server defaults to port `8080`.

## Using the Server

- **HTTP Request Handling:**  
  The server listens for HTTP connections, reads the incoming request until it detects the end-of-headers marker (`\r\n\r\n`), and then responds with an HTTP 200 OK message. The response body echoes back the received HTTP request.

- **Logging:**  
  Important events—such as server startup, port details, and errors—are printed to the console.

---

## Docker Instructions

This project supports Docker for both building and deploying the server.

### 1. Build the Base Image

This builds the Docker image used as the base for building the server.

```bash
docker build -f docker/base.Dockerfile \
  -t vibe-code-only:base \
  -t gcr.io/<your-project-id>/vibe-code-only:base \
  --cache-from gcr.io/<your-project-id>/vibe-code-only:base \
  .
```

### 2. Push the Base Image (Optional)

If you're using Google Cloud Build:

```bash
docker push gcr.io/<your-project-id>/vibe-code-only:base
```

### 3. Build the Final Server Image

```bash
docker build -f docker/Dockerfile \
  -t gcr.io/<your-project-id>/vibe-code-only:latest \
  .
```

### 4. Run the Server in a Container

Before running, make sure you've updated the Dockerfile with:

- The actual server binary name
- Correct `EXPOSE` port
- Proper `ENTRYPOINT` and `CMD` lines

Then run the container:

```bash
docker run -p 8080:8080 gcr.io/<your-project-id>/vibe-code-only:latest
```

---

## Additional Notes

- **Debug Builds:**  
  If the environment variable `DEVEL_SHELL` is set to `1` and no build type is specified, the project defaults to a Debug build.

- **Customization:**  
  The server’s source files, executable name, and dependencies are defined in the `CMakeLists.txt` file. Modify these sections if your project requirements differ.

Follow these instructions to successfully build, run, and deploy the server. For further customization or troubleshooting, review the source code comments and configuration details in the repository.
