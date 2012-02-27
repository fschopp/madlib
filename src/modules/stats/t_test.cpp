/* ----------------------------------------------------------------------- *//**
 *
 * @file t_test.cpp
 *
 * @brief t-Test functions
 *
 *//* ----------------------------------------------------------------------- */

#include <dbconnector/dbconnector.hpp>
#include <modules/prob/prob.hpp>

#include "TransitionStates.hpp"

// We use string concatenation with the + operator
#include <string>

namespace madlib {

namespace modules {

// Import names from other MADlib modules
using prob::chiSquaredCDF;
using prob::fisherF_CDF;
using prob::studentT_CDF;

namespace stats {

// Workaround for Doxygen: A header file that does not declare namespaces is to
// be ignored if and only if it is processed stand-alone
#undef _DOXYGEN_IGNORE_HEADER_FILE
#include "t_test.hpp"

struct internal : public AbstractionLayer {
    static AnyType tStatsToResult(double inT, double inDegreeOfFreedom);
};

/**
 * @brief Update the corrected sum of squares
 *
 * For numerical stability, we should not compute the sample variance in the
 * naive way. The literature has many examples where this gives bad results
 * even with moderately sized inputs.
 *
 * See:
 *
 * B. P. Welford (1962). "Note on a method for calculating corrected sums of
 * squares and products". Technometrics 4(3):419–420.
 *
 * Chan, Tony F.; Golub, Gene H.; LeVeque, Randall J. (1979), "Updating
 * Formulae and a Pairwise Algorithm for Computing Sample Variances.", Technical
 * Report STAN-CS-79-773, Department of Computer Science, Stanford University.
 * ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/79/773/CS-TR-79-773.pdf
 */
inline
void
updateCorrectedSumOfSquares(double &ioLeftWeight, double &ioLeftSum,
    double &ioLeftCorrectedSumSquares, double inRightWeight, double inRightSum,
    double inRightCorrectedSumSquares) {
    
    if (inRightWeight <= 0)
        return;
    
    // FIXME: Use compensated sums for numerical stability
    // See Ogita et al., "Accurate Sum and Dot Product", SIAM Journal on
    // Scientific Computing (SISC), 26(6):1955-1988, 2005.
    if (ioLeftWeight <= 0)
        ioLeftCorrectedSumSquares = inRightCorrectedSumSquares;
    else {
        double diff = inRightWeight / ioLeftWeight * ioLeftSum - inRightSum;
        ioLeftCorrectedSumSquares
               += inRightCorrectedSumSquares
                + ioLeftWeight / (inRightWeight * (ioLeftWeight + inRightWeight))
                    * diff * diff;
    }
    
    ioLeftSum += inRightSum;
    ioLeftWeight += inRightWeight;
}

/**
 * @brief Perform the one-sample t-test transition step
 */
AnyType
t_test_one_transition::run(AnyType &args) {
    TTestTransitionState<MutableArrayHandle<double> > state = args[0];
    double x = args[1].getAs<double>();
    
    updateCorrectedSumOfSquares(
        state.numX, state.x_sum, state.correctedX_square_sum,
        1, x, 0);

    return state;
}

/**
 * @brief Perform the two-sample t-test transition step
 */
AnyType
t_test_two_transition::run(AnyType &args) {
    TTestTransitionState<MutableArrayHandle<double> > state = args[0];
    bool firstSample = args[1].getAs<bool>();
    double value = args[2].getAs<double>();
    
    if (firstSample)
        updateCorrectedSumOfSquares(
            state.numX, state.x_sum, state.correctedX_square_sum,
            1, value, 0);
    else
        updateCorrectedSumOfSquares(
            state.numY, state.y_sum, state.correctedY_square_sum,
            1, value, 0);

    return state;
}

/**
 * @brief Perform the perliminary aggregation function: Merge transition states
 */
AnyType
t_test_merge_states::run(AnyType &args) {
    TTestTransitionState<MutableArrayHandle<double> > stateLeft = args[0];
    TTestTransitionState<ArrayHandle<double> > stateRight = args[1];
    
    // Merge states together and return
    updateCorrectedSumOfSquares(
        stateLeft.numX, stateLeft.x_sum, stateLeft.correctedX_square_sum,
        stateRight.numX, stateRight.x_sum, stateRight.correctedX_square_sum);
    updateCorrectedSumOfSquares(
        stateLeft.numY, stateLeft.y_sum, stateLeft.correctedY_square_sum,
        stateRight.numY, stateRight.y_sum, stateRight.correctedY_square_sum);
    
    return stateLeft;
}

/**
 * @brief Perform the one-sided t-Test final step
 */
AnyType
t_test_one_final::run(AnyType &args) {
    TTestTransitionState<ArrayHandle<double> > state = args[0];

    // If we haven't seen any data, just return Null. This is the standard
    // behavior of aggregate function on empty data sets (compare, e.g.,
    // how PostgreSQL handles sum or avg on empty inputs)
    if (state.numX == 0)
        return Null();
    
    double degreeOfFreedom = state.numX - 1;
    double sampleVariance = state.correctedX_square_sum
                          / degreeOfFreedom;
    double t = std::sqrt(state.numX / sampleVariance)
             * (state.x_sum / state.numX);
    
    return internal::tStatsToResult(t, degreeOfFreedom);
}

/**
 * @brief Perform the pooled (i.e., assuming equal variances) two-sample t-Test
 *     final step
 */
AnyType
t_test_two_pooled_final::run(AnyType &args) {
    TTestTransitionState<ArrayHandle<double> > state = args[0];

    // If we haven't seen enough data, just return Null. This is the standard
    // behavior of aggregate function (compare, e.g.,
    // how PostgreSQL handles corr on just one row. It also returns NULL.)
    if (state.numX < 1 || state.numY < 1 || state.numX + state.numY <= 2)
        return Null();

    // Formulas taken from:
    // http://www.itl.nist.gov/div898/handbook/eda/section3/eda353.htm
    double dfEqualVar = state.numX + state.numY - 2;
    double diffInMeans = state.x_sum / state.numX - state.y_sum / state.numY;
    double sampleVariancePooled
        = (state.correctedX_square_sum + state.correctedY_square_sum)
        / dfEqualVar;
    double tDenomEqualVar
        = std::sqrt(sampleVariancePooled * (1. / state.numX + 1. / state.numY));
    double tEqualVar = diffInMeans / tDenomEqualVar;
    
    return internal::tStatsToResult(tEqualVar, dfEqualVar);
}

/**
 * @brief Perform the unpooled (i.e., assuming unequal variances) two-sample
 *     t-Test final step
 */
AnyType
t_test_two_unpooled_final::run(AnyType &args) {
    TTestTransitionState<ArrayHandle<double> > state = args[0];

    // If we haven't seen enough data, just return Null. This is the standard
    // behavior of aggregate function (compare, e.g.,
    // how PostgreSQL handles corr on just one row. It also returns NULL.)
    if (state.numX < 2 || state.numY < 2)
        return Null();

    // Formulas taken from:
    // http://www.itl.nist.gov/div898/handbook/eda/section3/eda353.htm
    double sampleVarianceX = state.correctedX_square_sum / (state.numX - 1);
    double sampleVarianceY = state.correctedY_square_sum / (state.numY - 1);
    
    double sampleVarianceX_over_numX = sampleVarianceX / state.numX;
    double sampleVarianceY_over_numY = sampleVarianceY / state.numY;
    
    double dfUnequalVar
        = std::pow(sampleVarianceX_over_numX + sampleVarianceY_over_numY, 2)
        / (
            std::pow(sampleVarianceX_over_numX, 2) / (state.numX - 1.)
          + std::pow(sampleVarianceY_over_numY, 2) / (state.numY - 1.)
          );
    double diffInMeans = state.x_sum / state.numX - state.y_sum / state.numY;
    double tDenomUnequalVar
        = std::sqrt(sampleVarianceX / state.numX + sampleVarianceY / state.numY);
    double tUnequalVar = diffInMeans / tDenomUnequalVar;
    
    return internal::tStatsToResult(tUnequalVar, dfUnequalVar);
}

inline
AnyType
internal::tStatsToResult(double inT, double inDegreeOfFreedom) {
    // Return t statistic, degrees of freedom, one-tailed p-value (Null
    // hypothesis \mu <= \mu_0), and two-tailed p-value (\mu = \mu_0).
    // Recall definition of p-value: The probability of observating a
    // value at least as extreme as the one observed, assuming that the null
    // hypothesis is true.
    AnyType tuple;
    tuple
        << inT
        << inDegreeOfFreedom
        << 1. - studentT_CDF(inT, inDegreeOfFreedom)
        << 2. * (1. - studentT_CDF(std::fabs(inT), inDegreeOfFreedom));
    return tuple;
}

/**
 * @brief Perform the F-test final step
 */
AnyType
f_test_final::run(AnyType &args) {
    TTestTransitionState<ArrayHandle<double> > state = args[0];

    // If we haven't seen enough data, just return Null. This is the standard
    // behavior of aggregate function (compare, e.g.,
    // how PostgreSQL handles corr on just one row. It also returns NULL.)
    if (state.numX < 2 || state.numY < 2)
        return Null();

    // Formulas taken from:
    // http://www.itl.nist.gov/div898/handbook/eda/section3/eda359.htm
    double dfX = state.numX - 1;
    double dfY = state.numY - 1;
    double sampleVarianceX = state.correctedX_square_sum / dfX;
    double sampleVarianceY = state.correctedY_square_sum / dfY;
    double statistic = sampleVarianceX / sampleVarianceY;
    
    AnyType tuple;
    double pvalue_one_sided = 1. - fisherF_CDF(statistic, dfX, dfY);
    double pvalue_two_sided = 2. * std::min(pvalue_one_sided,
        1. - pvalue_one_sided);
    tuple
        << statistic
        << dfX
        << dfY
        << pvalue_one_sided
        << pvalue_two_sided;
    return tuple;
}

} // namespace stats

} // namespace modules

} // namespace madlib
