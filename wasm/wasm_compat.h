/* GeneralsX wasm - force-included libc gap shim (via -include in wasm-deps.cmake).
 * musl lacks the BSD wcslcpy/wcslcat that macOS provides natively and several
 * engine translation units call without including WWLib's stringex.h. */
#ifndef IGROTEKA_WASM_COMPAT_H
#define IGROTEKA_WASM_COMPAT_H
#ifdef __cplusplus

#include <cwchar>

#ifndef HAVE_WCSLCPY
#define HAVE_WCSLCPY 1
static inline size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dstsize) {
    size_t srclen = wcslen(src);
    if (dstsize) {
        size_t n = srclen < dstsize - 1 ? srclen : dstsize - 1;
        wmemcpy(dst, src, n);
        dst[n] = L'\0';
    }
    return srclen;
}
#endif

#ifndef HAVE_WCSLCAT
#define HAVE_WCSLCAT 1
static inline size_t wcslcat(wchar_t* dst, const wchar_t* src, size_t dstsize) {
    size_t dstlen = wcslen(dst);
    if (dstlen >= dstsize) return dstsize + wcslen(src);
    return dstlen + wcslcpy(dst + dstlen, src, dstsize - dstlen);
}
#endif

#endif /* __cplusplus */
#endif
