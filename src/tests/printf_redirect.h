/******************************************************************************/
/**
@file		printf_redirect.h
@author		Scott Fazackerley
@brief		Redirects printf statement for Arduino.
@details        Note that this file does not need to be included
		and is automatically used by @ref serial_c_iface.h.
		This is originally from the IonDB project.
@copyright      Copyright 2014 Scott Fazackerley
@license        Licensed under the Apache License, Version 2.0 (the "License");
                you may not use this file except in compliance with the License.
                You may obtain a copy of the License at
                        http://www.apache.org/licenses/LICENSE-2.0

@par
                Unless required by applicable law or agreed to in writing,
                software distributed under the License is distributed on an
                "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
                either express or implied. See the License for the specific
                language governing permissions and limitations under the
*/
/******************************************************************************/
#ifndef PRINTF_REDIRECT_H_
#define PRINTF_REDIRECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Preprocessor replacement for printf */
#define printf(format, ...) {char buf[128]; snprintf(buf,128,format,##__VA_ARGS__); serial_print(buf);}


#ifdef __cplusplus
}
#endif

#endif /* PRINTF_REDIRECT_H_ */
