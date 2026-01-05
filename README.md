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
## Docker
For cro