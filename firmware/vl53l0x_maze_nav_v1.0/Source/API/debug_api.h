#ifndef SOURCE_API_DEBUG_API_H_
#define SOURCE_API_DEBUG_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "uart_baudrate.h"
#include "uart_api.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define CREATE_MODULE_NAME(file_name) static const char *trace_module_name = #file_name;
#define TRACE_INFO(format, ...) Debug_API_Print(eTraceLevel_Info, trace_module_name,__FILE__, __LINE__, format, ##__VA_ARGS__)
#define TRACE_WRN(format, ...) Debug_API_Print(eTraceLevel_Warning, trace_module_name,__FILE__, __LINE__, format, ##__VA_ARGS__)
#define TRACE_ERR(format, ...) Debug_API_Print(eTraceLevel_Error, trace_module_name, __FILE__, __LINE__, format, ##__VA_ARGS__)

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eTraceLevel {
    eTraceLevel_First = 0,
    eTraceLevel_Info = eTraceLevel_First,
    eTraceLevel_Warning,
    eTraceLevel_Error,
    eTraceLevel_Last
} eTraceLevel_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Debug_API_Init (const eUartBaudrate_t baudrate);
bool Debug_API_Print (const eTraceLevel_t trace_level, const char *file_trace, const char *file_name, const size_t line_number, const char *format, ...);

#endif /* SOURCE_API_DEBUG_API_H_ */
