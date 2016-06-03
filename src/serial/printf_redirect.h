/******************************************************************************/
/**
@file		printf_redirect.h
@author		Scott Fazackerley
@brief		redirects printf statement for Arduino
*/
/******************************************************************************/
#if !defined(PRINTF_REDIRECT_H_)
#define PRINTF_REDIRECT_H_

#if defined(__cplusplus)
extern "C" {
#endif

/** Preprocessor replacement for printf */
#define printf(format, ...) {char buf[128]; snprintf(buf,128,format,##__VA_ARGS__); serial_print(buf);}


#if defined(__cplusplus)
}
#endif

#endif /* PRINTF_REDIRECT_H_ */
