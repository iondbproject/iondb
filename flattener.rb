### Author: Graeme Douglas
###
### This code is place in the public domain.

require 'pathname'
require 'fileutils'

class FFlat
 @filters       # Rules that filter out certain files.
 @replacements      # Replacement rules. These are the parameters for gsub.
 @indir         # Input directory to flatten.
 @outdir        # Output directory to flatten.
 
 def initialize(indir, outdir)
    @filters    = Array.new
    @replacements   = Array.new
    FileUtils.mkdir_p outdir
    @indir      = Pathname.new(indir).realpath.to_s
    @outdir     = Pathname.new(outdir).realpath.to_s
 end
 
 def addFilter(pattern)
    @filters.push(pattern)
 end
 
 def checkIfFileFiltered(filePath)
    @filters.each do |filter|
        if filePath =~ filter then return true end
    end
    return false
 end
 
 def addReplacementRule(pattern, replacement)
    @replacements.push([pattern, replacement])
 end
 
 def performReplacements(str)
    toReturn    = String.new(str)
    
    @replacements.each do |replacement|
        toReturn.gsub!(replacement[0], replacement[1])
    end
    
    return toReturn
 end
 
 def addFixCIncludes
    self.addReplacementRule(\
        /#include\s*[\"](?:.*\/|)(.*\.h)\s*\"/,\
        '#include "\1"'\
    )
 end
 
 def getFileList
    currentDir  = Dir.pwd
    Dir.chdir(@indir)
    allFiles    = Dir.glob("**/*").reject do |file_path|
                pn  = Pathname.new(file_path)
                File.directory?(file_path) or\
                 self.checkIfFileFiltered(file_path) or\
                 self.checkIfFileFiltered(pn.realpath.to_s)
              end
    Dir.chdir(currentDir)
    
    return allFiles
 end

 def getFileContents(file_path)
    file        = File.open(file_path)
    contents    = file.read
    file.close
    return contents
 end
 
 def flatten
    currentDir  = Dir.pwd
    
    allFiles    = self.getFileList
    
    Dir.chdir(@indir)
    allFiles.each do |file_path|
        pn  = Pathname.new(file_path)
        fqpath  = pn.realpath.to_s
        contents= self.getFileContents(fqpath)
        contents= self.performReplacements(contents)
        File.open(File.join(@outdir, File.basename(pn)), 'w') do |file|
            file.write(contents)
        end
    end
    
    Dir.chdir(currentDir)
 end
end

if __FILE__ == $0
    flattener   = FFlat.new('./src/', 'bin/flatten/')

    flattener.addFilter(/CuTest(\.c|\.h)/)
    flattener.addFilter(/run_\w+\.c/)
    flattener.addFilter(/test_\w+\.c/)
    flattener.addFilter(/sltests\.h/)

    flattener.addFilter(/ramutil(\.c|\.h)/)
    flattener.addFilter(/lfsr(\.c|\.h)/)
    flattener.addFilter(/lfsr_example\.c/)
    flattener.addFilter(/hashmap\.c/)
    flattener.addFilter(/skiplist\.c/)
    flattener.addFilter(/ionbench.*/)
    flattener.addFilter(/benchmark(\.c|\.h)/)

    flattener.addFixCIncludes
=begin
    flattener.addReplacementRule(\
        /(?:.*\/|)(.*\.h)\s*\"/,\
        '#include "\1"'\
    )
=end
    #p flattener.performReplacements('#include "this/is/a/test.h"')
    #p flattener.getFileList
    flattener.flatten
end