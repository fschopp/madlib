/* ----------------------------------------------------------------------- *//**
 *
 * @file ByteStringHandle_proto.hpp
 *
 *//* ----------------------------------------------------------------------- */
 
// Workaround for Doxygen: Ignore if not included by dbconnector.hpp
#ifdef MADLIB_DBCONNECTOR_HPP

template <typename T>
class AbstractionLayer::ByteStringHandle {
public:
    enum { isMutable = false };

    ByteStringHandle(const bytea* inByteString);
    
    const char* ptr() const;
    size_t size() const;
    const bytea* byteString() const;
    const char* operator[](size_t inIndex) const;

protected:
    const bytea* mByteString;
};


template <typename T>
class AbstractionLayer::MutableByteStringHandle
  : public AbstractionLayer::ByteStringHandle {

    typedef ByteStringHandle<T> Base;
    
public:
    enum { isMutable = true };

    MutableByteStringHandle(bytea* inByteString)
      : Base(inByteString) { }
    
    using Base::ptr;
    using Base::byteString;
    
    char* ptr();
    bytea* byteString();
    char* operator[](size_t inIndex);
    MutableByteStringHandle &operator=(
        const MutableByteStringHandle &inByteString);
};

#endif // MADLIB_DBCONNECTOR_HPP (workaround for Doxygen)
