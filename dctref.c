/*
    2  * reference discrete cosine transform (double precision)
    3  * Copyright (C) 2009 Dylan Yudaken
    4  *
    5  * This file is part of FFmpeg.
    6  *
    7  * FFmpeg is free software; you can redistribute it and/or
    8  * modify it under the terms of the GNU Lesser General Public
    9  * License as published by the Free Software Foundation; either
   10  * version 2.1 of the License, or (at your option) any later version.
   11  *
   12  * FFmpeg is distributed in the hope that it will be useful,
   13  * but WITHOUT ANY WARRANTY; without even the implied warranty of
   14  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   15  * Lesser General Public License for more details.
   16  *
   17  * You should have received a copy of the GNU Lesser General Public
   18  * License along with FFmpeg; if not, write to the Free Software
   19  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
   20  */
   21 
   22 /**
   23  * @file
   24  * reference discrete cosine transform (double precision)
   25  *
   26  * @author Dylan Yudaken (dyudaken at gmail)
   27  *
   28  * @note This file could be optimized a lot, but is for
   29  * reference and so readability is better.
   30  */
   31 
   32 #include "libavutil/mathematics.h"
   33 #include "dctref.h"
   34 
   35 static double coefficients[8 * 8];
   36 
   37 /**
   38  * Initialize the double precision discrete cosine transform
   39  * functions fdct & idct.
   40  */
   41 av_cold void ff_ref_dct_init(void)
   42 {
   43     unsigned int i, j;
   44 
   45     for (j = 0; j < 8; ++j) {
   46         coefficients[j] = sqrt(0.125);
   47         for (i = 8; i < 64; i += 8) {
   48             coefficients[i + j] = 0.5 * cos(i * (j + 0.5) * M_PI / 64.0);
   49         }
   50     }
   51 }
   52 
   53 /**
   54  * Transform 8x8 block of data with a double precision forward DCT <br>
   55  * This is a reference implementation.
   56  *
   57  * @param block pointer to 8x8 block of data to transform
   58  */
   59 void ff_ref_fdct(short *block)
   60 {
   61     /* implement the equation: block = coefficients * block * coefficients' */
   62 
   63     unsigned int i, j, k;
   64     double out[8 * 8];
   65 
   66     /* out = coefficients * block */
   67     for (i = 0; i < 64; i += 8) {
   68         for (j = 0; j < 8; ++j) {
   69             double tmp = 0;
   70             for (k = 0; k < 8; ++k) {
   71                 tmp += coefficients[i + k] * block[k * 8 + j];
   72             }
   73             out[i + j] = tmp * 8;
   74         }
   75     }
   76 
   77     /* block = out * (coefficients') */
   78     for (j = 0; j < 8; ++j) {
   79         for (i = 0; i < 64; i += 8) {
   80             double tmp = 0;
   81             for (k = 0; k < 8; ++k) {
   82                 tmp += out[i + k] * coefficients[j * 8 + k];
   83             }
   84             block[i + j] = floor(tmp + 0.499999999999);
   85         }
   86     }
   87 }
   88 
   89 /**
   90  * Transform 8x8 block of data with a double precision inverse DCT <br>
   91  * This is a reference implementation.
   92  *
   93  * @param block pointer to 8x8 block of data to transform
   94  */
   95 void ff_ref_idct(short *block)
   96 {
   97     /* implement the equation: block = (coefficients') * block * coefficients */
   98 
   99     unsigned int i, j, k;
  100     double out[8 * 8];
  101 
  102     /* out = block * coefficients */
  103     for (i = 0; i < 64; i += 8) {
  104         for (j = 0; j < 8; ++j) {
  105             double tmp = 0;
  106             for (k = 0; k < 8; ++k) {
  107                 tmp += block[i + k] * coefficients[k * 8 + j];
  108             }
  109             out[i + j] = tmp;
  110         }
  111     }
  112 
  113     /* block = (coefficients') * out */
  114     for (i = 0; i < 8; ++i) {
  115         for (j = 0; j < 8; ++j) {
  116             double tmp = 0;
  117             for (k = 0; k < 64; k += 8) {
  118                 tmp += coefficients[k + i] * out[k + j];
  119             }
  120             block[i * 8 + j] = floor(tmp + 0.5);
  121         }
  122     }
  123 }

