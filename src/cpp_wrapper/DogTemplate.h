#if !defined(PROJECT_DOG_TEMPLATE_H)
#define PROJECT_DOG_TEMPLATE_H

template<typename K>
class DogT {
public:

string	name;
int		age;
int		phone_number;
int		food_level;

DogT<K>(string dog_name, int dog_age, int dog_phonenumber) {
	name			= dog_name;
	age				= dog_age;
	phone_number	= dog_phonenumber;
}

~DogT(
) {}

int
feed(
	int food
) {
	food_level += food;
	return food_level;
}

static DogT<K> *
initialize_dog(
	string	dog_name,
	int		dog_age,
	int		dog_phonenumber,
	K		type
) {
	type = 0;
	return new DogT<K>(dog_name, dog_age, dog_phonenumber);
}
};

#endif /* PROJECT_DOG_TEMPLATE_H */
