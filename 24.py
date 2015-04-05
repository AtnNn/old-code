# Written by Etienne Laurin

# Use + - * / on 4 playing cards (ace = 1, jack = 11, queen = 12, king = 13)
# to get the number 24

import sys
if len(sys.argv) != 5:
	sys.exit('usage: python 24.py num1 num2 num3 num4\n')
def tf(n,rl=[]):
	#print n,rl
	if len(n) == 1:
		if n[0]>23.99 and n[0]<24.01:
			print rl
		return
	l=rl[:]
	l.append(0)
	l.append(0)
	l.append('.')
	for i in n:
		m=n[:]
		m.remove(i)
		l[-2]=i
		for j in m:
			o=m[:]
			o.remove(j)
			l[-3]=j
			o.append(j+i)
			l[-1]='+'
			tf(o,l)
			o[-1]=j-i
			l[-1]='-'
			tf(o,l)
			if i:
				o[-1]=j/i
				l[-1]='/'
				tf(o,l)
			o[-1]=j*i
			l[-1]='*'
			tf(o,l)
l=[]
for a in sys.argv[1:]:
	l.append(float(a))
tf(l)
