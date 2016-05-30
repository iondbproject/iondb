### Author:	Graeme Douglas
###
### Setup git hooks and other repository goodies.

import os

# Setup paths we will need.
pwd	= os.getcwd()
cfg	= os.path.join(pwd, '.git', 'hooks', 'iondb_style.cfg')
df	= os.path.join(pwd, 'dotfiles')
gh	= os.path.join(pwd, '.git', 'hooks')
files	= [
	{'copy': False, 'exec': True,  'file': 'pre-commit-uncrustify',
	 'replacements': {
		'%CONFIG_FILE%': os.path.join(gh, 'iondb_style.cfg')
	 }
	},
	{'copy': True,  'exec': True,  'file': 'pre-commit'},
	{'copy': True,  'exec': True,  'file': 'canonicalize_filename.sh'},
	{'copy': True,  'exec': False, 'file': 'iondb_style.cfg'},
]

for f in files:
	if f['copy']:
		os.system("cp " + os.path.join(df, f['file']) + " " + os.path.join(gh, f['file']))
	else:
		with open(os.path.join(gh, f['file']), "wt") as fout:
			with open(os.path.join(df, f['file']), "rt") as fin:
				for line in fin:
					for toreplace in f['replacements'].keys():
						line = line.replace(toreplace, f['replacements'][toreplace])
					fout.write(line)

for f in files:
	if f['exec']:
		os.system("chmod +x " + os.path.join(gh, f['file']))
