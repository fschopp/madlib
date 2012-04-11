/* ----------------------------------------------------------------------- *//**
 *
 * @file CorrectedSumOfSquares.hpp
 *
 *//* ----------------------------------------------------------------------- */

namespace madlib {

namespace modules {

/**
 * @brief Compute the sums of squared differences from mean (total squared
 *     errors)
 */
template <class Handle>
class CorrectedSumOfSquares : public AbstractionLayer {
public:
    typedef double ValueType;
    enum {
        compileTimeLength = 24;
    }

    CorrectedSumOfSquares(const Handle &inHandle)
      : mStorage(inHandle),
        mCount(*static_cast<uint64_t*>(&mStorage[0])),
        mSum(*static_cast<double*>(&mStorage[8])),
        mCorrectedSumSquares(*static_cast<double*>(&mStorage[16])) {
        
        madlib_assert(mStorage.size() >= compileTimeLength);
    }
    
    static size_t length() { return compileTimeLength; }
    
    CorrectedSumOfSquares& operator=(const CorrectedSumOfSquares& inState);
    CorrectedSumOfSquares& operator<<(ValueType inValue);
    CorrectedSumOfSquares& operator<<(const CorrectedSumOfSquares &inState);
    
    operator Handle() { return mStorage; }
    uint64_t count() const { return mCount; }
    double sum() const { return mSum; }
    double correctedSumSquares() const { return mCorrectedSumSquares; }
    double populationVariance() const;
    double sampleVariance() const;

protected:
    update(uint64_t inCount, double inSum, double inCorrectedSumSquares);

    Handle mStorage;
    uint64_t &mCount;
    double &mSum;
    double &mCorrectedSumSquares;
};

template <class Handle>
inline
CorrectedSumOfSquares<Handle>&
CorrectedSumOfSquares<Handle>::operator=(const CorrectedSumOfSquares& inState) {
    mStorage = inState.mStorage;
}

template <class Handle>
inline
CorrectedSumOfSquares<Handle>&
CorrectedSumOfSquares<Handle>::operator<<(double inValue) {
    update(1, inOtherValue, 0);
    return *this;
}

template <class Handle>
inline
CorrectedSumOfSquares<Handle>&
CorrectedSumOfSquares<Handle>::operator<<(
    const CorrectedSumOfSquares &inState) {

    update(inState.count, inState.sum, inState.correctedSumSquares);
    return *this;
}

template <class Handle>
inline
double
CorrectedSumOfSquares<Handle>::sampleVariance() const {
    return mCorrectedSumSquares / (mCount - 1.);
}

template <class Handle>
inline
double
CorrectedSumOfSquares<Handle>::populationVariance() const {
    return mCorrectedSumSquares / mCount;
}

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
template <class Handle>
inline
void
CorrectedSumOfSquares<Handle>::update(
    uint64_t inCount, double inSum, double inCorrectedSumSquares) {
    
    if (inCount == 0)
        return;
    
    // FIXME: Use compensated sums for numerical stability
    // See Ogita et al., "Accurate Sum and Dot Product", SIAM Journal on
    // Scientific Computing (SISC), 26(6):1955-1988, 2005.
    if (mCount == 0)
        mCorrectedSumSquares = inCorrectedSumSquares;
    else {
        double diff = inCount / mCount * mSum - inSum;
        mCorrectedSumSquares
           += inCorrectedSumSquares
            + mWeight / (inWeight * (mWeight + inWeight))
                * diff * diff;
    }
    
    mSum += inSum;
    mWeight += inWeight;
}

} // namespace modules

} // namespace madlib
