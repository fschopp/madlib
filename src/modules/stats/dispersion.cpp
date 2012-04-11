/* ----------------------------------------------------------------------- *//**
 *
 * @file dispersion.cpp
 *
 * @brief Compute variability or spread in a variable
 *
 *//* ----------------------------------------------------------------------- */

#include <dbconnector/dbconnector.hpp>

#include "CorrectedSumOfSquares.hpp"

namespace madlib {

namespace modules {

namespace stats {

// Workaround for Doxygen: A header file that does not declare namespaces is to
// be ignored if and only if it is processed stand-alone
#undef _DOXYGEN_IGNORE_HEADER_FILE
#include "dispersion.hpp"

/**
 * @brief Perform the population-variance transition step
 */
AnyType
corrected_sum_of_sqaures_transition::run(AnyType &args) {
    typedef CorrectedSumOfSquares<MutableByteStringHandle> MutableState;
    
    MutableState state = args[0].isNull()
        ? allocateByteString(*this, MutableState::length())
        : args[0].getAs<MutableByteStringHandle>();
    double x = args[1].getAs<double>();
    
    return state << x;
}

/**
 * @brief Perform the two-sample t-test transition step
 */
AnyType
corrected_sum_of_sqaures_merge::run(AnyType &args) {
    CorrectedSumOfSquares<MutableByteStringHandle> stateLeft
        = args[0].getAs<MutableByteStringHandle>();
    CorrectedSumOfSquares<ByteStringHandle> stateRight
        = args[1].getAs<ByteStringHandle>();
    
    return stateLeft << stateRight;
}

/**
 * @brief Perform the one-sided t-Test final step
 */
AnyType
var_pop_final::run(AnyType &args) {
    CorrectedSumOfSquares<ByteStringHandle> state
        = args[0].getAs<ByteStringHandle>();
    
    // If we haven't seen any data, just return Null. This is the standard
    // behavior of aggregate function on empty data sets (compare, e.g.,
    // how PostgreSQL handles sum or avg on empty inputs)
    return state.count() > 0
        ? state.populationVariance()
        : Null();
}

} // namespace stats

} // namespace modules

} // namespace madlib
