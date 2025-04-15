#!/bin/bash
# Script to build/test the webserver docker container
set -euo pipefail

echo "Building base image"
docker build -f docker/base.Dockerfile -t vibe-code-only:base .

echo "Building server image"
docker build -f docker/Dockerfile -t vibe-code-only:latest .

echo "Starting container"
CID=$(docker run -d -p 8080:8080 \
  -v "$(pwd)/config/production_config:/etc/webserver/server_config:ro" \
  vibe-code-only:latest)

echo "Waiting for server to become responsive"
for i in {1..10}; do
  if curl -s -o /dev/null http://localhost:8080; then
    echo "✅ Server responded"
    break
  fi
  echo "Waiting for server..."
  sleep 1
done

# Using netcat
echo -e "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n" | nc localhost 8080

echo "Stopping container"
docker stop "$CID" > /dev/null