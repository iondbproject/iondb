#if !defined(PROJECT_DOG_H)
#define PROJECT_DOG_H

class Dog {
public:

string	name;
int		age;
int		phone_number;
int		food_level;

Dog (
	string	dog_name,
	int		dog_age,
	int		dog_phonenumber
) {
	name			= dog_name;
	age				= dog_age;
	phone_number	= dog_phonenumber;
}

~Dog(
) {}

int
feed(
	int food
) {
	food_level += food;
	return food_level;
}

static Dog *
initialize_dog(
	string	dog_name,
	int		dog_age,
	int		dog_phonenumber
) {
	return new Dog(dog_name, dog_age, dog_phonenumber);
}
};

#endif /* PROJECT_DOG_H */
