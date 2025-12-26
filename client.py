#!/usr/bin/env python3

import socket
import sys

def main():
    # Server configuration
    HOST = 'localhost'
    PORT = 3500
    
    # Create a TCP/IP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        # Connect to the server
        print(f"Connecting to {HOST}:{PORT}...")
        client_socket.connect((HOST, PORT))
        print("Connected to server!")
        
        # Send message to server
        message = "Hello from Python client!"
        print(f"Sending: {message}")
        client_socket.sendall(message.encode('utf-8'))
        
        # Receive response from server
        data = client_socket.recv(1024)
        
        if data:
            response = data.decode('utf-8')
            print(f"Server response: {response}")
        else:
            print("Server closed connection")
            
    except ConnectionRefusedError:
        print(f"Error: Could not connect to server at {HOST}:{PORT}")
        print("Make sure the server is running.")
        sys.exit(1)
        
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
        
    finally:
        # Clean up
        client_socket.close()
        print("Disconnected from server")

if __name__ == "__main__":
    main()
