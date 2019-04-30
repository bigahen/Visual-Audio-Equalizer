import serial
import time

def get_board_port():
    windows_ports = ['COM%s' % (i + 1) for i in range(4,256)]
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


if __name__ == "__main__":

    board_port = None

    # Main Loop of Program

    while board_port is None:
        board_port = get_board_port()

    s = serial.Serial(board_port, 115200, timeout=0)
    while True:
        print("Change Orientation or Color?\r\n")
        command = input()

        if command == 'C' or command == 'Color' or command == 'c':
            output_bytes = b'C'
        elif command == 'O' or command == "Orientation" or command == 'o':
            output_bytes = b'O'

        # Skip rest of program if invalid command
        else:
            print("Invalid command!")
            continue

        #output_bytes = bytes(command_phrase, 'utf-8')

        # Loop to ensure command was received
        while True:
            # Wait for board to check in
            while s.inWaiting() == 0:
                pass

            # Read out the incoming message in case it is needed later
            incoming_message = s.readline()
            if incoming_message != b'Board Checking In\r\n':
                #print(f"Received invalid checkin!{incoming_message}")
                continue

            #

            # Create command phrase
            s.reset_input_buffer()
            s.write(output_bytes)
            s.flush()
            #print(f'Command Data: {output_bytes}')
            #print('Command Sent')

            # Wait for board to respond with ack
            while s.inWaiting() == 0:
                pass

            # Read ack
            ack_message = s.readline()
            #print(f'ACK Message: {ack_message}')
            # Break if message failed
            if ack_message == b'Board Checking In\r\n':
                continue


            # If ack is equal to the command sent, message was receive so go back to menu
            if ack_message[:-2] == output_bytes:
                print('Command acknowledged!')
                break

        #s.close()




