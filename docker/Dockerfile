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

RUN mkdir -p /etc/webserver
# TODO(!): Copy server output binary to "."
COPY --from=builder /usr/src/project/build/bin/webserver .

# creating a config file for the server:
RUN echo "port 8080;" > /etc/webserver/server_config


# TODO(!): Expose some port(s)
EXPOSE 8080

# Use ENTRYPOINT to specify the binary name
# TODO(!): Update with real server name
ENTRYPOINT ["./webserver"]

# Use CMD to specify arguments to ENTRYPOINT
# TODO(!): Update with real server args
CMD ["/etc/webserver/server_config"]
