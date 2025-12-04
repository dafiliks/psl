/* utils/stack.hpp by David Filiks */
/* The stack header for the PsL compiler */

#ifndef STACK_HPP
#define STACK_HPP

#include <iostream>
#include <memory>

#include "../utils/error_types.hpp"

template <typename T>
/* Class to be used as a stack data structure */
class Stack
{
/* Public members */
public:

    /* Functions */

    /* Carries out a push operation to the stack */
    /* Param: const T& - the data to push onto the stack */
    void push(const T& data)
    {
        /* If the stack is at capacity already */
        if (m_size == m_capacity)
        {
            /* Resize the stack capacity to two times its current capacity */
            resize(m_capacity * 2);
        }

        /* Push the data onto the stack */
        /* Increment the stack size by one */
        m_data[m_size++] = data;
    }

    /* Carries out a pop operation to the stack */
    /* Returns: T - the data which was popped off the stack */
    T pop()
    {
        /* If the stack is empty */
        if (empty())
        {
            /* Throw stack error */
            throw StackError
            {
                "cannot pop from an empty stack"
            };
        }

        /* Pop the data off the stack */
        /* Decrement the stack size by one */
        return m_data[--m_size]; /* Return the popped data */
    }

    /* Gets the data that lies at the top of the stack */
    /* Returns: T - the data at the top of the stack */
    [[nodiscard]] T top() const
    {
        return m_data[m_size - 1]; /* Return the data at the top of the stack */
    }

    /* Checks whether the stack is empty or not */
    /* Returns: bool - whether the stack is empty */
    [[nodiscard]] bool empty() const
    {
        /* Return true if size is equal to zero, false if not */
        return m_size == 0 ? true : false;
    }

    /* Getter function for the data size */
    /* Returns: std::size_t - the data size */
    [[nodiscard]] std::size_t get_size() const
    {
        /* Return the stack size */
        return m_size;
    }

    /* Getter function for the data capacity */
    /* Returns: std::size_t - the data capacity */
    [[nodiscard]] std::size_t get_capacity() const
    {
        /* Return the stack capacity */
        return m_capacity;
    }

    /* Getter function for the data */
    /* Returns: const std::unique_ptr<T>& - the data */
    [[nodiscard]] const std::unique_ptr<T>& get_data() const
    {
        /* Return the data held by the stack */
        return m_data;
    }

/* Private members */
private:

    /* Functions */

    /* Resizes the stack the a given capacity */
    /* const std::size_t - the new capacity of the stack */
    void resize(const std::size_t new_capacity)
    {
        /* Create a data container capable of holding the new capacity size */
        std::unique_ptr<T[]> new_data{std::make_unique<T[]>(new_capacity)};

        /* Set the stack capacity to the new capacity */
        m_capacity = new_capacity;

        /* Loop through the stack data */
        for (std::size_t i{}; i < m_size; i++)
        {
            /* Move current stack data into the bigger, new data container */
            new_data[i] = std::move(m_data[i]);
        }

        /* Move new data into data, giving the stack a capacity of new_capacity */
        m_data = std::move(new_data);
    }

    /* Variables */

    std::size_t m_size{}; /* The current size of the stack */
    std::size_t m_capacity{}; /* The capacity of the stack*/

    std::unique_ptr<T[]> m_data{}; /* Holds the stack data */
};

#endif
