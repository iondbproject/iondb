import os
import stat
import subprocess

executable = stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH
for filename in os.listdir('.'):
	if os.path.isfile(filename):
		st = os.stat(filename)
		mode = st.st_mode
		if mode & executable:
			#print(filename,oct(mode))
			command	= './'+filename
			output	= subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)\
					.stdout.read()
			print(output)
