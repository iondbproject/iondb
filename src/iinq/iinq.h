/*INDENT-OFF*/
#if !defined(IINQ_H_)
#define IINQ_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <malloc.h>
#else /* Not windows, then use the proper header */
#include <alloca.h>
#endif
#include "../dictionary/dictionary_types.h"
#include "../dictionary/ion_master_table.h"
#include "../key_value/kv_system.h"
#include "../util/sort/external_sort/external_sort.h"
#include "../dictionary/flat_file/flat_file_dictionary_handler.h"

#if defined(ARDUINO)
#include "../file/kv_stdio_intercept.h"
#include "../file/sd_stdio_c_iface.h"
#endif

/**
 * @brief Handler used for all dictionaries within Iinq.
 */
#define iinq_handler_init			ffdict_init	/* TODO: Use B+ Tree when ready */

/**
 * @brief Handler used for table dictionaries.
 */
#define iinq_table_handler_init		iinq_handler_init

/**
 * @brief Handler used for queries that require materialization (not currently used).
 */
#define iinq_select_handler_init	iinq_handler_init

/**
 * @brief Handler used for dictionaries in update statements.
 */
#define iinq_update_handler_init	iinq_handler_init

/**
 * @brief Handler used for dictionaries in delete statements.
 */
#define iinq_delete_handler_init	iinq_handler_init

/**
 * @brief Size used for dictionaries created in Iinq. Considers the size of key, value, and overhead relative to the page size.
 */
#define IINQ_DICTIONARY_SIZE(key_size, value_size, overhead)	IINQ_PAGE_SIZE / ((value_size) + (key_size) + (overhead))

/**
 * @brief Size used for flat file dictionaries. Overhead is the row status indicator at the start of every row in the dictionary.
 */
#define iinq_flat_file_dictionary_size(key_size, value_size)	IINQ_DICTIONARY_SIZE((key_size), (value_size), sizeof(ion_flat_file_row_status_t))

/**
 * @brief Size used for dictionaries created for tables. Current implementation is a flat file.
 */
#define iinq_table_dictionary_size(key_size, value_size)		iinq_flat_file_dictionary_size((key_size), (value_size))

/**
@brief		Page size in bytes.
*/
#define IINQ_PAGE_SIZE 512

/**
@brief		A unique identifier used for an IINQ table
*/
typedef unsigned char iinq_table_id_t;

/**
@brief		A generic size type for the IINQ library.
*/
typedef size_t iinq_size_t;

/**
@brief		A type for storing the sorting direction used in ORDER BY clauses
			and such in IINQ queries.
*/
typedef int8_t iinq_order_direction_t;

/**
@brief		An object describing one part of an ordering clause (such as
			ORDER BY and GROUP BY).
*/
typedef struct {
	/**> A pointer to a stack-allocated-and-evaluated expression result. */
	void					*pointer;
	/**> A type flag for distinguishing signed integers and unsigned integers from everthing else. */
	uint8_t					type;
	/**> The size of the exression pointed to by @ref pointer. */
	iinq_size_t				size;
	/**> The ordering direction of this ordering object (ASCENDING or
		 DESCENDING. */
	iinq_order_direction_t	direction;
} iinq_order_part_t;

/**
@brief		Comparator context for IonDB's sorting methods.
@details	IonDB provides sorting utilities for efficiently implementing
			complex query techniques, but requires some help for comparisons.
			This comes in the form of a user-defined context. This context will
			allow IINQ queries to compare ordering keys based on directions
			(this looks like ASCENDING and DESCENDING in traditional SQL
			queries).
*/
typedef struct {
	/**> The ordering parts for the clause we are comparing for. */
	iinq_order_part_t	*parts;
	/**> The number of parts to compare. */
	int					n;
} iinq_sort_context_t;

/* TODO: change order by to use regular iinq types
/**
@brief		Types used by the comparator.
*/
typedef enum {
	IINQ_ORDERTYPE_INT, IINQ_ORDERTYPE_UINT, IINQ_ORDERTYPE_FLOAT, IINQ_ORDERTYPE_OTHER
} iinq_order_type_e;

/**
@param		table_id
				A unique identifier for a table.
@param		key_type
				The type of key to store in this source and it's dictionary.
@param		key_size
				The size of the key to store in this source and it's dictionary.
@param		value_size
				The size of the value to store in this source and it's
				dictionary.
@return		An error describing the result of the call.
*/
ion_err_t
iinq_create_source(
	iinq_table_id_t		table_id,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
);

/**
@param		table_id
				A unique identifier for a table.
@param		dictionary
				A pointer to a dictionary object to open, initialize, and
				manipulate.
@param		handler
				A pointer to a pre-allocated handler object that will be
				initialized as a result of this function call.
@return		An error describing the result of the call.
*/
ion_err_t
iinq_open_source(
	iinq_table_id_t				table_id,
	ion_dictionary_t			*dictionary,
	ion_dictionary_handler_t	*handler
);

/**
@brief		Drop a source.
@param		table_id
				A unique identifier for a table.
@return		An error describing the result of the call.
*/
ion_err_t
iinq_drop(
	iinq_table_id_t table_id
);

ion_comparison_t
iinq_sort_compare(
	void	*context,	/* TODO: Turn this into a ion_sort_comparator_context_t. */
	void	*a,
	void	*b
);

#if defined(__cplusplus)
}
#endif

#endif
/*INDENT-ON*/
