/* ----------------------------------------------------------------------- *//**
 *
 * @file ByteStringHandle_impl.hpp
 *
 *//* ----------------------------------------------------------------------- */

// Workaround for Doxygen: Ignore if not included by dbconnector.hpp
#ifdef MADLIB_DBCONNECTOR_HPP

inline
AbstractionLayer::ByteStringHandle::ByteStringHandle(const bytea* inByteString)
  : mByteString(inByteString) { }

inline 
const int8_t*
AbstractionLayer::ByteStringHandle::ptr() const {
    return reinterpret_cast<int8_t*>(VARDATA(mByteString));
}

inline 
size_t
AbstractionLayer::ByteStringHandle::size() const {
    return VARSIZE(mByteString);
}

inline 
const bytea*
AbstractionLayer::ByteStringHandle::byteString() const {
    return mByteString;
}

inline 
const char&
AbstractionLayer::ByteStringHandle::operator[](size_t inIndex) const {
    return ptr()[inIndex];
}

inline 
T*
AbstractionLayer::MutableByteStringHandle::ptr() {
    return const_cast<int8_t*>(static_cast<const ByteStringHandle*>(this)->ptr());
}

inline 
bytea*
AbstractionLayer::MutableByteStringHandle::byteString() {
    return const_cast< bytea* >(Base::mByteString);
}

inline 
char&
AbstractionLayer::MutableByteStringHandle::operator[](size_t inIndex) {
    return const_cast<char&>(
        static_cast<const ByteStringHandle*>(this)->operator[](inIndex)
    );
}

inline
AbstractionLayer::MutableByteStringHandle&
AbstractionLayer::MutableByteStringHandle::operator=(
    const MutableByteStringHandle &inByteString) {
    
    
}


#endif // MADLIB_DBCONNECTOR_HPP (workaround for Doxygen)
