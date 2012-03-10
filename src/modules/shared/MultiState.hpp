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
template <
    class Handle,
    template <class> class State,
    int32_t CompileTimeNum = Dynamic>
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

    MultiState(const Handle &inHandle, uint32_t inNumStates = CompileTimeNum);
    static size_t length(uint32_t inNumStates = CompileTimeNum);
    
    size_t numStates();
    State<TransparentHandle>& state(uint32_t inState);

    void operator=(const MultiState& inState);
    MultiState& operator<<(const MultiState& inState);
    
    operator Handle() {
        return mStorage;
    }
    
protected:    
    Handle mStorage;
    typename HandleTraits<Handle>::ReferenceToUInt32 mNum;
    std::vector<State> mStates;
};

template <class Handle, template <class> class State, int32_t CompileTimeNum>
inline
MultiState<Handle, State, CompileTimeNum>::MultiState(
    const Handle &inHandle,
    uint32_t inNumStates = Dynamic
) : mStorage(inHandle),
    mNum(&mStorage[0]) {
    
    uint32_t numStates = CompileTimeNum == Dynamic
        ? (inNumStates == Dynamic ? mNum : inNumStates)
        : CompileTimeNum;
    setNumStates(numStates);
}

template <class Handle, template <class> class State, int32_t CompileTimeNum>
inline
size_t
length(uint32_t inNumStates) {
    madlib_assert(CompileTimeNum == Dynamic || CompileTimeNum == inNumStates,
        std::invalid_argument());
    
    return CompileTimeNum == Dynamic
        ? 1 + inNumStates * State::length
        : compileTimeLength;
}

template <class Handle, template <class> class State, int32_t CompileTimeNum>
inline
void
setNumStates(uint32_t inNumStates) {
    madlib_assert((CompileTimeNum == Dynamic && inNumStates >= 0)
        || CompileTimeNum == inNumStates, std::invalid_argument());
    madlib_assert(mStorage.size() >= length(inNumStates),
        std::runtime_error("Insufficient storage size for MultiState."));
    
    if (mStates.size() < inNumStates) {
        for (int32_t i = mStates.size(); i < inNumStates; i++) {
            State newState(
                &mStorage[(CompileTimeNum == Dynamic) + i * State::length]);
            
            mStates.insert(newState);
        }
    } else {
        mStates.resize(inNumStates);
    }
}

template <class Handle, template <class> class State, int32_t CompileTimeNum>
inline
size_t
MultiState<Handle, State, CompileTimeNum>::numStates() {
    return CompileTimeNum == Dynamic
        ? mNum
        : CompileTimeNum;
}

template <class Handle, template <class> class State, int32_t CompileTimeNum>
inline
void
MultiState<Handle, State, CompileTimeNum>::operator=(
    const MultiState& inState
) {
    setNumStates(inState.mStates.size());
    std::copy(other.mStates.begin(), other.mStates.end(), mStates.begin());
}

template <class Handle, template <class> class State, int32_t CompileTimeNum>
inline
State& state(uint32_t inState) {
    return mStates.at(inState);
}


} // namespace modules

} // namespace madlib
