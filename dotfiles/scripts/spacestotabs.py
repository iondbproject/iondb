from __future__ import print_function
import re
import sys

def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

## Options #####################################################################
tabsize		= 4
mode		= "totabs"
################################################################################

spaces		= tabsize*" "
cur		= 0
newstr		= ""

string		= 'a         "    heeeeeeyyyyeah    "'
if len(sys.argv) > 1:
	try:
		with open(sys.argv[1], 'r') as content_file:
			string = content_file.read()
	except:
		eprint("Could not read file: " + sys.argv[1])
		sys.exit()
else:
	eprint("No file specified")
	sys.exit()

def bound_in(bound, bounds):
	for check in bounds:
		if bound[0] >= check[0] and bound[1] <= check[1]:
			return True
	return False

if "totabsall" == mode:
	newstr	= string.replace(spaces, "\t")
elif "totabs" == mode:
	# Get the position of the string data.
	regex	= re.compile(r'"(?:\\.|\\\n|[^"\\])*"')
	strings	= [[m.start(), m.end()] for m in regex.finditer(string)]
	regex	= re.compile(spaces)
	for m in regex.finditer(string):
		# Skip over spaces in strings.
		if bound_in([m.start(), m.end()], strings):
			continue
		newstr	+= string[cur:m.start()]
		newstr	+= '\t'
		cur	= m.end()
	if cur < len(string):
		newstr	+= string[cur:]
		
if len(sys.argv) > 2:
	try:
		with open(sys.argv[2], 'w+') as content_file:
			string = content_file.write(newstr)
	except:
		eprint("Could not write to file: " + sys.argv[1])
		sys.exit()
else:
	print(newstr)
