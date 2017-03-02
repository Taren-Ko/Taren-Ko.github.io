import sys
from socket import *

def mail_reader():
	# Tests for proper number of arguments passed
	# Then passes arguments to hostname and port for Socket
	if len(sys.argv) != 3:
		sys.stderr.write("Error: Too many or too few arguments.\n")
		sys.stderr.write("QUIT\n")
		sys.exit(0)
	else:
		hostname = sys.argv[1]
		port = int(sys.argv[2])

	# Asks for From Address and stores it if properly formatted
		print("Enter From")
		while True:
			from_piece = raw_input()
			if mail_addr_test(from_piece) == False:
				print("Invalid Address, try again.")
			else:
				break
	# Same as the last one but asks for Recipients.
	# A comma separated list will be split up and placed into an array
	# If any address is invalid we start over from the top of Enter Recipients
		print("Enter recipients")
		while True:
			to_piece = raw_input()
			to_set = to_piece.split(',')
			to_total = len(to_set)
			i = 0
			to_bool = False
			while i < to_total:
				if mail_addr_test(to_set[i]) == False:
					print("Invalid Address, try again.")
					break
				elif i == to_total-1:
					to_bool = True
					i = i+1
				else:
					i = i+1
			if to_bool == True:
				break

	# Same as the last two but can not be invalid
		print("Enter subject")
		subject_piece = raw_input()

	# Takes in the message until a <CRLF>.<CRLF>
	# Stores these into data[]
		print("Enter message. Terminate with a period on an empty line.")
		i = 0
		data = [None] * 1024
		dataLen = 0
		while True:
			dataset = raw_input()
			if dataset == ".":
				dataLen = i
				break
			else:
				data[i] = dataset
				i = i+1
		msglines = i

	# From here the user has finished input so we start the socket to the Server
		clientSocket = socket(AF_INET, SOCK_STREAM)
		clientSocket.connect((hostname, port))

	# confirming response from server then sending a friendly HELO
		response = clientSocket.recv(1024).decode()
		# print(response)
		if response == None:
			clientSocket.close()
			print("Error: Invalid hostname or port.")
			sys.exit(0)
		else:
			clientSocket.send("HELO <cs.unc.edu>".encode())
		response = clientSocket.recv(1024)

	# Package the from response into a valid SMTP command and send to server
		fromMsg = "MAIL FROM: <" + from_piece + ">"
		clientSocket.send(fromMsg.encode())
		response = clientSocket.recv(1024)
		response = clientSocket.recv(1024).decode()
		# print(response)
		if response != "250 OK":
			clientSocket.close()
			print("Connection encountered SMTP Error: MAIL FROM")
			sys.exit(0)

	# Ditto for the recipients
		i=0
		while i<to_total:
			toMsg = "RCPT TO: <" + to_set[i] + ">"
			clientSocket.send(toMsg.encode())
			response = clientSocket.recv(1024)
			response = clientSocket.recv(1024).decode()
			# print(response)
			if response != "250 OK":
				clientSocket.close()
				print("Connection encountered SMTP Error: RCPT TO")
				sys.exit(0)
			else:
				i= i+1

	# DATA transmission begins. Start by formatting properly with From: To: Subject: \n
	# After this send a part of data[], receive it back,
	# repeat until finished then send the terminating character.
		# response = clientSocket.recv(1024).decode()
		clientSocket.send("DATA".encode())
		response = clientSocket.recv(1024)
		response = clientSocket.recv(1024).decode()
		# print(response)
		if response[0:3] != "354":
			clientSocket.close()
			print("Connection encountered SMTP Error: DATA")
			sys.exit(0)

		fromMsg = "From: " + from_piece
		clientSocket.send(fromMsg.encode())
		response = clientSocket.recv(1024)
		# print(response)

		toMsg = "To: " + to_piece
		clientSocket.send(toMsg.encode())
		response = clientSocket.recv(1024)
		# print(response)

		subMsg = "Subject: " + subject_piece
		clientSocket.send(subMsg.encode())
		response = clientSocket.recv(1024)
		# print(response)

		clientSocket.send(" ".encode())
		response = clientSocket.recv(1024)
		# print(response)

		i=0
		while i<dataLen:
			# print(data[i])
			clientSocket.send(data[i].encode())
			response = clientSocket.recv(1024)
			# print(response)
			i= i+1

		clientSocket.send(".".encode())
		response = clientSocket.recv(1024)
		response = clientSocket.recv(1024).decode()
		# print(response)
		# Final response received, if it's a 250 OK then we are done and exit
		if response != "250 OK":
			clientSocket.close()
			print("Connection encountered SMTP Error: Data Termination")
			sys.exit(0)

		clientSocket.send("QUIT".encode())
		clientSocket.close()
		sys.exit(0)






def mail_addr_test(str):
	"""Tests for a mailing address with a singular @ symbol"""
	testcase = str


	if testcase.find('<') != -1 or testcase.find('>') != -1:
		return False
	if testcase.find(' ') != -1:
		return False
	if testcase.find('@') == -1:
		return False
	else:
		endpt = testcase.find('@')
		if testcase[endpt+1:].find('@') != -1:
			return False
		else:
			return True


while True:
	try:
		mail_reader()
	except EOFError:
		break
