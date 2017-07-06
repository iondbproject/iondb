/******************************************************************************/
/**
@file		kv_stdio_intercept.h
@author		Scott Fazackerley
@brief		Intercepts stdio.h defined functions for use on AVR platform.
@details	stdio.h is not fully defined within avr-gcc. As a result, functions
			for file operations are defined and not implemented or not defined.
			This means it is difficult to cross compile src that contains
			stdio.h functions. Include this header at the top of the file,
			but after stdio.h and the functions defined below will override.

			Flags:  -DIntercept

			Compiling with the -DIntercept flag will override stdio.h functions.
			Leaving the flag (and thus the functions) out will allow for
			regular use.
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#if !defined(ION_KV_STDIO_INTERCEPT_H_)
#define ION_KV_STDIO_INTERCEPT_H_

#if defined(ARDUINO)

#if defined(__cplusplus)
extern "C" {
#endif

#define  ION_FILE SD_FILE
#define  fopen(x, y)		sd_fopen(x, y)
#define  fclose(x)			sd_fclose(x)
#define  fwrite(w, x, y, z) sd_fwrite(w, x, y, z)
#define  fsetpos(x, y)		sd_fsetpos(x, y)
#define  fgetpos(x, y)		sd_fgetpos(x, y)
#define  fflush(x)			sd_fflush(x)
#define  fseek(x, y, z)		sd_fseek(x, y, z)
#define  fread(w, x, y, z)	sd_fread(w, x, y, z)
#define  feof(x)			sd_feof(x)
#define  ftell(x)			sd_ftell(x)
#define  fremove(x)			sd_remove(x)
#define  frewind(x)			sd_rewind(x)
#define  fdeleteall()		SD_File_Delete_All()
#if defined(__cplusplus)
}
#endif

#endif /* Clause ARDUINO */

#endif /* KV_STDIO_INTERCEPT_H_ */
