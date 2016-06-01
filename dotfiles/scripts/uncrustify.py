import platform
import os
import subprocess

# First, we build a list of files.
files		= ''
command		= "find . -type f -name '*.[c\|cpp\|h]' -print"
if 'windows' in platform.system().lower():
	command = "dir /b /s *.c *.h *.cpp"

files		= subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)\
			.stdout.read()

print files
for fname in files.split():
	print fname
	os.system("uncrustify -c ../iondb_style.cfg --replace --no-backup " + fname)
