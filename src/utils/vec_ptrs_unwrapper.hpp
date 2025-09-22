/* utils/vec_ptrs_unwrapper.hpp by David Filiks */
/* The vector of pointers unwrapper header for the PsL compiler */

#ifndef VEC_PTRS_UNWRAPPER_HPP
#define VEC_PTRS_UNWRAPPER_HPP

#include <vector>
#include <memory>

template <typename T>
/* Class to be used to unwrap a vector of pointers */
class VecPtrsUnwrapper
{
/* Public members */
public:

    /* Functions */

    /* Constructs a VecPtrsUnwrapper object */
    /* Param: const std::vector<std::shared_ptr<T>>& - the vector of pointers */
    VecPtrsUnwrapper(const std::vector<std::shared_ptr<T>>& vec_ptrs)
    /* Initialize member */
    : m_vec_ptrs(vec_ptrs) {}

    /* Unwraps the vector of pointers that was initialized earlier */
    /* Returns: std::vector<T> - the unwrapped vector */
    [[nodiscard]] std::vector<T> unwrap()
    {
        /* Create a storage container for the vector of dereferenced pointers */
        std::vector<T> unwrapped_vec{};

        /* Loop through the whole vector of pointers */
        for (const auto& element : m_vec_ptrs)
        {
            /* Dereference each element and add it to the unwrapped vector */
            unwrapped_vec.push_back(*element);
        }

        /* Return the unwrapped vector */
        return unwrapped_vec;
    }

    /* Getter function for the vector of pointers */
    /* Returns: const std::vector<std::shared_ptr<T>> - the vector of pointers */
    [[nodiscard]] const std::vector<std::shared_ptr<T>> get_vec_ptrs() const
    {
        /* Return the wrapped vector of pointers */
        return m_vec_ptrs;
    }

/* Private members*/
private:

    /* Variables */

    std::vector<std::shared_ptr<T>> m_vec_ptrs{}; /* The vector of pointers to unwrap */
};

#endif
