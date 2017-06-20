/******************************************************************************/
/**
@file		ion_file.c
@author		Graeme Douglas
@brief		A file API for the ionDB.
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

#include "ion_file.h"

ion_boolean_t
ion_fexists(
	char *name
) {
#if defined(ARDUINO)
	return (ion_boolean_t) SD_File_Exists(name);
#else
	return -1 != access(name, F_OK);
#endif
}

ion_file_handle_t
ion_fopen(
	char *name
) {
#if defined(ARDUINO)

	ion_file_handle_t toret;

	toret.file = fopen(name, "r+");

	if (NULL == toret.file) {
		toret.file = fopen(name, "w+");
	}

	return toret;
#else

	ion_file_handle_t file;

	file = fopen(name, "r+b");

	if (NULL == file) {
		file = fopen(name, "w+b");
	}

	return file;
#endif
}

ion_err_t
ion_fclose(
	ion_file_handle_t file
) {
#if defined(ARDUINO)
	fclose(file.file);
	return err_ok;
#else
	fclose(file);
	return err_ok;
#endif
}

ion_err_t
ion_fremove(
	char *name
) {
	int status;

	status = fremove(name);

	if (0 == status) {
		return err_ok;
	}
	else {
		return err_file_delete_error;
	}
}

ion_err_t
ion_fseek(
	ion_file_handle_t	file,
	ion_file_offset_t	seek_to,
	int					origin
) {
#if defined(ARDUINO)

	if (0 != fseek(file.file, seek_to, origin)) {
		return err_file_bad_seek;
	}

	return err_ok;
#else

	if (0 != fseek(file, seek_to, origin)) {
		return err_file_bad_seek;
	}

	return err_ok;
#endif
}

ion_file_offset_t
ion_ftell(
	ion_file_handle_t file
) {
#if defined(ARDUINO)
	return ftell(file.file);
#else
	return ftell(file);
#endif
}

ion_file_offset_t
ion_fend(
	ion_file_handle_t file
) {
	ion_file_offset_t	previous;
	ion_file_offset_t	to_return;

	previous	= ion_ftell(file);
	ion_fseek(file, 0, ION_FILE_END);
	to_return	= ion_ftell(file);
	ion_fseek(file, previous, ION_FILE_START);

	return to_return;
}

ion_err_t
ion_fwrite(
	ion_file_handle_t	file,
	unsigned int		num_bytes,
	ion_byte_t			*to_write
) {
#if defined(ARDUINO)

	if (num_bytes != (fwrite(to_write, num_bytes, 1, file.file) * num_bytes)) {
		return err_file_write_error;
	}

	return err_ok;
#else
	fwrite(to_write, num_bytes, 1, file);
	return err_ok;
#endif
}

ion_err_t
ion_fwrite_at(
	ion_file_handle_t	file,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*to_write
) {
	ion_err_t error;

	error = ion_fseek(file, offset, ION_FILE_START);

	if (err_ok != error) {
		return error;
	}

	error = ion_fwrite(file, num_bytes, to_write);
	return error;
}

ion_err_t
ion_fappend(
	ion_file_handle_t	file,
	unsigned int		num_bytes,
	ion_byte_t			*to_write
) {
	ion_err_t error;

	error = ion_fseek(file, 0, ION_FILE_END);

	if (err_ok != error) {
		return error;
	}

	error = ion_fwrite(file, num_bytes, to_write);
	return error;
}

ion_err_t
ion_fread(
	ion_file_handle_t	file,
	unsigned int		num_bytes,
	ion_byte_t			*write_to
) {
#if defined(ARDUINO)

	if (num_bytes != (fread(write_to, num_bytes, 1, file.file) * num_bytes)) {
		return err_file_read_error;
	}

	return err_ok;
#else

	if (1 != fread(write_to, num_bytes, 1, file)) {
		return err_file_read_error;
	}

	return err_ok;
#endif
}

ion_err_t
ion_fread_at(
	ion_file_handle_t	file,
	ion_file_offset_t	offset,
	unsigned int		num_bytes,
	ion_byte_t			*write_to
) {
	ion_err_t error;

	error = ion_fseek(file, offset, ION_FILE_START);

	if (err_ok != error) {
		return error;
	}

	error = ion_fread(file, num_bytes, write_to);
	return error;
}
