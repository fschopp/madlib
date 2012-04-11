/* ----------------------------------------------------------------------- *//**
 *
 * @file dispersion.hpp
 *
 *//* ----------------------------------------------------------------------- */

// Workaround for Doxygen: A header file that does not declare namespaces is to
// be ignored if and only if it is processed stand-alone
#if !defined(_DOXYGEN_IGNORE_HEADER_FILE)

/**
 * @brief Population variance: Transition function
 */
DECLARE_UDF(var_pop_transition)

/**
 * @brief Population variance: State merge function
 */
DECLARE_UDF(var_pop_merge_states)

/**
 * @brief Population variance: Final function
 */
DECLARE_UDF(var_pop_final)

#endif // workaround for Doxygen
