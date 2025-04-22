#!/bin/bash
set -euo pipefail

# Script to build/test the webserver docker container

echo "Creating test configuration directory"
mkdir -p tests
mkdir -p config

# Create default test config if it doesn't exist
if [ ! -f tests/port_config ]; then
  echo "Creating tests/port_config"
  echo "port 8080;" > tests/port_config
fi

# Create production config if it doesn't exist
if [ ! -f config/production_config ]; then
  echo "Creating config/production_config"
  echo "port 80;" > config/production_config
fi

echo "Building base image"
docker build -f docker/base.Dockerfile -t vibe-code-only:base .

echo "Building server image"
docker build -f docker/Dockerfile -t vibe-code-only:latest .

echo "Running unit tests"
# Overriding the ENTRYPOINT to run the test binary directly
docker run --rm --entrypoint /usr/local/bin/webserver_test vibe-code-only:latest

echo "Starting server container"
CID=$(docker run -d -p 8080:80 \
  -v "$(pwd)/config/production_config:/etc/webserver/server_config:ro" \
  vibe-code-only:latest)

# Wait up to ~10s for server
RETRIES=10
for i in $(seq 1 $RETRIES); do
  if nc -z 127.0.0.1 8080; then
    echo "✅ Server is up!"
    break
  fi
  echo "Waiting for server to respond..."
  sleep 1
done

if ! nc -z 127.0.0.1 8080; then
  echo "ERROR: Server never responded on port 8080"
  docker logs "$CID" || true
  docker stop "$CID"
  exit 1
fi

echo "Performing functional tests"
# Test with curl
if ! curl -fs http://127.0.0.1:8080; then
  echo "ERROR: Curl test failed!"
  docker logs "$CID" || true
  docker stop "$CID"
  exit 1
fi

# Test with netcat
echo -e "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" | nc localhost 8080

echo "Stopping container"
docker stop "$CID" > /dev/null

echo "✅ All tests passed successfully!"
