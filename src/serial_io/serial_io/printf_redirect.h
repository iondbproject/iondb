/******************************************************************************/
/**
@file		printf_redirect.h
@author		Scott Fazackerley
@brief		redirects printf statement for Arduino
*/
/******************************************************************************/
#ifndef PRINTF_REDIRECT_H_
#define PRINTF_REDIRECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Preprocessor replacement for printf */
#define printf(format, ...) {char buf[128]; snprintf(buf,128,format, __VA_ARGS__); serial_print(buf);}


#ifdef __cplusplus
}
#endif

#endif /* PRINTF_REDIRECT_H_ */
