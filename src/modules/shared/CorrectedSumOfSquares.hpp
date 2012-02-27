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
 *
 * Note: We assume that the DOUBLE PRECISION array is initialized by the
 * database with length 6, and all elemenets are 0.
 */
template <class Handle>
class CorrectedSumOfSquares : public AbstractionLayer {
public:
    typedef double ValueType;
    enum {
        compileTimeLength = 3;
    }

    CorrectedSumOfSquares(const Handle &inHandle)
      : mStorage(inHandle),
        count(&mStorage[0]),
        sum(&mStorage[1]),
        correctedSquareSum(&mStorage.at(2)) { }
    
    size_t length() const {
        return compileTimeLength;
    }

    CorrectedSumOfSquares& operator<<(ValueType inValue);
    CorrectedSumOfSquares& operator<<(const CorrectedSumOfSquares &inState);
    
    operator Handle() {
        return mStorage;
    }
    
    void init() {
        std::fill(mStorage.begin(), mStorage.end(), 0);
    }
    
    void operator=(const CorrectedSumOfSquares& inState) {
        std::copy(inState.mStorage.begin(), inState.mStorage.end(),
            mStorage.begin());
    }
    
    uint64_t count() const;
    double sum() const;
    double correctedSumSquares() const;
    double degreeOfFreedom() const;
    double sampleVariance() const;

protected:
    update(uint64_t inCount, double inSum, double inCorrectedSumSquares);

    Handle mStorage;
    typename HandleTraits<Handle>::ReferenceToUInt64 mCount;
    typename HandleTraits<Handle>::ReferenceToDouble mSum;
    typename HandleTraits<Handle>::ReferenceToDouble mCorrectedSumSquares;
};

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
uint64_t
CorrectedSumOfSquares<Handle>::count() const {
    return mCount;
}

template <class Handle>
inline
double
CorrectedSumOfSquares<Handle>::sum() const {
    return mSum;
}

template <class Handle>
inline
double
CorrectedSumOfSquares<Handle>::correctedSumSquares() const {
    return mCorrectedSumSquares;
}

template <class Handle>
inline
double
CorrectedSumOfSquares<Handle>::sampleVariance() const {
    return mCorrectedSumSquares / degreeOfFreedom();
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
