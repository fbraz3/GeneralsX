#ifndef MSS_H
#define MSS_H

// Miles Sound System stub for macOS port
// This is a minimal stub to allow compilation

#ifdef __cplusplus
extern "C" {
#endif

// Bring in canonical Windows compatibility types (HANDLE, DWORD, U32, S32, etc.)
// so we don't redefine fundamental types and cause conflicts.
#include "../WWLib/win.h"

// Basic Miles-like handle types (use existing typedefs when available)
typedef void* HDIGDRIVER;
typedef void* HSAMPLE;
typedef void* HSTREAM;
typedef void* H3DPOBJECT;
typedef void* H3DSAMPLE;
typedef void* HDLSDEVICE;
typedef void* HMDIDRIVER;
typedef void* HSEQUENCE;
typedef void* HPROVIDER;
typedef void* HAUDIO;
typedef void* HPROENUM;
typedef void* AILLPDIRECTSOUND;

// Provider enumeration constants
#define HPROENUM_FIRST ((HPROENUM)0)

// Callback calling convention
#define AILCALLBACK

// Callback function types
typedef void (AILCALLBACK * AILSTREAMCB)(HSTREAM stream);
typedef void (AILCALLBACK * AILSAMPLECB)(HSAMPLE sample);
typedef void (AILCALLBACK * AIL3DSAMPLECB)(H3DSAMPLE sample);

// Floating point type used by Miles API
typedef float F32;

// Structure stubs
typedef struct {
    S32 format;
    void* data_ptr;
    U32 data_len;
    U32 rate;
    S32 bits;
    S32 channels;
    U32 samples;
    U32 block_size;
    void* initial_ptr;
} AILSOUNDINFO;

// Error codes
#define AIL_NO_ERROR 0

// Function stubs - all return success or do nothing
static inline void AIL_startup(void) {}
static inline void AIL_shutdown(void) {}
static inline S32 AIL_digital_configuration(void* a, void* b, void* c, void* d) { return AIL_NO_ERROR; }
static inline void AIL_set_preference(U32 a, S32 b) {}
static inline HDIGDRIVER AIL_open_digital_driver(void* a, void* b, void* c, void* d) { return (HDIGDRIVER)1; }
static inline void AIL_close_digital_driver(HDIGDRIVER a) {}
static inline HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER a) { return (HSAMPLE)1; }
static inline void AIL_release_sample_handle(HSAMPLE a) {}
static inline S32 AIL_set_sample_file(HSAMPLE a, void* b, S32 c) { return AIL_NO_ERROR; }
static inline void AIL_start_sample(HSAMPLE a) {}
static inline void AIL_stop_sample(HSAMPLE a) {}
static inline void AIL_resume_sample(HSAMPLE a) {}
static inline void AIL_end_sample(HSAMPLE a) {}
static inline U32 AIL_sample_status(HSAMPLE a) { return 0; }
static inline void AIL_set_sample_volume_pan(HSAMPLE a, F32 b, F32 c) {}
static inline void AIL_set_sample_volume(HSAMPLE a, F32 b) {}
static inline void AIL_set_sample_pan(HSAMPLE a, F32 b) {}
static inline S32 AIL_sample_buffer_ready(HSAMPLE a) { return 1; }
static inline void AIL_load_sample_buffer(HSAMPLE a, U32 b, void* c, U32 d) {}
static inline void AIL_set_file_callbacks(
    unsigned int (*open_cb)(const char*, unsigned int*),
    void (*close_cb)(unsigned int),
    signed int (*seek_cb)(unsigned int, signed int, unsigned int),
    unsigned int (*read_cb)(unsigned int, void*, unsigned int)
) {}
static inline void AIL_register_EOS_callback(HSAMPLE a, AILSAMPLECB b) {}
static inline void AIL_register_3D_EOS_callback(H3DSAMPLE a, AIL3DSAMPLECB b) {}
static inline void AIL_stop_3D_sample(H3DSAMPLE a) {}
static inline void AIL_register_stream_callback(HSTREAM a, AILSTREAMCB b) {}
static inline void AIL_pause_stream(HSTREAM a, S32 b) {}
static inline HSTREAM AIL_open_stream(HDIGDRIVER a, const char* b, S32 c) { return (HSTREAM)1; }
static inline void AIL_close_stream(HSTREAM a) {}
static inline void AIL_release_3D_sample_handle(H3DSAMPLE a) {}
static inline void AIL_resume_3D_sample(H3DSAMPLE a) {}
static inline void AIL_set_3D_sample_volume(H3DSAMPLE a, F32 b) {}
static inline void AIL_set_stream_volume_pan(HSTREAM a, F32 b, F32 c) {}
static inline void AIL_stream_volume_pan(HSTREAM a, F32* b, F32* c) {}
static inline void AIL_sample_volume_pan(HSAMPLE a, F32* b, F32* c) {}
static inline void AIL_quick_unload(void* a) {}
static inline S32 AIL_sample_playback_rate(HSAMPLE a) { return 22050; }
static inline void AIL_set_sample_playback_rate(HSAMPLE a, S32 b) {}
static inline S32 AIL_3D_sample_playback_rate(H3DSAMPLE a) { return 22050; }
static inline void AIL_set_3D_sample_playback_rate(H3DSAMPLE a, S32 b) {}
static inline void AIL_set_3D_sample_occlusion(H3DSAMPLE a, F32 b) {}
static inline S32 AIL_stream_loop_count(HSTREAM a) { return 0; }
static inline void AIL_set_redist_directory(const char* a) {}
static inline S32 AIL_quick_startup(S32 a, S32 b, S32 c, S32 d, S32 e) { return AIL_NO_ERROR; }
static inline void AIL_quick_handles(HDIGDRIVER* a, HMDIDRIVER* b, HDLSDEVICE* c) {}
static inline void AIL_get_DirectSound_info(void* a, void** b, void* c) {}
static inline S32 AIL_open_3D_provider(HPROVIDER provider_id) { return 0; }
static inline void AIL_close_3D_provider(HPROVIDER provider_id) {}
static inline void AIL_set_3D_speaker_type(HPROVIDER provider_id, U32 speaker_type) {}
static inline void AIL_close_3D_listener(void* a) {}
static inline void* AIL_open_3D_listener(void* a) { return (void*)1; }
static inline H3DSAMPLE AIL_allocate_3D_sample_handle(void* a) { return (H3DSAMPLE)1; }
static inline S32 AIL_init_sample(HSAMPLE a) { return AIL_NO_ERROR; }
static inline void AIL_set_3D_sample_distances(H3DSAMPLE a, F32 b, F32 c) {}
static inline S32 AIL_set_3D_sample_file(H3DSAMPLE a, void* b) { return AIL_NO_ERROR; }
static inline S32 AIL_set_stream_loop_count(HSTREAM a, S32 b) { (void)a; (void)b; return 0; }
static inline void AIL_start_3D_sample(H3DSAMPLE a) {}
static inline void AIL_start_stream(HSTREAM a) {}
static inline S32 AIL_stream_ms_position(HSTREAM a, void* b, void* c) { return 0; }
static inline HSAMPLE AIL_quick_load_and_play(const char* a, U32 b, S32 c) { return (HSAMPLE)1; }
static inline void AIL_quick_set_volume(HSAMPLE a, F32 b, F32 c) {}
static inline S32 AIL_WAV_info(void* a, AILSOUNDINFO* b) { return AIL_NO_ERROR; }
static inline S32 AIL_decompress_ADPCM(AILSOUNDINFO* a, void** b, U32* c) { return AIL_NO_ERROR; }
static inline void AIL_mem_free_lock(void* a) {}
static inline void AIL_set_3D_user_data(H3DSAMPLE a, U32 b, U32 c) {}
static inline void AIL_set_sample_user_data(HSAMPLE a, U32 b, U32 c) {}
static inline S32 AIL_enumerate_3D_providers(HPROENUM* a, HPROVIDER* b, char** c) { return 0; }
static inline S32 AIL_enumerate_filters(HPROENUM* a, HPROVIDER* b, char** c) { return 0; }

