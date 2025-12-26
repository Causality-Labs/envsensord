# A Quick Docker Tutorial

1. Build the container
``` bash
$ docker build -t my-server .
```

2. Run the container in the background 
```bash
# Pattern:
# docker run -d -p 3500:3500 --name [container-name] [image-name]
docker run -d -p 3500:3500 --name my-server my-server
```
- The -d flag (short for --detach) runs the container in the background.
- The -p flag (short for --publish) creates a port mapping between the host and the container.

3. You can verify your container is running with the following command:
```bash
$ docker ps
CONTAINER ID   IMAGE       COMMAND          CREATED         STATUS         PORTS                                         NAMES
0d4bfd75ba8e   my-server   "./bin/server"   3 seconds ago   Up 3 seconds   0.0.0.0:3500->3500/tcp, [::]:3500->3500/tcp   my-server
```

4. Open a terminal and obsever some logs, these logs are what the running program
prints to stdout and stderr.
```bash
docker logs -f my-server
```

5. Because you mapped port 3500 of the container to port 3500 of our host we can use clients on the host to send data to the server running in the container.
Open a new terminal and run one of the client programs and obsever the output
```bash
$ ./bin/client
[HW_Client] [INFO] Connecting to server...
[HW_Client] [INFO] Connected to server!
[HW_Client] [INFO] Sending: Hello from C++ client!
[HW_Client] [INFO] Server response: Hello from C++ client!
[HW_Client] [INFO] Disconnected from server

# In your do container you will see the following logs too
$ docker logs -f my-server
[HW_Server] [INFO] Server listening on port 3500...
[HW_Server] [INFO] Client connected
[HW_Server] [INFO] Received: Hello from C++ client!
[HW_Server] [INFO] Response sent to client
```

6. This command stops the container
```bash
docker stop my-server
```

7. You can also enter the container via interactive mode and run the test script manually
```bash
# Start the container if its not running already:
$ docker start my-server

# Enter a shell
$ docker exec -it my-server /bin/bash

# Check running proccesses in container:
root@799f684fc374:/app# ps aux
USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
root           1  0.0  0.0   6368  3584 ?        Ss   20:01   0:00 ./bin/server
root           7  0.0  0.0   4588  3968 pts/0    Ss   20:02   0:00 /bin/bash
root          18  0.0  0.0   7888  4096 pts/0    R+   20:03   0:00 ps aux
```
This gives you a bash shell inside the running container. You can then run commands like:
```bash
./test_server.sh
```
Exit the container shell with `exit`.

8. To remove a container the command
```bash
$ docker rm my-server
```
