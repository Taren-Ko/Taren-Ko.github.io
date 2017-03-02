import sys
from socket import *

serverPort = int(sys.argv[1])
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('',serverPort))
serverSocket.listen(1)

def mail_time():

	fromboolean = False
	toboolean = False
	databoolean = False

	while fromboolean == False:
		mailSet = connectionSocket.recv(1024).decode()
		connectionSocket.send(mailSet.encode())

		if mailSet == "QUIT":
			return 0

		test1 = mail_from_test(mailSet)
		if test1 != "250 OK":
			if rcpt_test(mailSet) == True or termination_test(mailSet) == True or data_cmd_test(mailSet) == True:
					connectionSocket.send("503 Bad sequence of commands".encode())
			else:
				connectionSocket.send(test1.encode())
		else:
			connectionSocket.send(test1.encode())
			fromboolean = True

	i = 0
	receivers = [None] * 1024
	while toboolean == False:
		toSet = connectionSocket.recv(1024).decode()
		connectionSocket.send(toSet.encode())

		if toSet == "QUIT":
			return 0

		test2 = rcpt_to_test(toSet)
		if test2 != "250 OK":
			if i == 0:
				if mail_test(toSet) == True or termination_test(toSet) == True or data_cmd_test(toSet) == True:
					connectionSocket.send("503 Bad sequence of commands".encode())
				else:
					connectionSocket.send(test2.encode())

			if mail_test(toSet) == True or termination_test(toSet) == True:
				connectionSocket.send("503 Bad sequence of commands".encode())
			elif data_cmd_test(toSet) == True:
				connectionSocket.send("354 Start mail input; end with <CRLF>.<CRLF>".encode())
				toboolean = True
			else:
				connectionSocket.send(test2.encode())
		else:
			connectionSocket.send(test2.encode())
			receivers[i] = recpient_puller(toSet)
			i = i+1

	j = 0
	data = [None] * 1024
	while databoolean == False:
		dataSet = connectionSocket.recv(1024).decode()
		# print(dataSet)
		connectionSocket.send(dataSet.encode())

		if mailSet == "QUIT":
			return 0

		if termination_test(dataSet) == True:
			connectionSocket.send("250 OK".encode())
			databoolean = True
		else:
			data[j] = dataSet
			j = j+1

	# print("Now we're ready!")

	"""Now here's where things start getting hairy"""
	"""This code below pulls out the receivers, creates a file based on the reciever, then writes the data to the file"""

	k = 0
	while k != i:
		addr = "forward/" + receivers[k]
		target = open(addr, 'w')

		# fromMsg = "From: <" + recpient_puller(mailSet) + ">\n"
		# target.write(fromMsg)
		#
		# m = 0
		# while m != i:
		# 	toMsg = "To: <" + receivers[m] + ">\n"
		# 	target.write(toMsg)
		# 	m = m+1

		l = 0
		while l != j:
			dataLine = data[l] + "\n"
			target.write(dataLine)
			l = l+1

		target.close()
		k = k+1

	return 1




def mail_from_test(str):
		"""Runs the proper commands for doing a full test of the mail from command"""
		#mail_from = str.upper()
		mail_from = str

		if mail_test(mail_from) == False:
			return "500 Syntax error: command unrecognized"
		elif mail_addr_test(mail_from) == False:
			return "501 Syntax error in parameters or arguments"
		elif mailbox_test(mail_from) == False:
			return "501 Syntax error in parameters or arguments"
		else:
			return "250 OK"


def rcpt_to_test(str):
	"""Runs the proper commands for doing a full test of the rcpt to command"""
	mail_to = str.upper()

	if rcpt_test(mail_to) == False:
		return "500 Syntax error: command unrecognized"
	elif mail_addr_test(mail_to) == False:
		return "501 Syntax error in parameters or arguments"
	elif mailbox_test(mail_to) == False:
		return "501 Syntax error in parameters or arguments"
	else:
		return "250 OK"


#------------------------------------------------------------------------------------------------------


def mail_test(str):
	"""Tests to see if the line starts with MAIL FROM (loose case)"""
	testcase = str[0:5]
	testcase2 = str.replace(" ", "")

	#testcase = testcase.upper()
	#testcase2 = testcase2.upper()

	if testcase == "MAIL ":
		if testcase2[4:9] == "FROM:":
			return True
		else:
			return False
	else:
		return False

def mail_addr_test(str):
	"""Tests for a bracketed mailing address"""
	testcase = str
	start = str.find('<')
	end = str.find('>')
	if start==-1 or end==-1:
		return False
	else:
		testcase2 = testcase[start+1:end]

		if testcase2.find('<') != -1 or testcase2.find('>') != -1:
			return False
		elif testcase2.find(' ') != -1:
			return False
		else:
			return True

def mailbox_test(str):
	"""Tests if the address inside brackets is valid"""
	testcase = str
	start = str.find('<')
	end = str.find('>')
	testcase = testcase[start+1:end]

	at_test = testcase.find('@')
	if at_test == -1:

		return False
	else:

		"""
		This is code that was removed for being too strict for the grader in HW1. I'm keeping it here just in case I need it later.
		"""
		testcase2 = testcase[0:at_test]
		testcase3 = testcase[at_test+1:]
		if testcase2.isalnum() == False:
			return False
		if testcase3.find('@') != -1 or testcase3.find(' ') != -1:
			return False



		return True

#Work below here is about the RCPT Command-------------------------------------------------------------

def rcpt_test(str):
	"""Tests to see if the line starts with RCPT TO: (loose case)"""
	testcase = str[0:5]
	testcase2 = str.replace(" ", "")

	#testcase = testcase.upper()
	#testcase2 = testcase2.upper()

	if testcase == "RCPT ":
		if testcase2[4:7] == "TO:":
			return True
		else:
			return False
	else:
		return False

#After this perform the mailbox_addr_test and mailbox_test as the formatting is the same
#Need to add testing to make sure only a nullspace between : and <forward-path>

#Work below here is about the DATA command-------------------------------------------------------------

def data_cmd_test(str):
	testcase = str.upper()
	if testcase == "DATA":
		return True
	else:
		return False


#Work below here is about the terminate data command---------------------------------------------------

def termination_test(str):
	if str == ".":
		return True
	else:
		return False

#Here we're going to pull recipients out of RCPT TO: commands for writing purposes---------------------
def recpient_puller(str):
	testcase = str
	start = str.find('<')
	end = str.find('>')
	testcase2 = testcase[start+1:end]
	return testcase2

#Below here is the actual command that is run----------------------------------------------------------

while True:
	try:
		connectionSocket, addr = serverSocket.accept()
		connectionSocket.send("You have accessed Cameron's SMTP Server".encode())
		confirmation = connectionSocket.recv(1024).decode()
		# print(confirmation)
		if confirmation[0:4] == "HELO":
			connectionSocket.send("250 OK".encode())
			result = mail_time()
			while result!=0:
				result = mail_time()

			connectionSocket.close()
		else:
			connectionSocket.close()
	except EOFError:
		break
