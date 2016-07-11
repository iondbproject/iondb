/******************************************************************************/
/**
@file
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
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
*/
/******************************************************************************/

#if !defined(KV_STDIO_INTERCEPT_H_)
#define KV_STDIO_INTERCEPT_H_

#if defined(ARDUINO)

#if defined(__cplusplus)
extern "C" {
#endif

/* @todo requires testing on previous versions of avr-gcc */
#if __GNUC__ > 4 || \
	(__GNUC__ == 4 && (__GNUC_MINOR__ > 8 || \
					   (__GNUC_MINOR__ == 1)))
/* Stub */
#else

typedef long fpos_t;

#endif

#define  FILE SD_FILE
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

#if defined(__cplusplus)
}
#endif

#endif /* Clause ARDUINO */

#endif /* KV_STDIO_INTERCEPT_H_ */
