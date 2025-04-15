### Build/test container ###
# Define builder stage
FROM vibe-code-only:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:noble as deploy

# Copy server output binary from builder stage
COPY --from=builder /usr/src/project/build/bin/webserver .

# Create a directory for config files
RUN mkdir -p /etc/webserver

# Expose port 8080 for development
EXPOSE 8080
EXPOSE 80

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["/etc/webserver/server_config"]