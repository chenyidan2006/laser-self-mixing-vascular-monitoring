/*
 * MATLAB Compiler: 8.6 (R2023a)
 * Date: Mon Mar 30 17:55:17 2026
 * Arguments: "-B""macro_default""-W""lib:libBP""-T""link:lib""predictBP.m""-C"
 */

#ifndef libBP_h
#define libBP_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#ifdef __cplusplus
extern "C" { // sbcheck:ok:extern_c
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_libBP_C_API 
#define LIB_libBP_C_API /* No special import/export declaration */
#endif

/* GENERAL LIBRARY FUNCTIONS -- START */

extern LIB_libBP_C_API 
bool MW_CALL_CONV libBPInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_libBP_C_API 
bool MW_CALL_CONV libBPInitialize(void);
extern LIB_libBP_C_API 
void MW_CALL_CONV libBPTerminate(void);

extern LIB_libBP_C_API 
void MW_CALL_CONV libBPPrintStackTrace(void);

/* GENERAL LIBRARY FUNCTIONS -- END */

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_libBP_C_API 
bool MW_CALL_CONV mlxPredictBP(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

/* C INTERFACE -- MLF WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_libBP_C_API bool MW_CALL_CONV mlfPredictBP(int nargout, mxArray** DBP, mxArray** SBP, mxArray* signal, mxArray* fs);

#ifdef __cplusplus
}
#endif
/* C INTERFACE -- MLF WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

#endif
