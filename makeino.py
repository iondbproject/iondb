#!python3
import shutil
import os
import os.path
import re

###
#Config
###
ignore_file = ".makeinoignore"
source_folder = os.path.join("src")
destination_folder = os.path.join("bin", "ion_deployment", "src")

exclude_rules = []
try:
    with open(ignore_file, "r") as exclude_f:
        exclude_rules = [fn.strip() for fn in exclude_f.readlines() if not fn.startswith(";") and not fn.isspace()]
except FileNotFoundError:
    print("Creating {}...".format(ignore_file), end="")
    print("Done.")
    open(ignore_file, "x").close()

print("Ignoring the following rules: {}".format(", ".join(exclude_rules)))

###
#Process
###
if os.path.isdir(destination_folder):
    print("Cleaning old mirror...", end="")
    shutil.rmtree(destination_folder)
    print("Done.")

print("Making directories for {}...".format(destination_folder), end="")
os.makedirs(destination_folder)
print("Done.")

for src_file in os.listdir(source_folder):
    #If one exclusion rule is satisfied, we skip the file
    if any([re.match(each_rule, src_file) for each_rule in exclude_rules]):
        print("Skipping {}...".format(src_file))
        continue

    old_src_file_path = os.path.join(source_folder, src_file)
    new_src_file_path = os.path.join(destination_folder, src_file)

    print("Copying {} to {}...".format(old_src_file_path, new_src_file_path), end="")
    shutil.copy(old_src_file_path, new_src_file_path)
    print("Done.")

print("Finished.")