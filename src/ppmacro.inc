/*
    The Scopes Compiler Infrastructure
    This file is distributed under the MIT License.
    See LICENSE.md for details.
*/

#ifndef SCOPES_PPMACRO_INC
#define SCOPES_PPMACRO_INC

#if defined(_MSC_VER)
#define SCOPES_PP_NARG(FN, ...) SCOPES_PP_NARG_CAT( SCOPES_PP_NARG_A, ( 0, ##__VA_ARGS__, FN ## 10, FN ## 9, FN ## 8, FN ## 7, FN ## 6, FN ## 5, FN ## 4, FN ## 3, FN ## 2, FN ## 1, FN ## 0 ) )
#define SCOPES_PP_NARG_CAT( a, b ) a b
#define SCOPES_PP_NARG_A( a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, n, ... ) n
#else
#pragma GCC diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma GCC diagnostic ignored "-Wextra-semi"
#define SCOPES_PP_NARG(FN, ...) \
    SCOPES_PP_NARG_(FN,##__VA_ARGS__,9,8,7,6,5,4,3,2,1,0)
#define SCOPES_PP_NARG_(FN,N0,N1,N2,N3,N4,N5,N6,N7,N8,N,...) FN ## N
#endif

#define SCOPES_STRINGIFY(X) #X

#define SCOPES_CAT(a, ...) SCOPES_PRIMITIVE_CAT(a, __VA_ARGS__)
#define SCOPES_PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define SCOPES_EACH_ARG0(F, TOK)
#define SCOPES_EACH_ARG1(F, TOK, A1) \
    F(0,A1)
#define SCOPES_EACH_ARG2(F, TOK, A1, A2) \
    F(0,A1) TOK() F(1,A2)
#define SCOPES_EACH_ARG3(F, TOK, A1, A2, A3) \
    F(0,A1) TOK() F(1,A2) TOK() F(2,A3)
#define SCOPES_EACH_ARG4(F, TOK, A1, A2, A3, A4) \
    F(0,A1) TOK() F(1,A2) TOK() F(2,A3) TOK() F(3,A4)
#define SCOPES_EACH_ARG5(F, TOK, A1, A2, A3, A4, A5) \
    F(0,A1) TOK() F(1,A2) TOK() F(2,A3) TOK() F(3,A4) TOK() F(4,A5)
#define SCOPES_EACH_ARG6(F, TOK, A1, A2, A3, A4, A5, A6) \
    F(0,A1) TOK() F(1,A2) TOK() F(2,A3) TOK() F(3,A4) TOK() F(4,A5) TOK() F(5,A6)
#define SCOPES_EACH_ARG7(F, TOK, A1, A2, A3, A4, A5, A6, A7) \
    F(0,A1) TOK() F(1,A2) TOK() F(2,A3) TOK() F(3,A4) TOK() F(4,A5) TOK() F(5,A6) TOK() F(6,A7)
#define SCOPES_EACH_ARG8(F, TOK, A1, A2, A3, A4, A5, A6, A7, A8) \
    F(0,A1) TOK() F(1,A2) TOK() F(2,A3) TOK() F(3,A4) TOK() F(4,A5) TOK() F(5,A6) TOK() F(6,A7) TOK() F(7,A8)
#define SCOPES_EACH_ARG9(F, TOK, A1, A2, A3, A4, A5, A6, A7, A8, A9) \
    F(0,A1) TOK() F(1,A2) TOK() F(2,A3) TOK() F(3,A4) TOK() F(4,A5) TOK() F(5,A6) TOK() F(6,A7) TOK() F(7,A8) TOK() F(8, A9)
#define SCOPES_EACH_ARG10(F, TOK, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10) \
    F(0,A1) TOK() F(1,A2) TOK() F(2,A3) TOK() F(3,A4) TOK() F(4,A5) TOK() F(5,A6) TOK() F(6,A7) TOK() F(7,A8) TOK() F(8, A9) TOK() F(9, A10)
#define SCOPES_FOREACH(F, TOK, ...) \
    SCOPES_PP_NARG(SCOPES_EACH_ARG, ##__VA_ARGS__)(F, TOK, ##__VA_ARGS__)

#define SCOPES_EXPRSEP_TOKEN() ,
#define SCOPES_STMTSEP_TOKEN() ;

#define SCOPES_FOREACH_EXPR(F, ...) SCOPES_FOREACH(F, SCOPES_EXPRSEP_TOKEN, ##__VA_ARGS__)
#define SCOPES_FOREACH_STMT(F, ...) SCOPES_FOREACH(F, SCOPES_STMTSEP_TOKEN, ##__VA_ARGS__)

#endif
