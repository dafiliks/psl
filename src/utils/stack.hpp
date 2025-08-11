/* utils/stack.hpp by David Filiks */
/* The stack header for the PsL compiler */

#ifndef STACK_HPP
#define STACK_HPP

#include <iostream>
#include <memory>

#include "stack.hpp"
#include "../utils/error_types.hpp"

/* Template to help with meta programming */
/* Grants the ability to create a stack of any data type */
template <typename T>

/* Class to be used as a stack data structure */
class Stack
{

/* Public members */
public:

    /* Functions */

    /* Carries out a push operation to the stack */
    /* Param: const T& - the data to push onto the stack */
    void push(const T& data);

    /* Carries out a pop operation to the stack */
    /* Returns: const T& - the data which was popped off the stack */
    T pop();

    /* Getter function for the data size */
	/* Returns: std::size_t - the data size */
    [[nodiscard]] std::size_t get_size() const;

    /* Getter function for the data capacity */
	/* Returns: const std::size_t& - the data capacity */
    [[nodiscard]] std::size_t get_capacity() const;

    /* Getter function for the data */
	/* Returns: const std::unique_ptr<T>& - the data */
    [[nodiscard]] const std::unique_ptr<T>& get_data() const;

/* Private members */
private:

    /* Functions */

    /* Resizes the stack the a given capacity */
    /* const std::size_t - the new capacity of the stack */
    void resize(const std::size_t new_capacity);

    /* Variables */

    std::size_t m_size{}; /* The current size of the stack */
    std::size_t m_capacity{}; /* The capacity of the stack*/

    std::unique_ptr<T[]> m_data{}; /* Holds the stack data */
};

#endif