// Sample-specific helpers that WWAudio expects. Provide thin stubs that map
// to the generic implementations above or no-op so code compiles.
static inline S32 AIL_set_named_sample_file(HSAMPLE s, char *name, void *buffer, U32 length, S32 flags) { (void)s; (void)name; (void)buffer; (void)length; (void)flags; return 0; }
static inline void AIL_set_sample_loop_count(HSAMPLE s, U32 count) { (void)s; (void)count; }
static inline U32 AIL_sample_loop_count(HSAMPLE s) { (void)s; return 0; }
static inline void AIL_set_sample_ms_position(HSAMPLE s, U32 ms) { (void)s; (void)ms; }
static inline void AIL_sample_ms_position(HSAMPLE s, S32 *len, S32 *pos) { if (len) *len = 0; if (pos) *pos = 0; (void)s; }

// Additional 3D functions used by WWAudio
static inline void AIL_set_3D_velocity_vector(H3DSAMPLE sample, F32 x, F32 y, F32 z) {}
static inline void AIL_set_3D_sample_effects_level(H3DSAMPLE sample, F32 level) {}

// Wave format constants
#define WAVE_FORMAT_IMA_ADPCM 0x0011

// Constants for speaker types
#define AIL_3D_2_SPEAKER  0
#define AIL_3D_4_SPEAKER  1
#define AIL_3D_51_SPEAKER 2
#define AIL_3D_71_SPEAKER 3
#define AIL_3D_HEADPHONE  4
#define AIL_3D_SURROUND   5

#ifdef __cplusplus
}
#endif

#endif // MSS_H
