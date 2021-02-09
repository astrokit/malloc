FROM ubuntu:latest

RUN set -ex; \
    apt-get update; \
    apt-get install -y --no-install-recommends build-essential g++-multilib

COPY . /app
