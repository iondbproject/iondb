#!/bin/sh
# Copyright (c) 2015, David Martin
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:

#* Redistributions of source code must retain the above copyright notice, this
#  list of conditions and the following disclaimer.
#
#* Redistributions in binary form must reproduce the above copyright notice,
#  this list of conditions and the following disclaimer in the documentation
#  and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# Make sure this script is executable. Bypass hooks with git commit --no-verify.

# Provide the canonicalize filename (physical filename with out any symlinks)
# like the GNU version readlink with the -f option regardless of the version of
# readlink (GNU or BSD).

# This file is part of a set of unofficial pre-commit hooks available
# at github.
# Link:    https://github.com/githubbrowser/Pre-commit-hooks
# Contact: David Martin, david.martin.mailbox@googlemail.com

###########################################################
# There should be no need to change anything below this line.

# Canonicalize by recursively following every symlink in every component of the
# specified filename.  This should reproduce the results of the GNU version of
# readlink with the -f option.
#
# Reference: http://stackoverflow.com/questions/1055671/how-can-i-get-the-behavior-of-gnus-readlink-f-on-a-mac
canonicalize_filename () {
    local target_file="$1"
    local physical_directory=""
    local result=""

    # Need to restore the working directory after work.
    local working_dir="`pwd`"

    cd -- "$(dirname -- "$target_file")"
    target_file="$(basename -- "$target_file")"

    # Iterate down a (possible) chain of symlinks
    while [ -L "$target_file" ]
    do
        target_file="$(readlink -- "$target_file")"
        cd -- "$(dirname -- "$target_file")"
        target_file="$(basename -- "$target_file")"
    done

    # Compute the canonicalized name by finding the physical path
    # for the directory we're in and appending the target file.
    physical_directory="`pwd -P`"
    result="$physical_directory/$target_file"

    # restore the working directory after work.
    cd -- "$working_dir"

    echo "$result"
}
