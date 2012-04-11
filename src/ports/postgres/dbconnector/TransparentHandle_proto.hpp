/* ----------------------------------------------------------------------- *//**
 *
 * @file TransparentHandle_proto.hpp
 *
 *//* ----------------------------------------------------------------------- */

// Workaround for Doxygen: Ignore if not included by dbconnector.hpp
#ifdef MADLIB_DBCONNECTOR_HPP

/**
 * @brief Handle without any meta data (essentially, a constant pointer)
 *
 * A TransparentHandle is simply a (constant) pointer. It is used whenever we
 * need a type that conforms to the handle policy, but no meta data is required.
 */
template <typename T>
class AbstractionLayer::TransparentHandle {
public:
    enum { isMutable = false };

    TransparentHandle(const T* inPtr, size_t inSize)
      : mPtr(const_cast<T*>(inPtr)), mSize(inSize) { }
    
    const T* ptr() const;
    size_t size() const;
    
protected:
    T *mPtr;
    size_t mSize;
};

/**
 * @brief Mutable handle without any meta data (essentially, a pointer)
 */
template <typename T>
class AbstractionLayer::MutableTransparentHandle
  : public AbstractionLayer::TransparentHandle<T> {

    typedef TransparentHandle<T> Base;
    
public:
    enum { isMutable = true };

    MutableTransparentHandle(T* inPtr, size_t inSize)
      : Base(inPtr, inSize) { }
    
    // Import the const version as well
    using Base::ptr;
    T* ptr();
};

#endif // MADLIB_DBCONNECTOR_HPP (workaround for Doxygen)
