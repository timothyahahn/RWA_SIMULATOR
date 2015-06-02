//
// MATLAB Compiler: 4.6 (R2007a)
// Date: Wed May 06 10:19:56 2009
// Arguments: "-B" "macro_default" "-W" "cpplib:nonlinear" "-T" "link:lib"
// "build_libxpm_database.m" "load_libxpm_database.m" "check_last_inputs.m"
// "-v" 
//

#ifndef __nonlinear_h
#define __nonlinear_h 1

#if defined(__cplusplus) && !defined(mclmcr_h) && defined(__linux__)
#  pragma implementation "mclmcr.h"
#endif
#include "mclmcr.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_nonlinear
#define PUBLIC_nonlinear_C_API __global
#else
#define PUBLIC_nonlinear_C_API /* No import statement needed. */
#endif

#define LIB_nonlinear_C_API PUBLIC_nonlinear_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_nonlinear
#define PUBLIC_nonlinear_C_API __declspec(dllexport)
#else
#define PUBLIC_nonlinear_C_API __declspec(dllimport)
#endif

#define LIB_nonlinear_C_API PUBLIC_nonlinear_C_API


#else

#define LIB_nonlinear_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_nonlinear_C_API 
#define LIB_nonlinear_C_API /* No special import/export declaration */
#endif

extern LIB_nonlinear_C_API 
bool MW_CALL_CONV nonlinearInitializeWithHandlers(mclOutputHandlerFcn error_handler,
                                                  mclOutputHandlerFcn print_handler);

extern LIB_nonlinear_C_API 
bool MW_CALL_CONV nonlinearInitialize(void);

extern LIB_nonlinear_C_API 
void MW_CALL_CONV nonlinearTerminate(void);


extern LIB_nonlinear_C_API 
bool MW_CALL_CONV mlxBuild_libxpm_database(int nlhs, mxArray *plhs[],
                                           int nrhs, mxArray *prhs[]);

extern LIB_nonlinear_C_API 
bool MW_CALL_CONV mlxLoad_libxpm_database(int nlhs, mxArray *plhs[],
                                          int nrhs, mxArray *prhs[]);

extern LIB_nonlinear_C_API 
bool MW_CALL_CONV mlxCheck_last_inputs(int nlhs, mxArray *plhs[],
                                       int nrhs, mxArray *prhs[]);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_nonlinear
#define PUBLIC_nonlinear_CPP_API __declspec(dllexport)
#else
#define PUBLIC_nonlinear_CPP_API __declspec(dllimport)
#endif

#define LIB_nonlinear_CPP_API PUBLIC_nonlinear_CPP_API

#else

#if !defined(LIB_nonlinear_CPP_API)
#if defined(LIB_nonlinear_C_API)
#define LIB_nonlinear_CPP_API LIB_nonlinear_C_API
#else
#define LIB_nonlinear_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_nonlinear_CPP_API void MW_CALL_CONV build_libxpm_database(const mwArray& fs
                                                                     , const mwArray& channel_power
                                                                     , const mwArray& D
                                                                     , const mwArray& alphaDB
                                                                     , const mwArray& gam
                                                                     , const mwArray& res_Disp
                                                                     , const mwArray& HalfWindow);

extern LIB_nonlinear_CPP_API void MW_CALL_CONV load_libxpm_database(int nargout
                                                                    , mwArray& matrix);

extern LIB_nonlinear_CPP_API void MW_CALL_CONV check_last_inputs(int nargout
                                                                 , mwArray& identical
                                                                 , const mwArray& fs_new
                                                                 , const mwArray& channel_power_new
                                                                 , const mwArray& D_new
                                                                 , const mwArray& alphaDB_new
                                                                 , const mwArray& gam_new
                                                                 , const mwArray& res_Disp_new
                                                                 , const mwArray& HalfWindow_new);

#endif

#endif
