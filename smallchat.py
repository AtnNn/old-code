import socket
import select
import sys
import re
class Server:
	"""Manages connections for server apps"""
	CONNECT=0
	RECEIVE=1
	INPUT=2
	DISCONNECT=3
	def __init__(self,port):
		self.sockets = {} #{socket: [host port tag]}
		self.events={}
		self.server=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		self.server.bind((socket.gethostname(),port))
		self.server.listen(socket.SOMAXCONN)
		self.server.setblocking(0)
	def connect(self,host,port,tag):
		s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		try:
			s.connect((host,port))
		except socket.error,msg:
			return msg
		s.setblocking(0)
		self.sockets[s]=[host, port, tag]
		self.do(self.CONNECT)(s)
		return s
	def disconnect(self,so):
		del self.sockets[so]
		so.shutdown(2)
	def tag(self,s,tag=None):
		if not tag:
			return self.sockets[s][2]
		self.sockets[s][2]=tag
	def on(self,event,sub=None):
		self.events[event]=sub
	def do(self,event):
		if self.events.has_key(event):
			return self.events[event]
		else:
			return lambda *x:None
	def addr(self,s):
		return self.sockets[s][:2]
	def end(self):
		self.doloop=0
	def listen(self):
		self.doloop=1
		while self.doloop:
			#try:
			read=select.select(self.sockets.keys()+[self.server,sys.stdin],[],self.sockets.keys())
			ex=read[2]
			read=read[0]
			#except:
			#	return -1
			for so in ex:
				self.disconnect(so)
			for s in read:
				if s == self.server:
					(new,(host,port))=s.accept();
					new.setblocking(0)
					self.sockets[new]=[host,port,None]
					self.do(self.CONNECT)(new)
				elif s == sys.stdin:
					self.do(self.INPUT)(raw_input())
				else:
					disc = 0
					buf = ""
					try:
						while 1:
							a = s.recv(256)
							if len(a) == 0:
								disc = 1
								raise
							buf = buf + a
					except: pass
					if disc:
						self.do(self.disconnect)(s)
						del self.sockets[s]
						continue
					else:
						self.do(self.RECEIVE)(s,buf)
		return
cut="\x1B[2K\x1B[100D"
s=Server(6668)
def onconnect(a):
	msg=cut+"*** new conection from "+a.getpeername()[0]+"\n"
	s.tag(a,a.getpeername()[0])
	for c in s.sockets.keys():
		if c != a:
			c.send(msg)
	print msg,
def chomp(a):
	while a[-1]=="\n" or a[-1]=="\r":
		a=a[:-1]
	return a
def onreceive(a,b):
	if re.match('[^ -~]',chomp(b)):
		a.send(cut+"*** error: contains invalid characters\n")
		return
	if b.find('/')==0:
		c=b[1:].split()
		if len(c)<2:
			a.send(cut+"*** error: '%s' is invalid\n"%(chomp(b)))
			return
		if c[0]=='nick':
			if len(c[1])>16:
				a.send(cut+"*** error: nick to long\n")
				return
			can=1
			n=c[1].capitalize()
			for d in s.sockets.keys():
				if s.tag(d).capitalize()==n:
					can=0
			if not can:
				a.send(cut+"*** error: nick already in use\n")
				return
			msg=cut+"*** %s changed nick to %s"%(s.tag(a),chomp(c))+"\n"
			s.tag(a,c[1])
		elif c[0]=='quit':
			msg=cut+"*** %s has quit (%s)"%(s.tag(a),b[5:-1])+"\n"
			s.disconnect(a)
		elif c[0]=='me':
			msg=cut+"* %s "%(s.tag(a))+b[4:]
		else:
			a.send(cut+"*** error: '%s' unknown command\n"%(c[0]))
			return
	else:
		msg=cut+"\x1B[2K<%s> "%(s.tag(a))+b
	for c in s.sockets.keys():
		if c != a:
			c.send(msg)
		else:
			c.send("\x1B[A"+msg)
	print msg,
def ondisconnect(a):
	msg=cut+"*** "+s.tag(a,'nick')+" (%s) "%(a.getpeername()[0])+" disconnected\n"
	for c in s.sockets.keys():
		if c != a:
			c.send(msg)
	print msg,
s.on(s.CONNECT,onconnect)
s.on(s.RECEIVE,onreceive)
s.on(s.DISCONNECT,ondisconnect)
s.listen()
