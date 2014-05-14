
status_t
lpmemhashtable_insert(
	lpmemhashtable_t	*handler,
	my_key_t		key,
	value_t			value
)
{
	hashvalue_t hashed = LPMHT_HASH(key);
	
	/* Linearly probe until a free spot found. */
	hashvalue_t i, candidate = hashed;
	for (i = 1; LPMHT_OCCUPIED(candidate) && i < handler->maxelements; ++i)
	{
		candidate = (hashed + i) % handler->maxelements;
	}
	
	/* If the found spot is full, no spots are available! */
	if (LPMHT_OCCUPIED(candidate))
	{
		return (status_t){ERROR_FULL, 0};
	}
	
	/* Otherwise, try to insert the element. */
	
	/* This is awful. */
	byte toinsert[
		sizeof(byte) +
		handler->super.keysize +
		handler->super.valuesize
	];
	
	toinsert[0] = 1;
	copybytes(toinsert, 1, key, 0, handler->keysize);
	copybytes(toinsert, 1+handler->keysize, value, 0, handler->valuesize);
	
	return storage_write(handler->chunk, toinsert, candidate);
}

dict_iterator_t
lpmemhashtable_find(
	lpmemhashtable_t	*handler,
	my_key_t		key
)
{
	hashvalue_t hashed		= LPMHT_HASH(key);
	
	/* Find the first thing with given key. */
	hashvalue_t i, candidate = hashed;
	for (i = 1;
	     !LPMHT_OCCUPIEDBY(candidate, key) && i < handler->maxelements;
	     ++i)
	{
		candidate = (hashed + i) % handler->maxelements;
	}
	
	lpmemhashtable_cursor_t cursor;
	
	if (LPHMT_OCCUPIEDBY(candidate, key))
	{
		cursor->super.status	= (status_t){ERROR_NONE, 0};
		cursor->super.dict	= handler;
		cursor->first		= candidate;
		cursor->current		= -1;	// TODO: Don't know what I am doing.
	}
	else
	{
		cursor->super.status	= (status_t){ERROR_NOTFOUND, 0};
	}
	
	return cursor;
}

bool_t
lpmemhashtable_next(
	lpmemhashtable_cursor_t	*cursor
)
{
	while (cursor->current != cursor->first &&
	       LPMHT_OCCUPIEDBY(cursor->cursor, key))
	{
		cursor->current =
			(cursor->current + 1) % cursor->super.dict->maxelements;
	}
	
	return (bool_t)(cursor->current != cursor->first);
}

status_t
lpmemhashtable_get(
	lpmemhashtable_cursor_t	*cursor,
	value_t			value
)
{
	chunk_handler_t* chunk =
		((lpmemhashtable_t*)(cursor->super.dict))->chunk;
	
	copybytes(, 1, key, 0, handler->keysize);
}
