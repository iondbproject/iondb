/**

@brief		Intercepts stdio.h defined functions for use on AVR platform

@details	stdio.h is not fully defined within avr-gcc.  As a result, function
			for file operations are defined and not implemented or not defined.
			As a result, it is difficult to cross compile src that contains
			defined functions.  Include this header at the top of the file,
			but after stdio.h and the functions defined below will override.

			Flags: 	-DIntercept

			Compiling with the -DIntercept flag will override functinons.  Leaving
			the function out will allow for regular use.

 */

#ifndef KV_STDIO_INTERCEPT_H_
#define KV_STDIO_INTERCEPT_H_

#ifdef ION_ARDUINO

#ifdef __cplusplus
extern "C" {
#endif

typedef long fpos_t;

#define 	FILE  				SD_FILE
#define 	fopen(x, y) 			sd_fopen(x ,y)
#define		fclose(x)			sd_fclose(x)
#define 	fwrite(w, x , y, z)		sd_fwrite(w, x, y, z)
#define		fsetpos(x, y)			sd_fsetpos(x, y)
#define	 	fgetpos(x, y)			sd_fgetpos(x, y)
#define 	fflush(x)			sd_fflush(x)
#define		fremove(x)			sd_remove(x)
#define		fseek(x ,y ,z)			sd_fseek(x ,y ,z)
#define		fread(w, x, y, z)		sd_fread(w, x, y, z)
#define 	feof(x)				sd_feof(x)
#define		ftell(x)			sd_ftell(x)
#define 	rewind(x)			sd_rewind(x)

#ifdef __cplusplus
}
#endif

#endif /* Clause ION_ARDUINO */

#endif /* KV_STDIO_INTERCEPT_H_ */
