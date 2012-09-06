import os

def main():
	if not os.path.isfile(".manifest"):
		return
		
	f = open(".manifest")
	for l in f:
		l = l.rstrip("\r\n")
		v = l.split("\t")
		
		if v[0] == "archive-as":
			try:
				os.rename(v[2], v[1])
			except:
				print "couldn't rename: %s in %s" % (v[2], v[1])
	f.close()

if __name__ == "__main__":
	main()
	