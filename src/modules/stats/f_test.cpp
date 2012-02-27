/* ----------------------------------------------------------------------- *//**
 *
 * @file f_test.cpp
 *
 * @brief f-Test functions
 *
 *//* ----------------------------------------------------------------------- */

#include <dbconnector/dbconnector.hpp>
#include <modules/prob/prob.hpp>

#include "TwoSumOfSquaresTransitionState.hpp"

namespace madlib {

namespace modules {

// Import names from other MADlib modules
using prob::fisherF_CDF;

namespace stats {

// Workaround for Doxygen: A header file that does not declare namespaces is to
// be ignored if and only if it is processed stand-alone
#undef _DOXYGEN_IGNORE_HEADER_FILE
#include "t_test.hpp"

typedef MultiState<MutableArrayHandle<double>, CorrectedSumSquares, 2>
    TwoCorrectedSumSquaresTransitionState;

/**
 * @brief Perform the two-sample t-test transition step
 */
AnyType
two_corrected_sums_squares_transition::run(AnyType &args) {
    TwoCorrSSTransState state = args[0].getAs<MutableArrayHandle<double> >();
    bool firstSample = args[1].getAs<bool>();
    double value = args[2].getAs<double>();
    
    state << TwoCorrSSTransState::ValueType(firstSample, value);
    return state;
}

/**
 * @brief Perform the perliminary aggregation function: Merge transition states
 */
AnyType
two_corrected_sums_squares_merge_states::run(AnyType &args) {
    TwoCorrSSTransState stateLeft = args[0].getAs<MutableArrayHandle<double> >();
    TwoCorrSSTransState stateRight = args[1].getAs<ArrayHandle<double> >();

    stateLeft << stateRight;
    return stateLeft;
}


/**
 * @brief Perform the F-test final step
 */
AnyType
f_test_final::run(AnyType &args) {
    TwoCorrSSTransState state = args[0].getAs<ArrayHandle<double> >();

    // If we haven't seen enough data, just return Null. This is the standard
    // behavior of aggregate function (compare, e.g.,
    // how PostgreSQL handles corr on just one row. It also returns NULL.)
    if (state(0).count() < 2 || state(1).count() < 2)
        return Null();

    // Formulas taken from:
    // http://www.itl.nist.gov/div898/handbook/eda/section3/eda359.htm
    double dfX = state(0).degreeOfFreedom();
    double dfY = state(1).degreeOfFreedom();
    double statistic = state(0).sampleVariance() / state(1).sampleVariance();
    
    AnyType tuple;
    double pvalue_one_sided = 1. - fisherF_CDF(statistic, dfX, dfY);
    double pvalue_two_sided
        = 2. * std::min(pvalue_one_sided, 1. - pvalue_one_sided);
    return tuple
        << statistic
        << dfX
        << dfY
        << pvalue_one_sided
        << pvalue_two_sided;
}

} // namespace stats

} // namespace modules

} // namespace madlib
