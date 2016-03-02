#!python3
import shutil
import os
import os.path
import re

def copy_dir(src, dst, *, follow_sym=True):
    if os.path.isdir(dst):
        dst = os.path.join(dst, os.path.basename(src))
    if os.path.isdir(src):
        shutil.copyfile(src, dst, follow_symlinks=follow_sym)
        shutil.copystat(src, dst, follow_symlinks=follow_sym)
    return dst

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

print("Ignoring using the following rules: {}".format(", ".join(exclude_rules)))

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

for dirpath, dirnames, filenames in os.walk(source_folder):
    for src_file in filenames:
        #If one exclusion rule is satisfied, we skip the file
        if any([re.match(each_rule, src_file) for each_rule in exclude_rules]):
            print("Skipping {}...".format(src_file))
            continue

        old_src_file_path = os.path.join(dirpath, src_file)
        new_src_directory = dirpath.replace(source_folder, destination_folder)
        new_src_file_path = old_src_file_path.replace(source_folder, destination_folder)
        print("Copying {} to {}...".format(old_src_file_path, new_src_file_path), end="")
        if not os.path.exists(new_src_directory):
            print("(With directory creation)...", end="")
            shutil.copytree(dirpath, new_src_directory, copy_function=copy_dir)
        shutil.copy(old_src_file_path, new_src_file_path)
        print("Done.")

print("Finished.")