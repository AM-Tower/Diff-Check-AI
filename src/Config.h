/****************************************************************
 * @file Config.h
 * @brief Global configuration macros for debug output.
 *
 * @author Jeffrey Scott Flesher with the help of AI
 * @version 0.6
 * @date    2025-11-07
 * @section License Unlicensed, MIT, or any.
 * @section DESCRIPTION
 * Provides a SHOW_DEBUG toggle and DEBUG_MSG() macro that
 * expands to qDebug() with function name context.
 * Usage: At top of file add: #define SHOW_DEBUG 1, then include Config.h
 * Set it to 1 to enable debug messages, or 0 to turn it off.
 ***************************************************************/
#pragma once
#include <QDebug>

/****************************************************************
 * @def SHOW_DEBUG
 * @brief Toggle debug output: set to 1 to enable, 0 to disable.
 * Define SHOW_DEBUG before including this header in .cpp files
 * to override the default.
 ***************************************************************/
#ifndef SHOW_DEBUG
    #define SHOW_DEBUG 0
#endif

/****************************************************************
 * @def DEBUG_MSG
 * @brief Macro for debug logging. Prints function name when
 * SHOW_DEBUG is enabled. Otherwise compiles to a no‑op.
 ***************************************************************/
#if SHOW_DEBUG
    // Prints function name for easier tracing
    #define DEBUG_MSG() qDebug() << __FUNCTION__ << ":"
#else
    // Compile-time no-op; keeps << chains syntactically valid
    #define DEBUG_MSG() if (true) {} else qDebug()
#endif

/************** End of Config.h **************************/
