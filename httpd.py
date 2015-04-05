#!/usr/local/bin/python

# Written by Etienne Laurin

# My first python application

import socket, sys, thread, os, re, pwd, grp

try:
	execfile(sys.path[0]+'/httpd.conf.py')
	if os.path.isfile('/etc/httpd.conf.py'):
		execfile('/etc/httpd.conf.py')
except Exception, msg:
	sys.exit('Error in config file:\n'+str(msg))

def debug(msg):
	if verbose:
		print msg

#parse command line args
i=0
for val in sys.argv[0:]:
	if val == '-v' or val == '--verbose':
		verbose = 1
	if val == '-p' or val == '--port':
		serverport=int(sys.argv[i+1])
	if val == '-h' or val == '--help':
		print 'AtnNn\'s beta python httpd'
		print 'usage: ./httpd.py [options] [stop|restart|status]'
		print '\t-v --verbose     show debug messages'
		print '\t-p --port        set custom port'
		print '\t-h --help        display this help message'
		print '\t-d --daemon      run in daemon mode'
		sys.exit(0)
	if val == '-d' or val == '--daemon':
		daemon = 1
	if val == 'stop' or val == 'restart':
		f=file(filepid)
		if not f:
			sys.exit('unable to open pid file')
		pid=f.read()
		f.close
		try:
			os.kill(int(pid),3)
		except:
			pass
		if val == 'stop':
			sys.exit('http.py was successfully stopped')
		else:
			print 'restarting server'
	if val == 'status':
		f=file(filepid)
		if not f:
			print 'server is stopped'
			sys.exit(0)
		pid=f.read()
		f.close
		try:
			f=os.readlink('/proc/'+pid+'/file')
		except:
			print 'server is stopped'
			sys.exit(0)
		if f.find('python') != -1:
			print 'server is running'
		else:
			print 'server is stopped'
		sys.exit(0)
	i+=1

#socket initialisation
try:
	server = socket.socket()
	server.bind((socket.gethostname(), serverport))
	server.listen(16)
	print "listening on port %d"%(serverport);
except socket.error, (num,msg):
	sys.exit('socket error %d (%s)' % (num,msg))

try:
	a=grp.getgrnam(servergroup)
	os.setgid(a[2])
	a=pwd.getpwnam(serveruser)
	os.setuid(a[2])
except:
	sys.exit('error while changing uid and gid of server')

if daemon:
	debug('starting daemon mode')
	if os.fork():
		sys.exit(0)
	os.setsid()

f=file(filepid,'w');
if not f:
	sys.exit('unable to open pid file')
f.write(str(os.getpid()))
f.close()

#check for needed files
if not os.path.isfile(rootdir+indexfile)\
or not os.path.isfile(rootdir+errorfile):
	sys.exit('missing index and error files in root folder')

#percenthextoint, a is '%[A-Ba-b0-9]{2}'
def pchtoi(a):
	a=a.group()
	return chr(int(a[1:],16))

#ill have to add all the default avlues for mime types here
def getcontenttype(file):
	ext=file.split('.')
	ext=ext[-1]
	if ext == 'html' or ext == 'htm':
		return 'text/html'
	if ext == 'gif' or ext == 'jpeg' or ext == 'bmp':
		return 'image/'+ext
	if ext == 'jpg':
		return 'image/jpeg'
	else : return 'text/plain'

def getfile(url):
	try:
		debug('requested url: '+url)
		ret=rootdir
		url=re.sub('%[A-Za-z0-9]{2}',pchtoi,url)
		a=url.find('://')
		if a != -1:
			url=url[a+3:]
			url=url[url.find('/')+1:]
		a=url.find('?')
		if a != -1:
			url=url[:a]
		if url[0] == '/' : url = url[1:]
		while 1:
			if url == '' : break
			a=url.find('/')
			if url[0] == '.': return rootdir+errorfile403
			if a == -1 : break
			ret += url[:a] + '/'
			if not os.access(ret,4):
				return rootdir+errorfile403
			url=url[a+1:]
		if url == '' : url = indexfile
		ret += url
		if os.path.isfile(ret) and os.access(ret,4) : return ret
		ret += '/' + indexfile
		if os.path.isfile(ret) and os.access(ret,4) : return ret
		return rootdir+errorfile404
	except IndexError:
		debug('exception: IndexError')
		return rootdir+errorfile404

def client_thread(client,addy):
	debug('new connection from %s (%d)'%addy)
	name=rootdir+indexfile
	lines = client.recv(2048)
	lines = lines.split('\n')
	for line in lines:
		if line[:3].upper() == 'GET':
			name=line[4:-10]
			name=getfile(name)
		if line == '\n':
			break
	debug('sending '+name)
	client.send(
				"HTTP/1.1 200 OK\n"+
#				"Content-Length: "+str(os.path.getsize(name))+"\n"
				"Connection: Close\n"+
				"Server: Python httpd dev version - made by AtnNn\n"+
				"Content-Type: "+getcontenttype(name)+"\n"+
#				"Date: Sun, 26 Jan 2003 22:28:55 GMT\n"+
#				"Location: http://www.google.ca/\n"+
				"\n"
				)
	if os.access(name,1):
		f=os.popen(name,"r")
	else:
		f=open(name,"rb")
	while 1:
		a=f.read(256)
		if a == '':
				break;
		client.send(a)
	client.shutdown(2)
	f.close

#int main(){ //its a bit short... :P
debug('starting main loop')
while 1:
	ret = server.accept()
	thread.start_new_thread(client_thread,ret)
