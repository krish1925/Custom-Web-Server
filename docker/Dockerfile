### Build/test container ###
# Define builder stage
FROM vibe-code-only:base AS builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
# Build the test executable specifically
RUN make webserver_test

# Run any CTest-based tests
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:noble AS deploy

# Create necessary directories
RUN mkdir -p /etc/webserver

# Copy server binary to standard location
COPY --from=builder /usr/src/project/build/bin/webserver /usr/local/bin/webserver

# Copy test binary for potential debugging/testing in production
COPY --from=builder /usr/src/project/build/bin/webserver_test /usr/local/bin/webserver_test

# Copy the config file from the project
COPY --from=builder /usr/src/project/config/gcloud_config /etc/webserver/config

# Create a default server config if no volume is mounted
RUN echo "port 8080;" > /etc/webserver/server_config

# Expose both standard HTTP ports
EXPOSE 8080
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["/usr/local/bin/webserver"]

# Use CMD to specify arguments to ENTRYPOINT
# Default to server_config but allow override through environment or runtime
CMD ["/etc/webserver/server_config"]
