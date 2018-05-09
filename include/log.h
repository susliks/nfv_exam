/*=============================================================================
# Author: stzhuangyi - stzhuangyi@outlook.com
# Create time: 2018-01-22 14:06
# Last modified: 2018-01-22 14:06
# Filename: log.h
# Description: ---
=============================================================================*/
#ifndef LOG_H
#define LOG_H

//#define DEBUG
//#define NOTICE

#define filename(x) strrchr(x,'/')?strrchr(x,'/')+1:x   //only select the filename from __FILE__

#include <string>

#define UNIT_LOG(level, format, args...) \
    rs_frame::log_debug( \
            std::string(std::string("[%s] [%s %s] [%s:%s:%d] ") + format + "\n").c_str(), \
            level, \
            __DATE__, \
            __TIME__, \
            filename(__FILE__), \
            __FUNCTION__, \
            __LINE__, \
            ##args)
#ifdef DEBUG
#define debug_log(fmt, args...) UNIT_LOG("DEBUG", fmt, ##args)
#define notice_log(fmt, args...) UNIT_LOG("NOTICE", fmt, ##args)
#else
#define debug_log(fmt, args...) ;
#ifdef NOTICE
#define notice_log(fmt, args...) UNIT_LOG("NOTICE", fmt, ##args)
#else
#define notice_log(fmt, args...) ;
#endif
#endif

#define warning_log(fmt, args...) UNIT_LOG("WARNING", fmt, ##args)
#define fatal_log(fmt, args...) UNIT_LOG("FATAL", fmt, ##args)

namespace rs_frame {

void log_debug(const char *fmt, ...);

} // end of namespace rs_frame

#endif

