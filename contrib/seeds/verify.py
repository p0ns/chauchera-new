import socket
from binascii import a2b_hex, b2a_hex
import re

DEBUG = False

version = 'aaa226a9' + \
	'76657273696f6e0000000000' + \
	'670000001877cead7f110100' + \
	'0d00000000000000b7b4075d' + \
	'00000000000000000000000000000000000000000000ffffc9eec5ba407f0d' + \
	'0000000000000000000000000000000000000000000000000020f12d9e1133e60d' + \
	'112f4368617563686572613a322e312e312f' + \
	'f4be0c00' + \
	'01'

with open('contrib/seeds/nodes_main.txt', 'r') as f:
	for line in f.readlines():
		HOST, PORT = line.strip().split(":")

		with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
			s.settimeout(1)

			try:
				s.connect((HOST, int(PORT)))
				s.sendall(a2b_hex(version))
				data = repr(s.recv(1024))

				if len(data) > 0:
					agent = data[data.find('/') + 1:]
					agent = agent[:agent.find('/')]
					print(agent, HOST)

			except Exception as error:
				if DEBUG:
					print('%s: %s' % (HOST, error))

			finally:
				s.close()