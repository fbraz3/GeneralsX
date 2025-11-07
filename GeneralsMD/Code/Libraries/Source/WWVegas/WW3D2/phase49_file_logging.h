/*
**	Phase 49: File-Based Logging System for DX8Wrapper Initialization Tracing
**	
**	This header provides robust file-based logging with timestamps and immediate flushing
**	to trace execution through the initialization sequence and pinpoint hangs.
*/

#ifndef __PHASE49_FILE_LOGGING_H__
#define __PHASE49_FILE_LOGGING_H__

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>

// Global file handle for Phase 49 logging
static FILE* g_phase49_logfile = NULL;

/**
 * Initialize the Phase 49 logging system
 * Opens the log file at /tmp/phase49_crash.log for writing
 */
inline void Phase49_Init_Logging() {
	if (g_phase49_logfile == NULL) {
		// Try /tmp first, if not accessible use home directory
		const char* log_path = getenv("HOME");
		char full_log_path[256];
		
		if (log_path != NULL) {
			snprintf(full_log_path, sizeof(full_log_path), "%s/phase49_crash.log", log_path);
		} else {
			strcpy(full_log_path, "/tmp/phase49_crash.log");
		}
		
		g_phase49_logfile = fopen(full_log_path, "w");
		if (g_phase49_logfile != NULL) {
			fprintf(g_phase49_logfile, "=== Phase 49 DX8Wrapper Initialization Logging ===\n");
			fprintf(g_phase49_logfile, "Log file: %s\n", full_log_path);
			fprintf(g_phase49_logfile, "Session start: %s\n", __TIMESTAMP__);
			fflush(g_phase49_logfile);
		} else {
			printf("ERROR: Failed to open log file at %s\n", full_log_path);
		}
	}
}

/**
 * Shutdown the Phase 49 logging system
 * Closes the log file
 */
inline void Phase49_Shutdown_Logging() {
	if (g_phase49_logfile != NULL) {
		fprintf(g_phase49_logfile, "=== Phase 49 Logging Session End ===\n");
		fflush(g_phase49_logfile);
		fclose(g_phase49_logfile);
		g_phase49_logfile = NULL;
	}
}

/**
 * Get current timestamp as a string
 */
inline const char* Phase49_Get_Timestamp() {
	static thread_local char timestamp_buf[32];
	time_t now = time(NULL);
	struct tm* timeinfo = localtime(&now);
	strftime(timestamp_buf, sizeof(timestamp_buf), "[%H:%M:%S]", timeinfo);
	return timestamp_buf;
}

/**
 * Main logging macro - writes to file with timestamp and immediate flush
 * Usage: PHASE49_LOG("Message with %s format", "strings");
 */
#define PHASE49_LOG(format, ...) do { \
	if (g_phase49_logfile != NULL) { \
		fprintf(g_phase49_logfile, "%s ", Phase49_Get_Timestamp()); \
		fprintf(g_phase49_logfile, format, ##__VA_ARGS__); \
		fprintf(g_phase49_logfile, "\n"); \
		fflush(g_phase49_logfile); \
	} \
} while(0)

/**
 * Entry/exit logging macros - mark function boundaries
 */
#define PHASE49_LOG_ENTER(funcname) \
	PHASE49_LOG(">>> ENTER: %s", funcname)

#define PHASE49_LOG_EXIT(funcname) \
	PHASE49_LOG("<<< EXIT: %s", funcname)

/**
 * Checkpoint logging - mark specific checkpoints
 */
#define PHASE49_CHECKPOINT(id, desc) \
	PHASE49_LOG("CHECKPOINT %s: %s", id, desc)

/**
 * Error logging - mark errors with context
 */
#define PHASE49_LOG_ERROR(desc) \
	PHASE49_LOG("!!! ERROR: %s (line %d in %s)", desc, __LINE__, __FUNCTION__)

/**
 * Separator for logical sections
 */
#define PHASE49_LOG_SEPARATOR(section) \
	PHASE49_LOG("--- %s ---", section)

#endif // __PHASE49_FILE_LOGGING_H__
