/* utils/stack.cpp by David Filiks */
/* The stack implementation for the PsL compiler */

#include "stack.hpp"

template <typename T>
void Stack<T>::push(const T& data)
{
    /* If the stack is at capacity already */
    if (m_size == m_capacity)
    {
        /* Resize the stack capacity to two times it's current capacity */
        resize(m_capacity * 2);
    }

    /* Push the data onto the stack */
    /* Increment the stack size by one */
    m_data[m_size++] = data;
}

template <typename T>
T Stack<T>::pop()
{
    /* If the stack size is zero */
    if (m_size == 0)
    {
        /* Error out */
        StackError
        {
            "cannot pop from an empty stack"
        };
    }

    /* Pop the data off the stack */
    /* Decrement the stack size by one */
    return m_data[--m_size]; /* Return the popped data */
}

template <typename T>
[[nodiscard]] std::size_t Stack<T>::get_size() const
{
    return m_size; /* Return the stack size */
}

template <typename T>
[[nodiscard]] std::size_t Stack<T>::get_capacity() const
{
    return m_capacity; /* Return the stack capacity */
}

template <typename T>
[[nodiscard]] const std::unique_ptr<T>& Stack<T>::get_data() const
{
    return m_data; /* Return the data held by the stack */
}

template <typename T>
void Stack<T>::resize(const std::size_t new_capacity)
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