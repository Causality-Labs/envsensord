FROM ubuntu:24.04

# Install dependencies
RUN apt-get -y update && \
    apt-get install -y --no-install-recommends \
    python3 \
    make \
    g++

WORKDIR /app

# Copy specific files
COPY Makefile /app/
COPY client.py /app/
COPY test_server.sh /app/

# Copy specific directories
COPY src/ /app/src/
COPY inc/ /app/inc/

RUN chmod +x /app/test_server.sh
RUN chmod +x /app/client.py

RUN make clean && make all

# Run the server (not the test script)
CMD ["./bin/server"]