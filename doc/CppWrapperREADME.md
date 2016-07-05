![IonDB](http://stickerpants.com/uploads/IonDB_logo_final.svg)
=========

#IonDB C++ Wrapper

##"What is this?"
Previously, IonDB was exclusively utilized through methods written in the C language.  Now, all the key features of IonDB have been conveniently packaged in C++ in a user-friendly C++ Wrapper.

>**Note:** IonDB C++ Wrapper currently does not support Master Table functions. These functions will become available in the future as rapid IonDB development continues.

#Usage Guide
##Creating a Dictionary
Dictionaries are **specific type agnostic**, and can be any of the following types: BppTree, FlatFile, LinearHash, OpenAddressFileHash, OpenAddressHash, and SkipList.

>**Note:** Before use of a file-based dictionary implementation (BppTree, FlatFile), *SD.begin(...)* must be called.

Key type must also be declared upon dictionary creation, with reference to both the C++ data type and the proper IonDB key category.

The other information necessary to create a dictionary includes: key size, value size, and dictionary size if required.

```c
    Dictionary<int> *btree = new BppTree<int>(key_type_numeric_signed, sizeof(int), 10);
```

##Usage

###Insert

```c
    int key = 3;
    string value = "hello!";
	btree->insert(key, value);
```

###Get
The expected data type of the value to be returned needs to be specified in the method call.

```c
    int key = 3;
    btree->get<string>(key);
```

###Update

```c
    int key = 3;
    string new_value = "hi!"
    btree->update(key, new_value);
```

###Delete

```c
    int key = 3;
    btree->deleteRecord(key);
```

###Equality Query
First, *equality(key)* is called to initialize the equality query. Then, *getRecord(cursor)* is called to retrieve each record which satisfies the given query.

```c
    int key = 3;
    dict_cursor_t *cursor = btree->equality(key);
    ion_record_t record_1 = btree->getRecord(cursor);
    ion_record_t record_2 = btree->getRecord(cursor);
    ion_record_t record_3 = btree->getRecord(cursor);
```

###All Records Query
First, *allRecords()* is called to initialize the select all records query. Then, *getRecord(cursor)* is called to retrieve each record.

```c
    dict_cursor_t *cursor = btree->allRecords();
    ion_record_t record_1 = btree->getRecord(cursor);
    ion_record_t record_2 = btree->getRecord(cursor);
    ion_record_t record_3 = btree->getRecord(cursor);
    ion_record_t record_4 = btree->getRecord(cursor);
```

###Range Query
First, *range(minimum_key, maximum_key)* is called to initialize the range query. Then, *getRecord(cursor)* is called to retrieve each record which satisfies the given query.

```c
    int minimum_key = 2;
    int maximum_key = 3;
    dict_cursor_t *cursor = btree->range(minimum_key, maximum_key);
    ion_record_t record_1 = btree->getRecord(cursor);
    ion_record_t record_2 = btree->getRecord(cursor);
    ion_record_t record_3 = btree->getRecord(cursor);
```

##Full Example

```c
     //Create Dictionary
     Dictionary<int> *btree = new BppTree<int>(key_type_numeric_signed, sizeof(int), 10);

	//Insert
	int key = 3;
	string value = "hello!";
	btree->insert(key, value);

	//Get
	btree->get<string>(key);

	//Update
	string new_value = "hi!";
	btree->update(key, new_value);

	//Delete
	btree->deleteRecord(key);

	//Insert additional records to test queries
	string value_1 = "test1!";
	btree->insert(key, value_1);

	string value_2 = "test2!";
	btree->insert(key, value_2);

	int key_2
	string value_3 = "test3!";
	btree->insert(key_2, value_3);

	//Equality Query
	dict_cursor_t *cursor1 = btree->equality(key);
	ion_record_t rec11 = btree->getRecord(cursor1);
	ion_record_t rec12 = btree->getRecord(cursor1);
	ion_record_t rec13 = btree->getRecord(cursor1);

	//All Records Query
	dict_cursor_t *cursor2 = btree->allRecords();
	ion_record_t rec21 = btree->getRecord(cursor2);
	ion_record_t rec22 = btree->getRecord(cursor2);
	ion_record_t rec23 = btree->getRecord(cursor2);
	ion_record_t rec24 = btree->getRecord(cursor2);

	//Range Query
	int minimum_key = 2;
    int maximum_key = 3;
    dict_cursor_t *cursor = btree->range(minimum_key, maximum_key);
	ion_record_t rec31 = btree->getRecord(cursor3);
	ion_record_t rec32 = btree->getRecord(cursor3);
	ion_record_t rec33 = btree->getRecord(cursor3);

	btree->destroy();
	delete btree;
```