# Environmental Sensor Server

Repository for an environmental sensor daemon that exposes temperature, pressure, and humidity data via a TCP/IP connection.

Tested on Debian GNU/Linux 13 (trixie) on a BeagleBone Black.
## Docker

we use docker for creating a consistent development enviroment.

To build the container:
```bash
$ docker build -t bbb-cross .
```

To enter the container for developement run the command:
```bash
$ docker run --rm -it --network=host -v $(pwd):/build bbb-cross /bin/bash
```

Where:
- `--rm` removes the container when you exit
- `-it` provides an interactive terminal
- `--network=host` uses your host machine's network (for hostname resolution)
- `-v $(pwd):/build` mounts your current directory to `/build` in the container
- `bbb-cross` is the image name
- `/bin/bash` starts a bash shell

To exit the contianer:
```bash
$ exit
```

Now you have a working container for cross compiling the application.
## Device Tree
Compile the DTS:
```bash
$ dtc -O dtb -o bin/bme280-overlay.dtbo bme280-overlay.dts
```

Copy the compiled overlay to `/lib/firmware`, then add it to `/boot/uEnv.txt` so it is loaded during boot.
```
### Additional custom capes
uboot_overlay_addr4=/lib/firmware/bme280-overlay.dtbo
```

Also ensure you enable U-Boot overlays:
```
enable_uboot_overlays=1
```

## Testing and deploying

#### Server

Copy the compiled server code to your target
```bash
$ scp ...
```

Excute the program as a background process
```bash
$ ./server &
```

The server supports the following options:
```bash
$ ./server [OPTIONS]
```

**Options:**
- `-p, --port PORT` - Server port (default: 3500)
- `-t, --threads NUM` - Number of worker threads (default: 4)
- `-i, --interval MS` - Sensor update interval in milliseconds (default: 1000)
- `-d, --device NAME` - Device name for logging (optional)
- `-h, --help` - Show help message
- `-v, --version` - Show version information

**Examples:**
```bash
$ ./server --port 8080 --threads 8
$ ./server -p 3500 -i 500
```

#### Client

There are two client implementations available: a compiled C++ client and a Python script.

##### Compiled C++ Client

The compiled client is built from [src/client.cpp](src/client.cpp) and provides a native interface to query the sensor server.

**Usage:**
```bash
$ ./client [OPTIONS]
```

**Options:**
- `-H, --host HOSTNAME` - Server hostname or IP address (default: localhost)
- `-p, --port PORT` - Server port (default: 3500)
- `-t, --temp` - Request temperature data
- `-u, --humid` - Request humidity data
- `-r, --press` - Request pressure data
- `-a, --all` - Request all sensor values (default if none specified)
- `-h, --help` - Show help message
- `-v, --version` - Show version information

**Examples:**
```bash
$ ./client                          # Request all values from localhost:3500
$ ./client -t                       # Request only temperature
$ ./client -t -u                    # Request temperature and humidity
$ ./client -p 8080 -a               # Request all from port 8080
$ ./client -H 192.168.1.100 -t      # Request temp from remote host
```

##### Python Client Script

The Python client ([envSensorClient.py](envSensorClient.py)) provides a portable alternative that requires no compilation.

**Requirements:**
- Python 3.x
- No external dependencies (uses standard library only)

**Usage:**
```bash
$ python3 envSensorClient.py [OPTIONS]
```

Or make it executable and run directly:
```bash
$ chmod +x envSensorClient.py
$ ./envSensorClient.py [OPTIONS]
```

**Options:**
- `-H, --host` - Server hostname or IP (default: localhost)
- `-p, --port` - Server port (default: 3500)
- `-t, --temp` - Request temperature data
- `-u, --humid` - Request humidity data
- `-r, --press` - Request pressure data
- `-a, --all` - Request all sensor values (default if none specified)

**Examples:**
```bash
$ python3 envSensorClient.py                          # Request all values
$ python3 envSensorClient.py -t                       # Request only temperature
$ python3 envSensorClient.py -H 192.168.1.100 -t -u   # Request temp and humidity from remote host
$ python3 envSensorClient.py --host server.local --port 8080 --all
```

#### Stress Testing

The [test_server.sh](test_server.sh) script allows you to stress test the server by running multiple concurrent client connections.

**Usage:**
```bash
$ ./test_server.sh [NUM_CLIENTS] [CLIENT_TYPE]
```

**Arguments:**
- `NUM_CLIENTS` - Number of concurrent clients to run (default: 5)
- `CLIENT_TYPE` - Which client to test: `cpp`, `python`, or `both` (default: both)

**Options:**
- `-h, --help` - Show help message

**Features:**
- Launches multiple clients concurrently with varied requests (temperature, humidity, pressure, or all)
- Tracks success/failure rates for each client
- Measures total duration and requests per second
- Saves detailed logs to `logs/test_TIMESTAMP/` directory
- Displays failed client outputs for debugging
- Small stagger between launches (50ms) to simulate realistic traffic

**Examples:**
```bash
$ ./test_server.sh                    # Test with 5 clients of each type
$ ./test_server.sh 20                 # Test with 20 clients of each type
$ ./test_server.sh 50 cpp             # Test with 50 C++ clients only
$ ./test_server.sh 100 python         # Test with 100 Python clients only
$ ./test_server.sh 200 both           # Stress test with 200 of each type
$ ./test_server.sh -h                 # Show help message
```

**Output:**
The script provides detailed statistics including:
- Total clients launched
- Success and failure counts
- Test duration
- Requests per second
- Location of log files
- Error messages from failed clients (if any)