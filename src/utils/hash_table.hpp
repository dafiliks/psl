/* utils/hash_table.hpp by David Filiks */
/* The hash table header for the PsL compiler */

#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <iostream>
#include <list>

template <typename T1, typename T2>
/* Class to be used as a hash table data structure */
class HashTable
{	
/* Public members */
public:

	/* Functions */

	/* Constructs a HashTable object */
	/* Param: const std::initializer_list<std::pair<T1, T2>>& - the key value pair initializer list */
	HashTable(const std::initializer_list<std::pair<T1, T2>>& init_list)
	/* Initialize the hash table bucket size */
	: m_table(m_buckets)
	{
		/* Loop through all of the key value pairs within the initializer list */
		for (auto& key_value_pair : init_list)
		{
			/* Insert each key value pair into the hash table */
			insert(key_value_pair.first, key_value_pair.second);
		}
	}

	/* Inserts a key value pair into the hash table */
	/* Param: const T1& - the key */
	/* Param: const T2& - the value */
	void insert(const T1& key, const T2& value)
	{
		/* Store the index at which the key should be stored */
		const std::size_t index{hash_func(key)};

		/* Loop through the bucket */
		for (auto& key_value_pair : m_table[index])
		{
			/* If the key already exists */
			if (key_value_pair.first == key)
			{
				/* Update the key value pair value */
				key_value_pair.second = value;
				
				/* Return early from the function to save unnecessary iterations */
				return;
			}
		}

		/* If the key does not exist, add the new key value pair */
		m_table[index].emplace_back(key, value);
	}

	/* Finds the value which has been mapped to a certain key */
	/* Param: const T1& - the key */
	/* Returns: const T2* - the value */
	[[nodiscard]] const T2* find(const T1& key) const
	{
		/* Store the index at which the key should be stored */
		const std::size_t index{hash_func(key)};

		/* Loop through the bucket */
		for (auto& key_value_pair : m_table[index])
		{
			/* If the key exists */
			if (key_value_pair.first == key)
			{
				/* Return the desired key value pair value */
				return &key_value_pair.second;
			}
		}

		/* If the key does not exist, return a nullptr */
		return nullptr;
	}

	/* Getter function for the number of buckets */
	/* Returns: std::size_t - the number of buckets */
	[[nodiscard]] std::size_t get_buckets() const
	{
		/* Return the number of buckets */
		return m_buckets;
	}

	/* Getter function for the table */
	/* Returns: const std::list<std::pair<T1, T2>>& - the table */
	[[nodiscard]] const std::list<std::pair<T1, T2>>& get_table() const
	{
		/* Return the table */
		return m_table;
	}

/* Private members */
private:

	/* Functions */

	/* The hashing function for the hash table */
	/* Param: const T1& - the key to hash */
	/* Returns: std::size_t - the index of the bucket */
	[[nodiscard]] std::size_t hash_func(const T1& key) const
	{
		/* If the key is a string, use the custom djb2 hash algorithm */
		if (std::is_same_v<T1, std::string>)
		{
			/* Store the hashing seed */
			std::size_t seed{5381};

			/* Loop through each character in the string */
			for (char character : key)
			{
				/* Perform the djb2 hashing operation */
				seed = ((seed << 5) + seed) + character;
			}

			/* Return the bucket index */
			return seed % m_buckets;
		}

		/* If the key is not a string */
		else
		{
			/* Use the appropriate standard C++ hashing algorithm instead */
			return std::hash<T1>{}(key) % m_buckets;
		}
	}

	/* Variables */

	const std::size_t m_buckets{128}; /* Stores the number of buckets in the hash table */
	std::vector<std::list<std::pair<T1, T2>>> m_table; /* The dynamic array of buckets */
};

#endif