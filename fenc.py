#!/usr/local/bin/python

# Written by Etienne Laurin

# Simple xor encryption + random mask

from sys import argv,exit
from os import write
from random import randint
if len(argv) != 3:
	exit("usage: ./fenc.py (-d|-e) file")
password=raw_input();
data=file(argv[2])
def encrypt(f,p):
	r=[]
	i=0
	for c in p:
		r.append(randint(0,255))
		write(1,chr(r[-1]^ord(c)))
	while 1:
		c=f.read(1)
		if not len(c):
			return
		write(1,chr(ord(c)^r[i%len(p)]))
		i=i+1
def decrypt(f,p):
	r=[]
	i=0
	for c in p:
		r.append(ord(f.read(1))^ord(c))
	while 1:
		c=f.read(1)
		if not len(c):
			return
		write(1,chr(ord(c)^r[i%len(p)]))
		i=i+1
if argv[1] == "-e":
	encrypt(data,password)
elif argv[1] == "-d":
	decrypt(data,password)
else:
	exit("uknown option")
