# Running and Using the Web Server

This document explains how to build, run, and use the web server provided in the project. The server utilizes CMake for configuration, Boost for asynchronous I/O, and GoogleTest for testing. It accepts HTTP requests and echoes them back in plain text.

## Prerequisites

- **CMake** version 3.10.0 or later
- **Boost Library** (version 1.50 or later) with static libraries
- **GoogleTest** (installed in `/usr/src/googletest`) for running unit tests
- A C++ compiler with C++11 support or later
- Docker (recommended) for containerized building and deployment

## Running the Server

The recommended way to run the server is with Docker. Alternatively, you can manually build and run it without Docker.

### Running with Docker (Default)

Docker handles the CMake configuration, building, and execution steps for you. To run the server in a container:

First, build the base container:

```bash
docker build \
  -f docker/base.Dockerfile \
  -t vibe-code-only:base \
  .
```

Then build the server container:

```bash
docker build \
  -f docker/Dockerfile \
  -t vibe-code-only:latest \
  .
```

Finally, run the container:

```bash
docker run \
  -p 80:80 \
  vibe-code-only:latest
```

**Testing the Server:**

Once the server is running, verify it by checking the port:

```bash
nc -zv localhost 80
```

A successful connection confirms the server is active.

**Stopping the Server:**

To stop the running server container:

1. List running containers and find the container ID:

```bash
docker ps
```

2. Stop the container using its ID:

```bash
docker stop <container-id>
```

### Building and Running Without Docker (Alternative)

If you prefer not to use Docker, you can build and run the server manually.

#### 1. Create a Build Directory
To avoid cluttering the source directory with build artifacts:

```bash
mkdir build
cd build
```

#### 2. Configure the Project
Run CMake to set up the build configuration:

```bash
cmake ..
```

#### 3. Build the Project
Compile the source code:

```bash
make
```

#### 4. (Optional) Run Tests
Run tests to verify your build:

```bash
ctest --output-on_failure
```

#### 5. Run the Server
After building, execute the server binary with your configuration file:

```bash
./bin/webserver /path/to/config/file
```

**Configuration File Guidelines:**
- The file must include at least one statement specifying the port, e.g.:

  ```
  port 80;
  ```

- If a valid port is not provided, the server defaults to port `80`.

**Testing the Server:**

After starting the server, verify it is running:

```bash
nc -zv localhost 80
```

---

## Using the Server

- **HTTP Request Handling:**  
  The server listens for HTTP connections, reads the incoming request until it detects the end-of-headers marker (`\r\n\r\n`), and then responds with an HTTP 200 OK message. The response body echoes back the received HTTP request.

- **Logging:**  
  Important events—such as server startup, port details, and errors—are printed to the console.

---

## How to Re-Deploy the Build

### To Re-Run the Build
You can manually trigger a new build in Google Cloud Build at any time:

- Go to the Cloud Build Triggers page.
- Find the trigger you set up (likely named something like `vibe-code-only-main-submit`).
- Click the **Run** button next to the trigger to start a build manually.
- The build will run the steps defined in your `docker/cloudbuild.yaml` file (compiling, testing, etc.).

Alternatively, any new push to the `main` branch (via Gerrit → mirrored repo) will also automatically trigger a new build.

### Where to Read Logs
To check the logs for your most recent or any past build:

- Go to the Cloud Build History page.
- You’ll see a list of builds — each row shows the status (e.g., success or failure).
- Click on any build ID (the hash) to see detailed logs.
- The logs are divided by steps, which match each command in your `cloudbuild.yaml`:
  - Example steps: *Build and test*, *Run unit tests*, *Generate coverage report*, etc.
- If you added test coverage generation, the coverage summary will appear in the final step’s logs (look for lines after **Generating coverage report**).

### Tips
- If a build fails, the log will show which step failed and the exact error message.
- If you don’t see your latest code in the build, make sure the push to `main` was mirrored from Gerrit to Cloud Source Repos or GitHub (depending on your setup).
- You can filter the history view by status or branch if needed.

---

## Additional Notes

- **Debug Builds:**  
  If the environment variable `DEVEL_SHELL` is set to `1` and no build type is specified, the project defaults to a Debug build.

- **Customization:**  
  The server’s source files, executable name, and dependencies are defined in the `CMakeLists.txt` file. Modify these sections if your project requirements differ.

Follow these instructions to successfully build, run, and deploy the server. For further customization or troubleshooting, review the source code comments and configuration details in the repository.
