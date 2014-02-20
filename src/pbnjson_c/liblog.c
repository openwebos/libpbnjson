// @@@LICENSE
//
//      Copyright (c) 2009-2014 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// LICENSE@@@

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <alloca.h>
#include <sys_malloc.h>
#include <compiler/detection.h>
#include <compiler/pure_attribute.h>
#include <pjson_syslog.h>
#include <libgen.h>
#include <strnlen.h>
#include <isatty.h>

#include "liblog.h"

#if LOG_INFO < LOG_ERR
	// increasing numbers indicate higher priority
	#define IS_HIGHER_PRIORITY(actual, base) ((actual) > (base))
#else
	// decreasing numbers indicate higher priority
	#define IS_HIGHER_PRIORITY(actual, base) ((actual) < (base))
#endif

#if HAVE_VFPRINTF
#define VFPRINTF(priority, file, format, ap)                        \
	do {                                                            \
		vfprintf(file, format, ap);                                 \
		if (IS_HIGHER_PRIORITY(priority, LOG_INFO))                 \
			fflush(file);                                           \
	} while (0)
#else
#define VFPRINTF(priority, file, format, ap) PJSON_NOOP
#endif

static const char *program_name = NULL;
static bool default_program_name = true;

#if GCC_VERSION >= __PJ_APP_VERSION(2, 96, 0)
#define PURE_FUNCTION __attribute__((pure))
#else
#define PURE_FUNCTION
#endif /* PURE_FUNCTION */

#define SAFE_STRING_PRINT(str) ((str) != NULL ? (str) : "(null)")

#if HAVE_VSYSLOG
static int PmLogLevelToSyslog(PmLogLevel level)
{
	switch (level)
	{
	case kPmLogLevel_Critical:
		return LOG_CRIT;
	case kPmLogLevel_Error:
		return LOG_ERR;
	case kPmLogLevel_Warning:
		return LOG_WARNING;
	case kPmLogLevel_Info:
		return LOG_INFO;
	case kPmLogLevel_Debug:
	default:
		return LOG_DEBUG;
	}
}
#endif

void setConsumerName(const char *name)
{
	PJ_LOG_DBG("PBNJSON_CHANGE_NAME", 1, PMLOGKS("APPID", program_name), "changing program name to %s", SAFE_STRING_PRINT(program_name));
	if (default_program_name)
		free((char *)program_name);
	program_name = name;
	default_program_name = name != NULL;
}

const char *getConsumerName()
{
	if (default_program_name)
		return NULL;
	return program_name;
}

static size_t setProgNameUnknown(char *buffer, size_t bufferSize) PURE_FUNC;
static size_t setProgNameUnknown(char *buffer, size_t bufferSize)
{
#define DEFAULT_UNKNOWN_CMDLINE "unknown process name"
	snprintf(buffer, bufferSize, "%s", DEFAULT_UNKNOWN_CMDLINE);
	return sizeof(DEFAULT_UNKNOWN_CMDLINE);
#undef DEFAULT_UNKNOWN_CMDLINE
}

static const char *getConsumerName_internal()
{
	pid_t proc_pid;
	char path[80];
	char cmdline[1024];
	char *program = cmdline;
	size_t cmdline_size;
	size_t prog_name_size;
	FILE *cmdline_file;
	char *dyn_program_name;

	if (program_name)
		return program_name;

	assert (default_program_name);

	proc_pid = getpid();
	snprintf(path, sizeof(path), "/proc/%d/cmdline", (int)proc_pid);
	cmdline_file = fopen(path, "r");
	if (cmdline_file == NULL) {
		cmdline_size = setProgNameUnknown(cmdline, sizeof(cmdline));
	} else {
		cmdline_size = fread(cmdline, sizeof(cmdline[0]), sizeof(cmdline) - 1, cmdline_file);
		if (cmdline_size) {
			cmdline_size--;
			cmdline[cmdline_size] = 0;
			program = basename(cmdline);
			cmdline_size = strnlen(cmdline, cmdline_size);
		}
		else
			cmdline_size = setProgNameUnknown(cmdline, sizeof(cmdline));
		fclose(cmdline_file);
	}

	prog_name_size = cmdline_size + 10;	// 10 characters for pid & null character just in case
	dyn_program_name = (char *)malloc(prog_name_size);
	if (dyn_program_name) {
		snprintf((char *)dyn_program_name, prog_name_size, "%d (%s)", (int) proc_pid, program);
	}

	return (program_name = dyn_program_name);
}

PmLogContext PmLogGetLibContext()
{
	return kPmLogDefaultContext;
}

PmLogErr _PmLogMsgKV(PmLogContext context, PmLogLevel level, unsigned int flags,
                     const char *msgid, size_t kv_count, const char *check_keywords,
                     const char *check_formats, const char *fmt, ...)
{
#if HAVE_LOG_TARGET
	static int using_terminal = -1;
	if (using_terminal == -1)
	{
#if defined HAVE_VSYSLOG
#if defined HAVE_VFPRINTF && defined HAVE_ISATTY
		using_terminal = isatty(fileno(stderr));
#else
		using_terminal = 0;
#endif
#elif defined HAVE_VFPRINTF
		using_terminal = 1;
#else
		using_terminal = 0;
#endif
	}

#define LOG_PREAMBLE "%s PBNJSON :: "

	va_list ap;
	va_start(ap, fmt);

	// TODO: memoize the program name string length
	size_t messageLen = strlen(fmt) + strlen(msgid) + 4 /* line number */ + 100 /* chars for message */;
	const char *programNameToPrint = getConsumerName_internal();
	size_t formatLen = messageLen + sizeof(LOG_PREAMBLE) + (using_terminal ? 1 : 0) + strlen(programNameToPrint);
	char format[formatLen];
	snprintf(format, formatLen, LOG_PREAMBLE "%s%s%s", programNameToPrint, msgid, fmt, using_terminal ? "\n" : "");

#if HAVE_VSYSLOG
	if (LIKELY(!using_terminal)) {
		vsyslog(PmLogLevelToSyslog(level), format, ap);
	} else {
		VFPRINTF(level, stderr, format, ap);
	}
#elif HAVE_VFPRINTF
	VFPRINTF(level, stderr, format, ap);
#else
#error Logging mechanism not implemented
#endif

	va_end(ap);
#endif /* HAVE_LOG_TARGET */

	return kPmLogErr_None;
}
