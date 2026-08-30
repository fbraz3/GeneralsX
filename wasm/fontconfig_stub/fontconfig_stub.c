/* Igroteka wasm - fontconfig stub implementation.
 * Every font query resolves to the single font bundled at /fonts/default.ttf
 * in the Emscripten virtual filesystem (packaged at build time). */
#include <fontconfig/fontconfig.h>

#include <stdlib.h>
#include <string.h>

static struct _FcConfig { int unused; } g_config;
static struct _FcPattern { int unused; } g_pattern;

static FcChar8 g_fontPath[] = "/fonts/default.ttf";

FcConfig* FcInitLoadConfigAndFonts(void) { return (FcConfig*)&g_config; }
void FcConfigDestroy(FcConfig* config) { (void)config; }

FcPattern* FcNameParse(const FcChar8* name) {
    (void)name; /* single-font world: family/style requests all map to the bundle */
    return (FcPattern*)&g_pattern;
}

FcBool FcConfigSubstitute(FcConfig* config, FcPattern* p, FcMatchKind kind) {
    (void)config; (void)p; (void)kind;
    return 1;
}

void FcDefaultSubstitute(FcPattern* pattern) { (void)pattern; }

FcPattern* FcFontMatch(FcConfig* config, FcPattern* p, FcResult* result) {
    (void)config; (void)p;
    if (result) *result = FcResultMatch;
    return (FcPattern*)&g_pattern;
}

void FcPatternDestroy(FcPattern* p) { (void)p; }

FcResult FcPatternGetString(const FcPattern* p, const char* object, int n, FcChar8** s) {
    (void)p; (void)n;
    if (s && object && strcmp(object, FC_FILE) == 0) {
        *s = g_fontPath;
        return FcResultMatch;
    }
    return FcResultNoMatch;
}
