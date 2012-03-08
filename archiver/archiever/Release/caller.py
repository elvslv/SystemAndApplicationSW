import sys
import os
import time
import subprocess
import filecmp

def main():
	args = sys.argv[1:]
	files = []
	compress = '-c' in args;
	archive = ''
	for i, arg in enumerate(args):
		if arg == '-in' or arg == '-dir':
			if i + 1 >= len(args):
				print "Invalid commands"
			if (arg == '-in' and not os.path.isfile(args[i + 1])) or (arg == '-dir' and not os.path.isdir(args[i + 1])):
				print ("Input file doesn't exist" if arg == '-in' else "Output directory doesn't exist")
				return
			if arg == '-in':
				if compress:
					files.append(args[i + 1])
				else:
					archive = args[i + 1]

	if compress:
		source_size = sum(os.path.getsize(file) for file in files)
	else:
		source_size = os.path.getsize(archive)

	start = time.time()
	subprocess.call(args)
	minutes = (time.time() - start) / 60
	print 'Time spent: %.2fs' % ((time.time() - start))
	print 'Speed: %.2f MB/min' % ((float(source_size) / (1024 ** 2)) / minutes)

	
if __name__ == '__main__':
	main()