#!/bin/bash
# Script to build/test the webserver docker container

echo "Creating test configuration directory..."
mkdir -p tests

if [ ! -f tests/port_config ]; then
  echo "Creating port configuration file..."
  echo "port 8080;" > tests/port_config
fi

# Create config directory for production config if it doesn't exist
mkdir -p config
if [ ! -f config/production_config ]; then
  echo "Creating production configuration file"
  echo "port 80;" > config/production_config
fi

echo "Building base image"
docker build -f docker/base.Dockerfile -t vibe-code-only:base .

echo "Building server image"
docker build -f docker/Dockerfile -t vibe-code-only:latest .

echo "Running server container on port 8080"
docker run -p 8080:8080 vibe-code-only:latest