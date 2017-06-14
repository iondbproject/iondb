/******************************************************************************/
/**
@file		test_behaviour_skip_list.c
@author		Kris Wallperington
@brief		Behaviour tests for the skip list implementation.
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
/******************************************************************************/

#include "../../../planck-unit/src/planck_unit.h"
#include "../behaviour_dictionary.h"
#include "../../../../dictionary/skip_list/skip_list_handler.h"
#include "test_behaviour_skip_list.h"

void
runalltests_behaviour_skip_list(
	void
) {
#if defined(ARDUINO)
	fdeleteall();
	bhdct_run_tests(sldict_init, 7, ION_BHDCT_ALL_TESTS & ~ION_BHDCT_STRING_INT);
#else
	bhdct_run_tests(sldict_init, 7, ION_BHDCT_ALL_TESTS);
#endif
}
