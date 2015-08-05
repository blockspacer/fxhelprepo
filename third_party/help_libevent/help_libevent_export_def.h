#ifndef _HELP_LIBEVENT_EXPORT_DEF_H_
#define _HELP_LIBEVENT_EXPORT_DEF_H_

#ifdef HELP_LIBEVENT_EXPORTS
#define HELP_LIBEVENT_API __declspec(dllexport)
#else
#define HELP_LIBEVENT_API __declspec(dllimport)
#endif

#endif