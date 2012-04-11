/* ----------------------------------------------------------------------- *//**
 *
 * @file TransparentHandle_impl.hpp
 *
 *//* ----------------------------------------------------------------------- */

// Workaround for Doxygen: Ignore if not included by dbconnector.hpp
#ifdef MADLIB_DBCONNECTOR_HPP

/**
 * @brief Return the (constant) pointer of this handle
 */
template <typename T>
inline
const T*
AbstractionLayer::TransparentHandle<T>::ptr() const {
    return mPtr;
}

/**
 * @brief Return the size of this handle
 */
template <typename T>
inline
const T*
AbstractionLayer::TransparentHandle<T>::ptr() const {
    return mPtr;
}

/**
 * @brief Return the pointer of this handle
 */
template <typename T>
inline
T*
AbstractionLayer::MutableTransparentHandle<T>::ptr() {
    return const_cast<T*>(static_cast<const TransparentHandle<T>*>(this)->ptr());
}

#endif // MADLIB_DBCONNECTOR_HPP (workaround for Doxygen)
