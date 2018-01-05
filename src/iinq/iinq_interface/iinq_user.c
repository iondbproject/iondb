/******************************************************************************/
/**
@file		iinq_user.c
@author		Dana Klamut
@brief		This code contains definitions for iinq user functions
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

#include "iinq_user.h"

void
cleanup(
) {
	fremove("1.ffs");
/*	fremove("2.ffs"); */
/*	fremove("3.ffs"); */
/*	fremove("4.ffs"); */
	fremove("Dogs.inq");
/*	fremove("Dogs.sch"); */
/*	fremove("Cust.inq"); */
/*	fremove("Cust.sch"); */
/*	fremove("ion_mt.tbl"); */
}

int
main(
	void
) {
/*		  SQL_execute("CREATE TABLE Dogs (id INT, type CHAR[20], name VARCHAR[30], age INT, city VARCHAR[30], primary key(id));"); */
	create_table1();
/*			  SQL_execute("INSERT INTO Dogs VALUES (10, 'Frenchie', 'Minnie', 1, 'Penticton');"); */
	insertFields();
/*	  SQL_execute("INSERT INTO Dogs VALUES (40, 'Chihuahua', 'Barky', 7, 'Van');"); */
/*	  SQL_execute("INSERT INTO Dogs VALUES (30, 'Black Lab', 'Thunder', 5, 'Penticton');"); */
/*	  SQL_execute("INSERT INTO Dogs VALUES (50, 'Cockapoo', 'Corky', 2, 'West Bench');"); */
/*	  SQL_execute("UPDATE Dogs SET id = id-1, age = age * 10 WHERE name = 'Barky';"); */
	update1();
/*	  SQL_execute("DELETE FROM Dogs WHERE age < 5;"); */
/*	  SQL_execute("DROP TABLE Dogs;"); */

	/* Clean-up */
	cleanup();

	return 0;
}
