/* Igroteka wasm - minimal fontconfig stub.
 * The browser has no system font database; every query resolves to a font file
 * bundled into the Emscripten filesystem. Covers exactly the API surface
 * render2dsentence.cpp uses - extend if a new caller appears. */
#ifndef IGROTEKA_FONTCONFIG_STUB_H
#define IGROTEKA_FONTCONFIG_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char FcChar8;
typedef int FcBool;

typedef struct _FcConfig FcConfig;
typedef struct _FcPattern FcPattern;

typedef enum {
    FcResultMatch = 0,
    FcResultNoMatch,
    FcResultTypeMismatch,
    FcResultNoId,
    FcResultOutOfMemory
} FcResult;

typedef enum {
    FcMatchPattern = 0,
    FcMatchFont,
    FcMatchScan
} FcMatchKind;

#define FC_FILE "file"
#define FC_FAMILY "family"

FcConfig* FcInitLoadConfigAndFonts(void);
void FcConfigDestroy(FcConfig* config);
FcPattern* FcNameParse(const FcChar8* name);
FcBool FcConfigSubstitute(FcConfig* config, FcPattern* p, FcMatchKind kind);
void FcDefaultSubstitute(FcPattern* pattern);
FcPattern* FcFontMatch(FcConfig* config, FcPattern* p, FcResult* result);
void FcPatternDestroy(FcPattern* p);
FcResult FcPatternGetString(const FcPattern* p, const char* object, int n, FcChar8** s);

#ifdef __cplusplus
}
#endif

#endif
