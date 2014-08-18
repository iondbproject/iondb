
#include "ion_file.h"

boolean_t
ion_fexists(
	char		*name
)
{
#if FS_TARGET == FS_TARGET_ARDUINO
	return (boolean_t)SD_File_Exists(filename);
#else
	return (-1 != access(name, F_OK));
#endif
}

file_handle_t
ion_fopen(
	char		*name
)
{
#if FS_TARGET == FS_TARGET_ARDUINO
	file_handle_t toret;
	if (1 != SD_File_Open(&(toret.file), filename, SD_FILE_MODE_WRITE))
		return DB_STORAGE_NOFILE;
	return toret;
#else
	file_handle_t	file;
	
	file		= fopen(name, "r+b");
	if (NULL == file)
		file	= fopen(name, "w+b");
	
	return file;
#endif
}

err_t
ion_fclose(
	file_handle_t	file
)
{
#if FS_TARGET == FS_TARGET_ARDUINO
	SD_File_Close(file.file);
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
#if FS_TARGET == FS_TARGET_ARDUINO
	SD_File_Remove(name);
	return err_ok;
#else
	int	status;
	
	status	= remove(name);
	
	if (0 == status)
	{
		return err_ok;
	}
	else
	{
		return err_could_not_delete_file;
	}
#endif
}

err_t
ion_fseek(
	file_handle_t	file,
	file_offset_t	seek_to,
	int		origin
)
{
#if FS_TARGET == FS_TARGET_ARDUINO
	size_t pos;
	pos		= SD_File_Position(file.file);
	
	if (ION_FILE_START == origin)
	{
		pos	= seek_to;
	}
	else if (ION_FILE_END == origin)
	{
		pos	= file.end - seek_to;
	}
	
	if (1 != SD_File_Seek(f.file, pos+seek_to))
	{
		return err_file_incomplete_read;
	}
	
	return err_ok;
#else
	fseek(file, seek_to, origin);
	return err_ok;
#endif
}

file_offset_t
ion_ftell(
	file_handle_t	file
)
{
#if FS_TARGET == FS_TARGET_ARDUINO
	return SD_File_Position(file.file);
#else
	return ftell(file);
#endif
}

file_offset_t
ion_fend(
	file_handle_t	file
)
{
#if FS_TARGET == FS_TARGET_ARDUINO
	return file.end;
#else
	file_offset_t	previous;
	file_offset_t	to_return;
	
	previous	= ion_ftell(file);
	ion_fseek(file, 0, ION_FILE_END);
	to_return	= ion_ftell(file);
	ion_fseek(file, previous, ION_FILE_START);
#endif
	
	return to_return;
}

err_t
ion_fwrite(
	file_handle_t	file,
	unsigned int	num_bytes,
	byte*		to_write
)
{
#if FS_TARGET == FS_TARGET_ARDUINO
	file_offset_t	pos;
	
	pos	= SD_File_Position(file.file);
	
	if (pos+num_bytes > file.end)
		file.end	= pos + num_bytes;
	
	if (num_bytes != SD_File_Write(file.file, to_write, num_bytes))
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
#if FS_TARGET == FS_TARGET_ARDUINO
	if (num_bytes != SD_File_read(file.file, write_to, num_bytes);
		return err_file_incomplete_read;
	
	return err_ok;
#else
	if (1 != fread(write_to, num_bytes, 1, file))
		return err_file_incomplete_read;
	
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
