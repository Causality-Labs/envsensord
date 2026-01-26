#!/usr/bin/env python3

import socket
import sys
import argparse

def build_request(temp, humid, press):
    """Build sensor request string based on selected values"""
    values = []
    if temp:
        values.append("TEMP")
    if humid:
        values.append("HUMID")
    if press:
        values.append("PRESS")
    
    if not values:
        # If nothing selected, request all
        values = ["TEMP", "HUMID", "PRESS"]
    
    return ",".join(values) + ";"

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='BME280 Sensor Client')
    parser.add_argument('-H', '--host', default='localhost', 
                        help='Server hostname or IP (default: localhost)')
    parser.add_argument('-p', '--port', type=int, default=3500,
                        help='Server port (default: 3500)')
    parser.add_argument('-t', '--temp', action='store_true',
                        help='Request temperature data')
    parser.add_argument('-u', '--humid', action='store_true',
                        help='Request humidity data')
    parser.add_argument('-r', '--press', action='store_true',
                        help='Request pressure data')
    parser.add_argument('-a', '--all', action='store_true',
                        help='Request all sensor values (default if none specified)')
    
    args = parser.parse_args()
    
    # If --all is specified or nothing is specified, request everything
    if args.all or not (args.temp or args.humid or args.press):
        args.temp = args.humid = args.press = True
    
    # Build request message
    message = build_request(args.temp, args.humid, args.press)
    
    # Create a TCP/IP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        # Connect to the server
        print(f"Connecting to {args.host}:{args.port}...")
        client_socket.connect((args.host, args.port))
        print("Connected to server!")
        
        # Send sensor request to server
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
        print(f"Error: Could not connect to server at {args.host}:{args.port}")
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
