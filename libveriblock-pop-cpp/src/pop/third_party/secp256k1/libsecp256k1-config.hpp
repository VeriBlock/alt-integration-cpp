/**********************************************************************
 * Copyright (c) 2013, 2014 Pieter Wuille                             *
 * Distributed under the MIT software license, see the accompanying   *
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.*
 **********************************************************************/

#ifndef SECP256K1_BASIC_CONFIG_H
#define SECP256K1_BASIC_CONFIG_H

#undef USE_ASM_X86_64
#undef USE_ECMULT_STATIC_PRECOMPUTATION
#undef USE_ENDOMORPHISM
#undef USE_EXTERNAL_ASM
#undef USE_EXTERNAL_DEFAULT_CALLBACKS
#undef USE_FIELD_10X26
#undef USE_FIELD_5X52
#undef USE_FIELD_INV_BUILTIN
#undef USE_FIELD_INV_NUM
#undef USE_NUM_GMP
#undef USE_NUM_NONE
#undef USE_SCALAR_4X64
#undef USE_SCALAR_8X32
#undef USE_SCALAR_INV_BUILTIN
#undef USE_SCALAR_INV_NUM
#undef ECMULT_WINDOW_SIZE

// Set ecmult gen precision bits
#define ECMULT_GEN_PREC_BITS 4

// Set window size for ecmult precomputation
#define ECMULT_WINDOW_SIZE 15

// Define to 1 if you have the <stdint.h> header file.
#define HAVE_STDINT_H 1

// Define to 1 if you have the <stdlib.h> header file.
#define HAVE_STDLIB_H 1

// Define this symbol to use the FIELD_10X26 implementation
#define USE_FIELD_10X26 1

// Define this symbol to use the native field inverse implementation
#define USE_FIELD_INV_BUILTIN 1

// Define this symbol to use no num implementation
#define USE_NUM_NONE 1

// Define this symbol to use the 8x32 scalar implementation
#define USE_SCALAR_8X32 1

/* Define this symbol to use the native scalar inverse implementation */
#define USE_SCALAR_INV_BUILTIN 1

#endif /* SECP256K1_BASIC_CONFIG_H */
