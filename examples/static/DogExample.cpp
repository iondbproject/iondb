#include <iostream>
using namespace std;

#include "../../src/cpp_wrapper/Dog.h"
#include "../../src/cpp_wrapper/DogTemplate.h"

int
main(
	void
) {
	/* Create Dog object using constructor */
	Dog *dog1 = new Dog("Minnie", 1, 250);

	cout << dog1->name << endl;

	/* Create Dog object using static function */
	Dog *dog2 = Dog::initialize_dog("Sadie", 13, 493);

	cout << dog2->name << endl;

	/* Ensure returned object can access other non-static members of the class */
	dog2->feed(10);

	cout << dog2->food_level << endl;

	/* Re-try example using templated Dog class */

	/* Create Dog object using constructor */
	DogT<int> *dogT1 = new DogT<int>("Beau", 4, 490);

	cout << dogT1->name << endl;

	/* Create Dog object using static function */
	int type			= 0;

	DogT<int> *dogT2	= DogT<int>::initialize_dog("Charlie", 11, 778, type);

	cout << dogT2->name << endl;

	/* Ensure returned object can access other non-static members of the class */
	dogT2->feed(22);

	cout << dogT2->food_level << endl;

	return 0;
}
