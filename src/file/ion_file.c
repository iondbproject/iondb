
#include "ion_file.h"

boolean_t
ion_fexists(
	char		*name
)
{
#ifdef ION_ARDUINO
	return (boolean_t)SD_File_Exists(name);
#else
	return (-1 != access(name, F_OK));
#endif
}

file_handle_t
ion_fopen(
	char		*name
)
{
#ifdef ION_ARDUINO
	file_handle_t toret;
	toret.file = fopen(name,"r+");
	if(NULL == toret.file)
	{
		toret.file = fopen(name, "w+");
	}
	return toret;
#else
	file_handle_t	file;
	
	file		= fopen(name, "r+b");
	if (NULL == file)
	{
		file	= fopen(name, "w+b");
	}
	return file;
#endif
}

err_t
ion_fclose(
	file_handle_t	file
)
{
#ifdef ION_ARDUINO
	fclose(file.file);
	return err_ok;
#else
	fclose(file);
	return err_ok;
#endif
}

err_t
ion_fremove(
	char		*name
)
{
	int	status;
	
	status	= fremove(name);
	
	if (0 == status)
	{
		return err_ok;
	}
	else
	{
		return err_file_delete_error;
	}
}

err_t
ion_fseek(
	file_handle_t	file,
	file_offset_t	seek_to,
	int		origin
)
{
#ifdef ION_ARDUINO
	fseek(file.file, seek_to, origin);
	return err_ok;
#else
	if (0 != fseek(file, seek_to, origin)) return err_file_bad_seek;
	return err_ok;
#endif
}

file_offset_t
ion_ftell(
	file_handle_t	file
)
{
#ifdef ION_ARDUINO
	return ftell(file.file);
#else
	return ftell(file);
#endif
}

file_offset_t
ion_fend(
	file_handle_t	file
)
{
	file_offset_t	previous;
	file_offset_t	to_return;
	
	previous	= ion_ftell(file);
	ion_fseek(file, 0, ION_FILE_END);
	to_return	= ion_ftell(file);
	ion_fseek(file, previous, ION_FILE_START);
	
	return to_return;
}

err_t
ion_fwrite(
	file_handle_t	file,
	unsigned int	num_bytes,
	byte*		to_write
)
{
#ifdef ION_ARDUINO
	if (num_bytes != (fwrite(to_write, num_bytes, 1, file.file) * num_bytes))
	{
		return err_file_incomplete_write;
	}
	
	return err_ok;
#else
	fwrite(to_write, num_bytes, 1, file);
	return err_ok;
#endif
}

err_t
ion_fwrite_at(
	file_handle_t	file,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte		*to_write
)
{
	err_t	error;
	
	error	= ion_fseek(file, offset, ION_FILE_START);
	if (err_ok != error)
	{
		return error;
	}
	
	error	= ion_fwrite(file, num_bytes, to_write);
	return error;
}

err_t
ion_fappend(
	file_handle_t	file,
	unsigned int	num_bytes,
	byte*		to_write
)
{
	err_t	error;
	
	error	= ion_fseek(file, 0, ION_FILE_END);
	if (err_ok != error)
	{
		return error;
	}
	
	error	= ion_fwrite(file, num_bytes, to_write);
	return error;
}

err_t
ion_fread(
	file_handle_t	file,
	unsigned int	num_bytes,
	byte		*write_to
)
{
#ifdef ION_ARDUINO
	if (num_bytes != (fread(write_to, num_bytes, 1, file.file) * num_bytes)) {
		return err_file_incomplete_read;
	}
	
	return err_ok;
#else
	if (1 != fread(write_to, num_bytes, 1, file)) {
		return err_file_incomplete_read;
	}
	
	return err_ok;
#endif
}

err_t
ion_fread_at(
	file_handle_t	file,
	file_offset_t	offset,
	unsigned int	num_bytes,
	byte*		write_to
)
{
	err_t	error;
	error	= ion_fseek(file, offset, ION_FILE_START);
	if (err_ok != error)
	{
		return error;
	}
	
	error	= ion_fread(file, num_bytes, write_to);
	return error;
}
