/* ----------------------------------------------------------------------- *//**
 *
 * @file MultiState.hpp
 *
 *//* ----------------------------------------------------------------------- */

namespace madlib {

namespace modules {

/**
 * @brief Transition state to compute two sums of squares, used by t- and f-test
 *
 * Note: We assume that the DOUBLE PRECISION array is initialized by the
 * database with length 6, and all elemenets are 0.
 */
template <class Handle, class State, int32_t CompileTimeNum = Dynamic>
class MultiState : public AbstractionLayer {
public:
    enum {
        compileTimeLength = CompileTimeNum == Dynamic
            ? Dynamic
            : CompileTimeNum * State::length;
    };
    
    struct ValueType {
        ValueType(uint32_t inIdx, State::ValueType inValue)
          : idx(inIdx), value(inValue) { };
        
        uint32_t idx;
        State::ValueType value;
    }

    MultiState(const Handle &inHandle, uint32_t inNumStates = Dynamic);
    
    size_t numStates();
    static size_t length(uint32_t inNumStates = CompileTimeNum);
    void operator=(const MultiState& inState);
    
    State& state(uint32_t inState);
    
    MultiState& operator<<(const CorrectedSumOfSquares &inState);
    
    operator Handle() {
        return mStorage;
    }
    
protected:
    size_t numStates(uint32_t inNumStates) {
        madlib_assert(
            inNumStates == Dynamic
            || (CompileTimeNum == Dynamic && inNumStates >= 0)
            || (CompileTimeNum == inNumStates),
            std::logic_error("Invalid number of states in MultiState."));
        
        return CompileTimeNum == Dynamic
            ? (inNumStates == Dynamic ? mNum : inNumStates)
            : CompileTimeNum;
    }
    
    Handle mStorage;
    typename HandleTraits<Handle>::ReferenceToUInt32 mNum;
    std::vector<State> mStates;
};

template <class Handle>
inline
MultiState<Handle, State, CompileTimeNum>::MultiState(
    const Handle &inHandle,
    uint32_t inNumStates = Dynamic
) : mStorage(inHandle),
    mNum(&mStorage.at[0]) {
    
    uint32_t numStates = CompileTimeNum == Dynamic
        ? (inNumStates == Dynamic ? mNum : inNumStates)
        : CompileTimeNum;
    setNumStates(numStates);
}

setNumStates(uint32_t inNumStates) {
    madlib_assert(mStorage.size() >= length(inNumStates));
    
    if (mStates.size() < inNumStates) {
        for (int32_t i = mStates.size(); i < inNumStates; i++) {
            State newState(
                &mStorage[(CompileTimeNum == Dynamic) + i * State::length]);
            
            mStates.insert(newState);
        }
    } else {
        mStates.truncate(inNumStates);
    }
}

template <class Handle, class State, int32_t CompileTimeNum = Dynamic>
inline
size_t
MultiState<Handle, State, CompileTimeNum>::numStates() {
    return numStates(Dynamic);
}

template <class Handle, class State, int32_t CompileTimeNum = Dynamic>
inline
size_t
length(uint32_t inNumStates) {
    return CompileTimeNum == Dynamic
        ? 1 + numStates(inNumStates) * State::length
        : compileTimeLength;
}



template <class Handle, class State, int32_t CompileTimeNum = Dynamic>
inline
void
MultiState<Handle, State, CompileTimeNum>::operator=(
    const MultiState& inState
) {
    setNumStates(inState.mStates.size());
    for (iterator self = mStates.begin(), other = inState.mStates.begin();
        it != mStates.end(); ++self, ++other)
        *it = *other;
}

template <class Handle, class State, int32_t CompileTimeNum = Dynamic>
inline
State& state(uint32_t inState) {
    return mStates.at(inState);
}




} // namespace modules

} // namespace madlib
