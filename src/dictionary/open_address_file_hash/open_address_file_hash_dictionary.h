/******************************************************************************/
/**
@file		open_address_file_hash_dictionary.h
@author		Scott Ronald Fazackerley
@brief		This file includes common components for oadictionary and oahash.
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

#if !defined(OPEN_ADDRESS_FILE_DICTIONARY_H_)
#define OPEN_ADDRESS_FILE_DICTIONARY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"

/*edefines file operations for arduino */
#include "../../file/sd_stdio_c_iface.h"

/**
@brief		The position in the hashmap.
*/
typedef int ion_hash_t;

typedef struct oafdict_cursor {
	ion_dict_cursor_t	super;						/**< Cursor supertype this type inherits from */
	ion_hash_t			first;						/**<First visited spot*/
	ion_hash_t			current;					/**<Currently visited spot*/
	ion_cursor_status_t status;	/**<Status of last cursor call*/
} ion_oafdict_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif /* OPEN_ADDRESS_FILE_DICTIONARY_H_ */
