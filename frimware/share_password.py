import socket
import time

# ESP8266 IP address and port
ESP_IP = '192.168.4.1'
ESP_PORT = 20002

# Wi-Fi credentials to be sent to the ESP8266
ssid = "loop"
password = "cyber@123"

def send_wifi_credentials(ssid, password):
    try:
        # Create a TCP/IP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Connect to ESP8266
        print(f"Connecting to {ESP_IP}:{ESP_PORT}...")
        sock.connect((ESP_IP, ESP_PORT))
        print("Connected to ESP8266")

        # Receive the initial request from ESP8266 (CRED-REQ)
        response = sock.recv(1024).decode('utf-8')
        print(f"Received: {response}")
        
        if response == "CRED-REQ":
            print("Sending Wi-Fi credentials...")
            
            # Send the Wi-Fi credentials (SSID and password)
            wifi_data = f"{ssid}\n{password}"
            sock.sendall(wifi_data.encode('utf-8'))
            time.sleep(1)  # small delay before sending the END signal
            
            # Receive acknowledgment (if any)
            response = sock.recv(1024).decode('utf-8')
            print(f"Received: {response}")
            
            if response == "END":
                # Send the END signal back to complete the process
                sock.sendall("END".encode('utf-8'))
                print("Sent END signal")
        
        # Close the connection
        sock.close()
        print("Connection closed")
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if sock:
            sock.close()

if __name__ == "__main__":
    send_wifi_credentials(ssid, password)
