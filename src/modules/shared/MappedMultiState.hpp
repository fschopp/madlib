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
    template <class> class State>
class MappedMultiState : public AbstractionLayer {
public:
    enum {
        compileTimeLength = Dynamic;
    };
    const double kMaxLoadFactor = 0.5;
    typedef std::pair<uint32_t, State::ValueType> ValueType;
    
    MappedMultiState(const Handle &inHandle, uint32_t inMaxNumStates);
    static size_t length(uint32_t inNumStates);
    
    size_t numStates();
    State<TransparentHandle>& state(uint32_t inState);
    
    void operator=(const MappedMultiState& inState);
    MultiState& operator<<(const MappedMultiState& inState);
    
protected:
    Handle mStorage;
    typename HandleTraits<Handle>::ReferenceToUInt32 mNum;
    MultiState<TransparentHandle, State> mMultiState;
    TransparentHandle<Handle::ValueType> mValueToIndexMap;
};

template <class Handle, template <class> class State>
inline
MappedMultiState<Handle, State, CompileTimeNum>::MappedMultiState(
    const Handle &inHandle,
    uint32_t inNumStates = Dynamic
) : mStorage(inHandle),
//    mNum(&mStorage[0]),
    mMultiState(mStorage.begin()),
    mValueToIndexMap(mMultiState.handle().end()) {
}

template <class Handle, template <class> class State>
inline
size_t
length(uint32_t inMaxNumStates) {
    return MultiState<TransparentHandle, State>::length(inMaxNumStates)
        + 2 * sizeof(State::ValueType) * inMaxNumStates;
}

template <class Handle, template <class> class State, int32_t CompileTimeNum>
inline
size_t
MappedMultiState<Handle, State>::numStates() {
    return mMultiState.numStates();
}

template <class Handle, template <class> class State, int32_t CompileTimeNum>
inline
void
MappedMultiState<Handle, State>::operator=(
    const MultiState& inState
) {
    mMultiState = inState.nMultiState;
    for (inState.
    
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
