import serial
import time

def get_board_port():
    """
    Function to search for open COM port with board
    :return: value of com port or none
    """
    windows_ports = ['COM%s' % (i + 1) for i in range(1, 256)]
    open_ports = []

    for port in windows_ports:
        try:
            s = serial.Serial(port)
            s.close()
            open_ports.append(port)
        except(OSError, serial.SerialException):
            pass

    for port in open_ports:
        s = serial.Serial(port,115200,timeout=1)
        string = s.readline()
        s.close()
        print(f'{port}: {string}')
        if string == b'Board Checking In\r\n':
            print(f"Connected to Port: {port}")
            return port

    return None


def transmit_packet(serial_port, data):
    """
    Ensures transmission of packet to board using 3 way handshake
    :return: None
    """

    # Loop to ensure packet transmits
    while True:
        # Wait for board to check in
        while serial_port.inWaiting() == 0:
            pass

        # Read out the incoming message in case it is needed later
        incoming_message = serial_port.readline()
        if incoming_message != b'Board Checking In\r\n':
            # print(f"Received invalid checkin!{incoming_message}")
            continue

        # Flush input, write data, then flush to ensure data is written
        serial_port.reset_input_buffer()
        serial_port.write(data)
        serial_port.flush()
        # print(f'Command Data: {output_bytes}')
        # print('Command Sent')

        # Wait for board to respond with ack
        while serial_port.inWaiting() == 0:
            pass

        # Read ack
        ack_message = serial_port.readline()
        # print(f'ACK Message: {ack_message}')

        # Continue to try again if failure
        if ack_message == b'Board Checking In\r\n':
            continue

        # If ack is equal to the command sent, message was receive so go back to menu
        if ack_message[:-2] == data:
            print('Command acknowledged!')
            break


if __name__ == "__main__":
    board_port = None

    # Main Loop of Program

    while board_port is None:
        board_port = get_board_port()

    s = serial.Serial(board_port, 115200, timeout=0)
    while True:
        print("Change Orientation or Color?")
        command = input()

        if command == 'C' or command == 'Color' or command == 'c':
            output_bytes = b'C'
        elif command == 'O' or command == "Orientation" or command == 'o':
            output_bytes = b'O'

        # Skip rest of program if invalid command
        else:
            print("Invalid command!")
            continue

        transmit_packet(s, output_bytes)

        # Loop to ensure command was received





