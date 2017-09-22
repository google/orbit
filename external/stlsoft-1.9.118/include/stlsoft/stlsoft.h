/* /////////////////////////////////////////////////////////////////////////
 * File:        stlsoft/stlsoft.h
 *
 * Purpose:     Root header for the STLSoft libraries. Performs various
 *              compiler and platform discriminations, and definitions of
 *              types.
 *
 * Created:     15th January 2002
 * Updated:     31st May 2014
 *
 * Home:        http://stlsoft.org/
 *
 * Copyright (c) 2002-2014, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the
 *   names of any contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */


#ifndef STLSOFT_INCL_STLSOFT_H_STLSOFT
#define STLSOFT_INCL_STLSOFT_H_STLSOFT
#define STLSOFT_INCL_H_STLSOFT  /*!< \brief Definition of previous include-guard symbol for stlsoft/stlsoft.h, for backwards compatibility. */

#if defined(__STLSOFT_DOCUMENTATION_SKIP_SECTION) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define STLSOFT_DOCUMENTATION_SKIP_SECTION
#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION? */

/* File version */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_VER_STLSOFT_H_STLSOFT_MAJOR    3
# define STLSOFT_VER_STLSOFT_H_STLSOFT_MINOR    30
# define STLSOFT_VER_STLSOFT_H_STLSOFT_REVISION 1
# define STLSOFT_VER_STLSOFT_H_STLSOFT_EDIT     435
#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */
/* # include "./internal/doxygen_defs.h" */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \file stlsoft/stlsoft.h
 *
 * \brief [C, C++] The root header for the
 *   \ref group__project__stlsoft "STLSoft" project, and for all other
 *    \ref group__projects "projects".
 */

/* /////////////////////////////////////////////////////////////////////////
 * STLSoft version
 *
 * The libraries version information is comprised of major, minor and
 * revision components.
 *
 * Each release of the libraries will bear a different version, and that
 * version will also have its own symbol: Version 1.0.1 specifies
 * _STLSOFT_VER_1_0_1.
 *
 * Thus the symbol _STLSOFT_VER may be compared meaningfully with a specific
 * version symbol, e.g.# if _STLSOFT_VER >= _STLSOFT_VER_1_0_1
 */

/** \def _STLSOFT_VER_MAJOR
 * \brief The major version number of STLSoft
 *
 * A change to the major version component implies that a dramatic change
 * has occurred in the libraries, such that considerable changes to source
 * dependent on previous versions would need to be effected.
 */

/** \def _STLSOFT_VER_MINOR
 * \brief The minor version number of STLSoft
 *
 * A change to the minor version component imply that a significant change
 * has occurred to the libraries, either in the addition of new functionality
 * or in the destructive change to one or more components such that
 * recompilation and code change may be necessitated.
 */

/** \def _STLSOFT_VER_REVISION
 * \brief The revision version number of STLSoft
 *
 * A change to the revision version component imply that a bug has been
 * fixed. Dependent code should be recompiled in order to pick up the
 * changes.
 */

/** \def _STLSOFT_VER
 * \brief The current composite version number of STLSoft
 *
 * In addition to the individual version symbols - _STLSOFT_VER_MAJOR,
 * _STLSOFT_VER_MINOR and _STLSOFT_VER_REVISION - a composite symbol
 * _STLSOFT_VER is defined, where:
 *  - bits 24-31: the major version
 *  - bits 16-23: the minor version
 *  - bits 8-15: the revision version
 *  - bits 0-7: the beta number; if not a beta, it is 0xFF
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define _STLSOFT_VER_1_0_1     0x00010001  /*!< Version 1.0.1 */
# define _STLSOFT_VER_1_0_2     0x00010002  /*!< Version 1.0.2 */
# define _STLSOFT_VER_1_1_1     0x00010101  /*!< Version 1.1.1 */
# define _STLSOFT_VER_1_1_2     0x00010102  /*!< Version 1.1.2 */
# define _STLSOFT_VER_1_1_3     0x00010103  /*!< Version 1.1.3 */
# define _STLSOFT_VER_1_2_1     0x00010201  /*!< Version 1.2.1 */
# define _STLSOFT_VER_1_3_1     0x00010301  /*!< Version 1.3.1 */
# define _STLSOFT_VER_1_3_2     0x00010302  /*!< Version 1.3.2 */
# define _STLSOFT_VER_1_4_1     0x00010401  /*!< Version 1.4.1 */
# define _STLSOFT_VER_1_4_2     0x00010402  /*!< Version 1.4.2 */
# define _STLSOFT_VER_1_4_3     0x00010403  /*!< Version 1.4.3 */
# define _STLSOFT_VER_1_4_4     0x00010404  /*!< Version 1.4.4 */
# define _STLSOFT_VER_1_4_5     0x00010405  /*!< Version 1.4.5 */
# define _STLSOFT_VER_1_4_6     0x00010406  /*!< Version 1.4.6 */
# define _STLSOFT_VER_1_5_1     0x00010501  /*!< Version 1.5.1 */
# define _STLSOFT_VER_1_5_2     0x00010502  /*!< Version 1.5.2 */
# define _STLSOFT_VER_1_6_1     0x00010601  /*!< Version 1.6.1 */
# define _STLSOFT_VER_1_6_2     0x00010602  /*!< Version 1.6.2 */
# define _STLSOFT_VER_1_6_3     0x00010603  /*!< Version 1.6.3 */
# define _STLSOFT_VER_1_6_4     0x00010604  /*!< Version 1.6.4 */
# define _STLSOFT_VER_1_6_5     0x00010605  /*!< Version 1.6.5 */
# define _STLSOFT_VER_1_6_6     0x00010606  /*!< Version 1.6.6 */
# define _STLSOFT_VER_1_7_1     0x00010701  /*!< Version 1.7.1 */
# define _STLSOFT_VER_1_7_2     0x00010702  /*!< Version 1.7.2 */
# define _STLSOFT_VER_1_8_1     0x00010801  /*!< Version 1.8.1 */
# define _STLSOFT_VER_1_8_2     0x00010802  /*!< Version 1.8.2 */
# define _STLSOFT_VER_1_8_3     0x00010803  /*!< Version 1.8.3 */
# define _STLSOFT_VER_1_8_4     0x00010804  /*!< Version 1.8.4 */
# define _STLSOFT_VER_1_8_5     0x00010805  /*!< Version 1.8.5 */
# define _STLSOFT_VER_1_8_6     0x00010806  /*!< Version 1.8.6 */
# define _STLSOFT_VER_1_8_7     0x00010807  /*!< Version 1.8.7 */
# define _STLSOFT_VER_1_8_8     0x00010808  /*!< Version 1.8.8 */
# define _STLSOFT_VER_1_8_9     0x00010809  /*!< Version 1.8.9 */
# define _STLSOFT_VER_1_9_1_B13 0x0109010d  /*!< Version 1.9.1 beta 13 (10th Jul 2006) */
# define _STLSOFT_VER_1_9_1_B20 0x01090114  /*!< Version 1.9.1 beta 20 (28th Aug 2006) */
# define _STLSOFT_VER_1_9_1_B22 0x01090116  /*!< Version 1.9.1 beta 22 (13th Sep 2006) */
# define _STLSOFT_VER_1_9_1_B28 0x0109011c  /*!< Version 1.9.1 beta 28 (29th Oct 2006) */
# define _STLSOFT_VER_1_9_1_B29 0x0109011d  /*!< Version 1.9.1 beta 29 (27th Nov 2006) */
# define _STLSOFT_VER_1_9_1_B33 0x01090121  /*!< Version 1.9.1 beta 33 (13th Dec 2006) */
# define _STLSOFT_VER_1_9_1_B34 0x01090122  /*!< Version 1.9.1 beta 34 (24th Dec 2006) */
# define _STLSOFT_VER_1_9_1_B37 0x01090125  /*!< Version 1.9.1 beta 37 (30th Dec 2006) */
# define _STLSOFT_VER_1_9_1_B40 0x01090128  /*!< Version 1.9.1 beta 40 (6th Jan 2007) */
# define _STLSOFT_VER_1_9_1_B41 0x01090129  /*!< Version 1.9.1 beta 41 (14th Jan 2007) */
# define _STLSOFT_VER_1_9_1_B42 0x0109012a  /*!< Version 1.9.1 beta 42 (15th Jan 2007) */
# define _STLSOFT_VER_1_9_1_B45 0x0109012d  /*!< Version 1.9.1 beta 45 (1st Mar 2007) */
# define _STLSOFT_VER_1_9_1_B47 0x0109012f  /*!< Version 1.9.1 beta 47 (6th Apr 2007) */
# define _STLSOFT_VER_1_9_1_B48 0x01090130  /*!< Version 1.9.1 beta 48 (12th Apr 2007) */
# define _STLSOFT_VER_1_9_1     0x010901ff  /*!< Version 1.9.1 (30th Apr 2007) */
# define _STLSOFT_VER_1_9_2     0x010902ff  /*!< Version 1.9.2 (2nd Jun 2007) */
# define _STLSOFT_VER_1_9_3     0x010903ff  /*!< Version 1.9.3 (29th Jul 2007) */
# define _STLSOFT_VER_1_9_4     0x010904ff  /*!< Version 1.9.4 (2nd Aug 2007) */
# define _STLSOFT_VER_1_9_5     0x010905ff  /*!< Version 1.9.5 (4th Aug 2007) */
# define _STLSOFT_VER_1_9_6     0x010906ff  /*!< Version 1.9.6 (6th Aug 2007) */
# define _STLSOFT_VER_1_9_7     0x010907ff  /*!< Version 1.9.7 (16th Nov 2007) */
# define _STLSOFT_VER_1_9_8     0x010908ff  /*!< Version 1.9.8 (18th Nov 2007) */
# define _STLSOFT_VER_1_9_9     0x010909ff  /*!< Version 1.9.9 (19th Nov 2007) */
# define _STLSOFT_VER_1_9_10    0x01090aff  /*!< Version 1.9.10 (10th Dec 2007) */
# define _STLSOFT_VER_1_9_11    0x01090bff  /*!< Version 1.9.11 (17th Dec 2007) */
# define _STLSOFT_VER_1_9_12    0x01090cff  /*!< Version 1.9.12 (19th Dec 2007) */
# define _STLSOFT_VER_1_9_13    0x01090dff  /*!< Version 1.9.13 (20th Dec 2007) */
# define _STLSOFT_VER_1_9_14    0x01090eff  /*!< Version 1.9.14 (23rd Dec 2007) */
# define _STLSOFT_VER_1_9_15    0x01090fff  /*!< Version 1.9.15 (24th Dec 2007) */
# define _STLSOFT_VER_1_9_16    0x010910ff  /*!< Version 1.9.16 (29th Dec 2007) */
# define _STLSOFT_VER_1_9_17    0x010911ff  /*!< Version 1.9.17 (5th Jan 2008) */
# define _STLSOFT_VER_1_9_18    0x010912ff  /*!< Version 1.9.18 (27th Jan 2008) */
# define _STLSOFT_VER_1_9_19    0x010913ff  /*!< Version 1.9.19 (2nd Feb 2008) */
# define _STLSOFT_VER_1_9_20    0x010914ff  /*!< Version 1.9.20 (3rd Feb 2008) */
# define _STLSOFT_VER_1_9_21    0x010915ff  /*!< Version 1.9.21 (5th Feb 2008) */
# define _STLSOFT_VER_1_9_22    0x010916ff  /*!< Version 1.9.22 (8th Feb 2008) */
# define _STLSOFT_VER_1_9_23    0x010917ff  /*!< Version 1.9.23 (21st Feb 2008) */
# define _STLSOFT_VER_1_9_24    0x010918ff  /*!< Version 1.9.24 (8th Mar 2008) */
# define _STLSOFT_VER_1_9_25    0x010919ff  /*!< Version 1.9.25 (9th Mar 2008) */
# define _STLSOFT_VER_1_9_26    0x01091aff  /*!< Version 1.9.26 (15th Mar 2008) */
# define _STLSOFT_VER_1_9_27    0x01091bff  /*!< Version 1.9.27 (17th Mar 2008) */
# define _STLSOFT_VER_1_9_28    0x01091cff  /*!< Version 1.9.28 (24th Mar 2008) */
# define _STLSOFT_VER_1_9_29    0x01091dff  /*!< Version 1.9.29 (3rd Apr 2008) */
# define _STLSOFT_VER_1_9_30    0x01091eff  /*!< Version 1.9.30 (3rd Apr 2008) */
# define _STLSOFT_VER_1_9_31    0x01091fff  /*!< Version 1.9.31 (13th Apr 2008) */
# define _STLSOFT_VER_1_9_32    0x010920ff  /*!< Version 1.9.32 (22nd Apr 2008) */
# define _STLSOFT_VER_1_9_33    0x010921ff  /*!< Version 1.9.33 (24th Apr 2008) */
# define _STLSOFT_VER_1_9_34    0x010922ff  /*!< Version 1.9.34 (25th Apr 2008) */
# define _STLSOFT_VER_1_9_35    0x010923ff  /*!< Version 1.9.35 (29th Apr 2008) */
# define _STLSOFT_VER_1_9_36    0x010924ff  /*!< Version 1.9.36 (30th Apr 2008) */
# define _STLSOFT_VER_1_9_37    0x010925ff  /*!< Version 1.9.37 (3rd May 2008) */
# define _STLSOFT_VER_1_9_38    0x010926ff  /*!< Version 1.9.38 (4th May 2008) */
# define _STLSOFT_VER_1_9_39    0x010927ff  /*!< Version 1.9.39 (10th May 2008) */
# define _STLSOFT_VER_1_9_40    0x010928ff  /*!< Version 1.9.40 (13th May 2008) */
# define _STLSOFT_VER_1_9_41    0x010929ff  /*!< Version 1.9.41 (14th May 2008) */
# define _STLSOFT_VER_1_9_42    0x01092aff  /*!< Version 1.9.42 (17th May 2008) */
# define _STLSOFT_VER_1_9_43    0x01092bff  /*!< Version 1.9.43 (31st May 2008) */
# define _STLSOFT_VER_1_9_44    0x01092cff  /*!< Version 1.9.44 (1st Jun 2008) */
# define _STLSOFT_VER_1_9_45    0x01092dff  /*!< Version 1.9.45 (8th Jun 2008) */
# define _STLSOFT_VER_1_9_46    0x01092eff  /*!< Version 1.9.46 (11th Aug 2008) */
# define _STLSOFT_VER_1_9_48    0x010930ff  /*!< Version 1.9.48 (23rd Aug 2008) */
# define _STLSOFT_VER_1_9_49    0x010931ff  /*!< Version 1.9.49 (2nd Sep 2008) */
# define _STLSOFT_VER_1_9_50    0x010932ff  /*!< Version 1.9.50 (3rd Sep 2008) */
# define _STLSOFT_VER_1_9_51    0x010933ff  /*!< Version 1.9.51 (6th Sep 2008) */
# define _STLSOFT_VER_1_9_52    0x010934ff  /*!< Version 1.9.52 (11th Sep 2008) */
# define _STLSOFT_VER_1_9_53    0x010935ff  /*!< Version 1.9.53 (15th Sep 2008) */
# define _STLSOFT_VER_1_9_54    0x010936ff  /*!< Version 1.9.54 (15th Sep 2008) */
# define _STLSOFT_VER_1_9_55    0x010937ff  /*!< Version 1.9.55 (23rd Sep 2008) */
# define _STLSOFT_VER_1_9_56    0x010938ff  /*!< Version 1.9.56 (1st Oct 2008) */
# define _STLSOFT_VER_1_9_57    0x010939ff  /*!< Version 1.9.57 (10th Oct 2008) */
# define _STLSOFT_VER_1_9_58    0x01093aff  /*!< Version 1.9.58 (15th Oct 2008) */
# define _STLSOFT_VER_1_9_59    0x01093bff  /*!< Version 1.9.59 (24th Oct 2008) */
# define _STLSOFT_VER_1_9_60    0x01093cff  /*!< Version 1.9.60 (27th Oct 2008) */
# define _STLSOFT_VER_1_9_62    0x01093eff  /*!< Version 1.9.62 (1st Dec 2008) */
# define _STLSOFT_VER_1_9_63    0x01093fff  /*!< Version 1.9.63 (9th Dec 2008) */
# define _STLSOFT_VER_1_9_64    0x010940ff  /*!< Version 1.9.64 (28th Dec 2008) */
# define _STLSOFT_VER_1_9_65    0x010941ff  /*!< Version 1.9.65 (1st Jan 2009) */
# define _STLSOFT_VER_1_9_66    0x010942ff  /*!< Version 1.9.66 (4th Jan 2009) */
# define _STLSOFT_VER_1_9_67    0x010943ff  /*!< Version 1.9.67 (17th Jan 2009) */
# define _STLSOFT_VER_1_9_68    0x010944ff  /*!< Version 1.9.68 (18th Jan 2009) */
# define _STLSOFT_VER_1_9_69    0x010945ff  /*!< Version 1.9.69 (23rd Jan 2009) */
# define _STLSOFT_VER_1_9_70    0x010946ff  /*!< Version 1.9.70 (24th Jan 2009) */
# define _STLSOFT_VER_1_9_71    0x010947ff  /*!< Version 1.9.71 (28th Jan 2009) */
# define _STLSOFT_VER_1_9_72    0x010948ff  /*!< Version 1.9.72 (1st Feb 2009) */
# define _STLSOFT_VER_1_9_73    0x010949ff  /*!< Version 1.9.73 (5th Feb 2009) */
# define _STLSOFT_VER_1_9_74    0x01094aff  /*!< Version 1.9.74 (6th Feb 2009) */
# define _STLSOFT_VER_1_9_75    0x01094bff  /*!< Version 1.9.75 (24th Feb 2009) */
# define _STLSOFT_VER_1_9_76    0x01094cff  /*!< Version 1.9.76 (6th Mar 2009) */
# define _STLSOFT_VER_1_9_77    0x01094dff  /*!< Version 1.9.77 (9th Mar 2009) */
# define _STLSOFT_VER_1_9_78    0x01094eff  /*!< Version 1.9.78 (27th Apr 2009) */
# define _STLSOFT_VER_1_9_79    0x01094fff  /*!< Version 1.9.79 (2nd May 2009) */
# define _STLSOFT_VER_1_9_80    0x010950ff  /*!< Version 1.9.80 (5th May 2009) */
# define _STLSOFT_VER_1_9_81    0x010951ff  /*!< Version 1.9.81 (??? May 2009) */
# define _STLSOFT_VER_1_9_82    0x010952ff  /*!< Version 1.9.82 (14th May 2009) */
# define _STLSOFT_VER_1_9_83    0x010953ff  /*!< Version 1.9.83 (19th May 2009) */
# define _STLSOFT_VER_1_9_84    0x010954ff  /*!< Version 1.9.84 (22nd May 2009) */
# define _STLSOFT_VER_1_9_85    0x010955ff  /*!< Version 1.9.85 (16th June 2009) */
# define _STLSOFT_VER_1_9_86    0x010956ff  /*!< Version 1.9.86 (21st July 2009) */
# define _STLSOFT_VER_1_9_87    0x010957ff  /*!< Version 1.9.87 (10th August 2009) */
# define _STLSOFT_VER_1_9_88    0x010958ff  /*!< Version 1.9.88 (12th January 2010) */
# define _STLSOFT_VER_1_9_89    0x010959ff  /*!< Version 1.9.89 (19th January 2010) */
# define _STLSOFT_VER_1_9_90    0x01095aff  /*!< Version 1.9.90 (21st January 2010) */
# define _STLSOFT_VER_1_9_91    0x01095bff  /*!< Version 1.9.91 (30th January 2010) */
# define _STLSOFT_VER_1_9_92    0x01095cff  /*!< Version 1.9.92 (11th February 2010) */
# define _STLSOFT_VER_1_9_93    0x01095dff  /*!< Version 1.9.93 (14th February 2010) */
# define _STLSOFT_VER_1_9_95    0x01095fff  /*!< Version 1.9.95 (7th March 2010) */
# define _STLSOFT_VER_1_9_96    0x010960ff  /*!< Version 1.9.96 (10th March 2010) */
# define _STLSOFT_VER_1_9_97    0x010961ff  /*!< Version 1.9.97 (3rd April 2010) */
# define _STLSOFT_VER_1_9_98    0x010962ff  /*!< Version 1.9.98 (7th June 2010) */
# define _STLSOFT_VER_1_9_99    0x010963ff  /*!< Version 1.9.99 (21st June 2010) */
# define _STLSOFT_VER_1_9_100   0x010964ff  /*!< Version 1.9.100 (29th July 2010) */
# define _STLSOFT_VER_1_9_101   0x010965ff  /*!< Version 1.9.101 (30th September 2010) */
# define _STLSOFT_VER_1_9_102   0x010966ff  /*!< Version 1.9.102 (8th November 2010) */
# define _STLSOFT_VER_1_9_103   0x010967ff  /*!< Version 1.9.103 (20th November 2010) */
# define _STLSOFT_VER_1_9_104   0x010968ff  /*!< Version 1.9.104 (22nd November 2010) */
# define _STLSOFT_VER_1_9_105   0x010969ff  /*!< Version 1.9.105 (22nd December 2010) */
# define _STLSOFT_VER_1_9_106   0x01096aff  /*!< Version 1.9.106 (28th December 2010) */
# define _STLSOFT_VER_1_9_107   0x01096bff  /*!< Version 1.9.107 (30th January 2011) */
# define _STLSOFT_VER_1_9_108   0x01096cff  /*!< Version 1.9.108 (31st January 2011) */
# define _STLSOFT_VER_1_9_109   0x01096dff  /*!< Version 1.9.109 (17th February 2011) */
# define _STLSOFT_VER_1_9_110   0x01096eff  /*!< Version 1.9.110 (25th November 2011) */
# define _STLSOFT_VER_1_9_111   0x01096fff  /*!< Version 1.9.111 (30th November 2011) */
# define _STLSOFT_VER_1_9_112   0x010970ff  /*!< Version 1.9.112 (7th February 2012) */
# define _STLSOFT_VER_1_9_113   0x010971ff  /*!< Version 1.9.113 (4th June 2012) */
# define _STLSOFT_VER_1_9_114   0x010972ff  /*!< Version 1.9.114 (4th June 2012) */
# define _STLSOFT_VER_1_9_115   0x010973ff  /*!< Version 1.9.115 (30th July 2012) */
# define _STLSOFT_VER_1_9_116   0x010974ff  /*!< Version 1.9.116 (18th August 2012) */
# define _STLSOFT_VER_1_9_117   0x010975ff  /*!< Version 1.9.117 (16th February 2013) */
# define _STLSOFT_VER_1_9_118   0x010976ff  /*!< Version 1.9.118 (31st May 2014) */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

#define _STLSOFT_VER_MAJOR      1
#define _STLSOFT_VER_MINOR      9
#define _STLSOFT_VER_REVISION   118
#define _STLSOFT_VER            _STLSOFT_VER_1_9_118

/* /////////////////////////////////////
 * Underlying version detection
 */

/* defines STLSOFT_HEAD_VER, which specifies the current version of the 
 * main (HEAD) library. Will never be greater than STLSOFT_LEAD_VER.
 */
#include <stlsoft/internal/head_version.h>

/* defines STLSOFT_LEAD_VER, which specifies the current version of any
 * alpha (LEAD) library. Will never be less than STLSOFT_HEAD_VER.
 */
#include <stlsoft/internal/lead_version.h>

/* /////////////////////////////////////////////////////////////////////////
 * Basic macros
 */

/* Compilation messages
 *
 * To see certain informational messages during compilation define the
 * preprocessor symbol STLSOFT_COMPILE_VERBOSE
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# define STLSOFT_STRINGIZE_a_(x)            #x
# define STLSOFT_STRINGIZE_a(x)             STLSOFT_STRINGIZE_a_(x)

# if defined(__BORLANDC__) || \
     defined(__SUNPRO_C)
#  define STLSOFT_STRINGIZE_w_(x)           L"" ## STLSOFT_STRINGIZE_a(x)
# else /* ? compiler */
#  define STLSOFT_STRINGIZE_w_(x)           L ## #x
# endif /* compiler */
# define STLSOFT_STRINGIZE_w(x)             STLSOFT_STRINGIZE_w_(x)

# define STLSOFT_STRINGIZE(x)               STLSOFT_STRINGIZE_a(x)

/* Simple macro indirection */
# define STLSOFT_MACRO_INDIRECT(x)          x

/* Token pasting */
# define STLSOFT_PP_PASTE_2_TOKENS(t1, t2)        t1 ## t2
# define STLSOFT_PP_PASTE_3_TOKENS(t1, t2, t3)    t1 ## t2 ## t3

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Sanity checks - 1
 */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler compatibility
 *
 * Currently the only compilers supported by the STLSoft libraries are
 *
 * Borland C++ 5.5, 5.51, 5.6 & 5.6.4
 * Comeau 4.3.0.1 & 4.3.3
 * Digital Mars C/C++ 8.26 and above
 * GCC 2.95, 2.96, 3.2, 3.3, 3.4 & 4.0
 * Intel C/C++ 6.0, 7.0, 7.1, 8.0, 9.0
 * Metrowerks 2.4 & 3.0 (CodeWarrior 7.0 & 8.0)
 * SunPro 5.9
 * Visual C++ 4.2, 5.0, 6.0, 7.0 (.NET), 7.1 (.NET 2003), 8.0
 * Watcom C/C++ 11.0, Open Watcom 1.0, 1.1, 1.2, 1.3, 1.4, 1.5
 */

#ifdef STLSOFT_COMPILER_IS_UNKNOWN
# undef STLSOFT_COMPILER_IS_UNKNOWN
#endif /* STLSOFT_COMPILER_IS_UNKNOWN */

#ifdef STLSOFT_COMPILER_IS_BORLAND
# undef STLSOFT_COMPILER_IS_BORLAND
#endif /* STLSOFT_COMPILER_IS_BORLAND */

#ifdef STLSOFT_COMPILER_IS_COMO
# undef STLSOFT_COMPILER_IS_COMO
#endif /* STLSOFT_COMPILER_IS_COMO */

#ifdef STLSOFT_COMPILER_IS_DMC
# undef STLSOFT_COMPILER_IS_DMC
#endif /* STLSOFT_COMPILER_IS_DMC */

#ifdef STLSOFT_COMPILER_IS_GCC
# undef STLSOFT_COMPILER_IS_GCC
#endif /* STLSOFT_COMPILER_IS_GCC */

#ifdef STLSOFT_COMPILER_IS_INTEL
# undef STLSOFT_COMPILER_IS_INTEL
#endif /* STLSOFT_COMPILER_IS_INTEL */

#ifdef STLSOFT_COMPILER_IS_MSVC
# undef STLSOFT_COMPILER_IS_MSVC
#endif /* STLSOFT_COMPILER_IS_MSVC */

#ifdef STLSOFT_COMPILER_IS_MWERKS
# undef STLSOFT_COMPILER_IS_MWERKS
#endif /* STLSOFT_COMPILER_IS_MWERKS */

#ifdef STLSOFT_COMPILER_IS_SUNPRO
# undef STLSOFT_COMPILER_IS_SUNPRO
#endif /* STLSOFT_COMPILER_IS_SUNPRO */

#ifdef STLSOFT_COMPILER_IS_VECTORC
# undef STLSOFT_COMPILER_IS_VECTORC
#endif /* STLSOFT_COMPILER_IS_VECTORC */

#ifdef STLSOFT_COMPILER_IS_WATCOM
# undef STLSOFT_COMPILER_IS_WATCOM
#endif /* STLSOFT_COMPILER_IS_WATCOM */

/* Strict compability fix
 */

#if defined(_STLSOFT_STRICT) && \
    !defined(STLSOFT_STRICT)
# define STLSOFT_STRICT
#endif /* _STLSOFT_STRICT && !STLSOFT_STRICT */

/* First we do a check to see whether other compilers are providing
 * compatibility with Visual C++, and handle that.
 */

#ifdef _MSC_VER
# if defined(__BORLANDC__) ||      /* Borland */ \
     defined(__COMO__) ||          /* Comeau */ \
     defined(__DMC__) ||           /* Digital Mars */ \
     defined(__GNUC__) ||          /* GNU */ \
     defined(__INTEL_COMPILER) ||  /* Intel */ \
     defined(__MWERKS__) ||        /* Metrowerks */ \
     defined(__SUNPRO_C) ||        /* Sun Pro C */ \
     defined(__SUNPRO_CC) ||       /* Sun Pro C++ */ \
     defined(__VECTORC) ||         /* VectorC */ \
     defined(__WATCOMC__)          /* Watcom */
  /* Handle Microsoft Visual C++ support. */
#  if defined(_STLSOFT_NO_MSC_VER_SUPPORT) || \
     (   defined(STLSOFT_STRICT) && \
         !defined(_STLSOFT_MSC_VER_SUPPORT))
#   undef _MSC_VER
#  endif /* _STLSOFT_NO_MSC_VER_SUPPORT || (STLSOFT_STRICT && _STLSOFT_MSC_VER_SUPPORT) */
# endif /* compiler */
#endif /* _MSC_VER */

#if defined(_STLSOFT_FORCE_CUSTOM_COMPILER)
# define STLSOFT_COMPILER_LABEL_STRING          "Custom (forced) compiler"
# define STLSOFT_COMPILER_VERSION_STRING        "Custom (forced) compiler"
# define STLSOFT_COMPILER_IS_CUSTOM
# ifndef __STLSOFT_CF_CUSTOM_COMPILER_INCLUDE_NAME
#  error When using the custom compiler option you must define the symbol __STLSOFT_CF_CUSTOM_COMPILER_INCLUDE_NAME, e.g. #define __STLSOFT_CF_CUSTOM_COMPILER_INCLUDE_NAME <stlsoft/internal/cccap/my_compiler.h>
# endif /* !__STLSOFT_CF_CUSTOM_COMPILER_INCLUDE_NAME */

#elif defined(__COMO__) /* Do Comeau next, so that no Comeau back-end server compilers are preferentially discriminated */
 /* ******************************* Comeau ****************************** */
# define STLSOFT_COMPILER_IS_COMO
# define STLSOFT_COMPILER_LABEL_STRING          "Comeau C++"
# if __COMO_VERSION__ < 4300
#  error Only versions 4.3.0.1 and later of Comeau C++ compiler is supported by the STLSoft libraries
# elif (__COMO_VERSION__ == 4300)
#  define STLSOFT_COMPILER_VERSION_STRING       "Comeau C++ 4.3.0.1"
# elif (__COMO_VERSION__ == 4303)
#  define STLSOFT_COMPILER_VERSION_STRING       "Comeau C++ 4.3.3"
# else /* ? __COMO_VERSION__ */
#  define STLSOFT_COMPILER_VERSION_STRING       "Unknown version of Comeau C++"
# endif /* __COMO_VERSION__ */

#elif defined(__BORLANDC__)
 /* ******************************* Borland ***************************** */
# define STLSOFT_COMPILER_IS_BORLAND
# define STLSOFT_COMPILER_LABEL_STRING          "Borland C/C++"
# if 0 /* (__BORLANDC__ == 0x0460) */
#  define STLSOFT_COMPILER_VERSION_STRING       "Borland C++ 4.52"
# elif 0 /* (__BORLANDC__ == 0x0550) */
#  define STLSOFT_COMPILER_VERSION_STRING       "Borland C++ 5.5"
# elif (__BORLANDC__ == 0x0551)
#  define STLSOFT_COMPILER_VERSION_STRING       "Borland C++ 5.51"
# elif (__BORLANDC__ == 0x0560)
#  define STLSOFT_COMPILER_VERSION_STRING       "Borland C++ 5.6"
# elif (__BORLANDC__ == 0x0564)
#  define STLSOFT_COMPILER_VERSION_STRING       "Borland C++ 5.6.4 (C++ BuilderX)"
# elif (__BORLANDC__ == 0x0582)
#  define STLSOFT_COMPILER_VERSION_STRING       "Borland C++ 5.82 (Turbo C++)"
# elif (0x0590 == (__BORLANDC__ & 0xfff0))
#  define STLSOFT_COMPILER_VERSION_STRING       "Borland C++ 5.9.x"
# elif (0x0610 == (__BORLANDC__ & 0xfff0))
#  define STLSOFT_COMPILER_VERSION_STRING       "Borland C++ 6.1.x (C++ Builder)"
# else /* ? __BORLANDC__ */
#  error Currently only versions 5.51, 5.6, 5.6.4, 5.8.2, 5.9.x, and 6.1.x of the Borland C++ compiler are supported by the STLSoft libraries
# endif /* __BORLANDC__ */

#elif defined(__DMC__)
 /* **************************** Digital Mars *************************** */
# define STLSOFT_COMPILER_IS_DMC
# define STLSOFT_COMPILER_LABEL_STRING          "Digital Mars C/C++"
# if (__DMC__ < 0x0826)
#  error Only versions 8.26 and later of the Digital Mars C/C++ compilers are supported by the STLSoft libraries
# else /* ? __DMC__ */
#  if __DMC__ >= 0x0832
#   define STLSOFT_COMPILER_VERSION_STRING      __DMC_VERSION_STRING__
#  elif (__DMC__ == 0x0826)
#   define STLSOFT_COMPILER_VERSION_STRING      "Digital Mars C/C++ 8.26"
#  elif (__DMC__ == 0x0827)
#   define STLSOFT_COMPILER_VERSION_STRING      "Digital Mars C/C++ 8.27"
#  elif (__DMC__ == 0x0828)
#   define STLSOFT_COMPILER_VERSION_STRING      "Digital Mars C/C++ 8.28"
#  elif (__DMC__ == 0x0829)
#   define STLSOFT_COMPILER_VERSION_STRING      "Digital Mars C/C++ 8.29"
#  elif (__DMC__ == 0x0830)
#   define STLSOFT_COMPILER_VERSION_STRING      "Digital Mars C/C++ 8.30"
#  elif (__DMC__ == 0x0831)
#   define STLSOFT_COMPILER_VERSION_STRING      "Digital Mars C/C++ 8.31"
#  endif /* __DMC__ */
# endif /* version */

#elif defined(__GNUC__)
 /* ******************************** GCC ******************************** */
# define STLSOFT_COMPILER_IS_GCC
# define STLSOFT_COMPILER_LABEL_STRING          "GNU C/C++"
# if __GNUC__ != 2 && \
     __GNUC__ != 3 && \
     __GNUC__ != 4
#  error GNU C/C++ compilers whose major version is not 2, 3 or 4 are not currently supported by the STLSoft libraries
# elif __GNUC__ == 2
#  if __GNUC_MINOR__ < 95
#   error Currently only version 2.95 and above of the GNU C/C++ compiler is supported by the STLSoft libraries
#  elif __GNUC_MINOR__ == 95
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ 2.95"
#  elif __GNUC_MINOR__ == 96
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ 2.96"
#  else /* ? __GNUC_MINOR__ */
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ >2.96 - you should be aware that this version may not be supported correctly"
#  endif /* __GNUC_MINOR__  */
# elif __GNUC__ == 3
#  if __GNUC_MINOR__ == 2
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ 3.2"
#  elif __GNUC_MINOR__ == 3
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ 3.3"
#  elif __GNUC_MINOR__ == 4
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ 3.4"
#  else /* ? __GNUC_MINOR__ */
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ >3.4 - you should be aware that this version may not be supported correctly"
#  endif /* __GNUC_MINOR__  */
# elif __GNUC__ == 0
#  if __GNUC_MINOR__ == 0
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ 4.0"
#  else /* ? __GNUC_MINOR__ */
#   define STLSOFT_COMPILER_VERSION_STRING      "GNU C/C++ >4.0 - you should be aware that this version may not be supported correctly"
#  endif /* __GNUC__  */
# endif /* __GNUC_MINOR__ */

#elif defined(__INTEL_COMPILER)
 /* ******************************* Intel ******************************* */
# define STLSOFT_COMPILER_IS_INTEL
# define STLSOFT_COMPILER_LABEL_STRING          "Intel C/C++"
# if (__INTEL_COMPILER == 600)
#  define STLSOFT_COMPILER_VERSION_STRING       "Intel C/C++ 6.0"
# elif (__INTEL_COMPILER == 700)
#  define STLSOFT_COMPILER_VERSION_STRING       "Intel C/C++ 7.0"
# elif (__INTEL_COMPILER == 800)
#  define STLSOFT_COMPILER_VERSION_STRING       "Intel C/C++ 8.0"
# elif (__INTEL_COMPILER == 900)
#  define STLSOFT_COMPILER_VERSION_STRING       "Intel C/C++ 9.0"
# elif (__INTEL_COMPILER == 1000)
#  define STLSOFT_COMPILER_VERSION_STRING       "Intel C/C++ 10.0"
# elif (__INTEL_COMPILER == 1100)
#  define STLSOFT_COMPILER_VERSION_STRING       "Intel C/C++ 11.0"
# elif (__INTEL_COMPILER >= 1200) && (__INTEL_COMPILER < 1300)
#  define STLSOFT_COMPILER_VERSION_STRING       "Intel C/C++ 12.x"
# else /* ? __INTEL_COMPILER */
#  error Only Intel C++ Compiler versions 6.0, 7.0(/7.1), 8.0, 9.0, 10.0, 11.0 and 12.x currently supported by the STLSoft libraries
# endif /* __INTEL_COMPILER */

#elif defined(__MWERKS__)
 /* ***************************** Metrowerks **************************** */
# define STLSOFT_COMPILER_IS_MWERKS
# define STLSOFT_COMPILER_LABEL_STRING          "Metrowerks CodeWarrior C/C++"
# if ((__MWERKS__ & 0xFF00) == 0x2400)
#  define STLSOFT_COMPILER_VERSION_STRING       "Metrowerks CodeWarrior C/C++ 2.4"
# elif ((__MWERKS__ & 0xFF00) == 0x3000)
#  define STLSOFT_COMPILER_VERSION_STRING       "Metrowerks CodeWarrior C/C++ 3.0"
# elif ((__MWERKS__ & 0xFF00) == 0x3200)
#  define STLSOFT_COMPILER_VERSION_STRING       "Metrowerks CodeWarrior C/C++ 3.2"
# else /* ? __MWERKS__ */
#  error Only Metrowerks C++ Compiler 2.4 (CodeWarrior 7), 3.0 (CodeWarrior 8) and 3.2 (CodeWarrior 9) currently supported by the STLSoft libraries
# endif /* __MWERKS__ */

#elif defined(__SUNPRO_C) || \
      defined(__SUNPRO_CC)
 /* ******************************* Sun Pro ***************************** */
 /* Sanity check on language/compiler */
# ifdef __cplusplus
#  ifdef __SUNPRO_C
#   error __SUNPRO_C should not be defined by the Sun C compiler in C++ compilation
#  endif /* __SUNPRO_C */
#  ifndef __SUNPRO_CC
#   error __SUNPRO_CC should be defined by the Sun C++ compiler in C++ compilation
#  endif /* !__SUNPRO_CC */
# else /* ? __cplusplus */
#  ifndef __SUNPRO_C
#   error __SUNPRO_C should be defined by the Sun C compiler in C compilation
#  endif /* !__SUNPRO_C */
#  ifdef __SUNPRO_CC
#   error __SUNPRO_CC should not be defined by the Sun C++ compiler in C compilation
#  endif /* __SUNPRO_CC */
# endif /* __cplusplus */

# define STLSOFT_COMPILER_IS_SUNPRO
# ifdef __cplusplus
#  define STLSOFT_COMPILER_LABEL_STRING         "Sun Pro C++"
#  if (0x0590 == (__SUNPRO_CC & 0xFFF0))
#   define STLSOFT_COMPILER_VERSION_STRING      "Sun Pro C++ v5.9"
#  else /* ? __SUNPRO_CC */
#   error Currently only version v5.9 of the Sun Pro C++ compiler is supported by the STLSoft libraries
#  endif /* __SUNPRO_CC */
# else /* ? __cplusplus */
#  define STLSOFT_COMPILER_LABEL_STRING         "Sun Pro C"
#  if (0x0590 == (__SUNPRO_C & 0xFFF0))
#   define STLSOFT_COMPILER_VERSION_STRING      "Sun Pro C v5.9"
#  else /* ? __SUNPRO_CC */
#   error Currently only version v5.9 of the Sun Pro C compiler is supported by the STLSoft libraries
#  endif /* __SUNPRO_CC */
# endif /* __cplusplus */

#elif defined(__VECTORC)
 /* ************************** CodePlay VectorC ************************* */
# define STLSOFT_COMPILER_IS_VECTORC
# define STLSOFT_COMPILER_LABEL_STRING          "CodePlay VectorC C/C++"
# if (__VECTORC == 1)
#  define STLSOFT_COMPILER_VERSION_STRING       "CodePlay VectorC C/C++"
# else /* ? __VECTORC */
#  error Currently only versions of the CodePlay Vector C/C++ compiler defining __VECTORC == 1 are supported by the STLSoft libraries
# endif /* __VECTORC */

#elif defined(__WATCOMC__)
 /* ******************************* Watcom ****************************** */
# define STLSOFT_COMPILER_IS_WATCOM
# define STLSOFT_COMPILER_LABEL_STRING          "Watcom C/C++"

# if (__WATCOMC__ == 1200)
#  define STLSOFT_COMPILER_VERSION_STRING       "Open Watcom C/C++ 1.0"
# elif (__WATCOMC__ == 1210)
#  define STLSOFT_COMPILER_VERSION_STRING       "Open Watcom C/C++ 1.1"
# elif (__WATCOMC__ == 1220)
#  define STLSOFT_COMPILER_VERSION_STRING       "Open Watcom C/C++ 1.2"
# elif (__WATCOMC__ == 1230)
#  define STLSOFT_COMPILER_VERSION_STRING       "Open Watcom C/C++ 1.3"
# elif (__WATCOMC__ == 1240)
#  define STLSOFT_COMPILER_VERSION_STRING       "Open Watcom C/C++ 1.4"
# elif (__WATCOMC__ == 1250)
#  define STLSOFT_COMPILER_VERSION_STRING       "Open Watcom C/C++ 1.5"
# elif (__WATCOMC__ == 1260)
#  define STLSOFT_COMPILER_VERSION_STRING       "Open Watcom C/C++ 1.6"
# elif (__WATCOMC__ == 1270)
#  define STLSOFT_COMPILER_VERSION_STRING       "Open Watcom C/C++ 1.7"
# else /* ? __WATCOMC__ */
#  error Currently only versions 1.0-1.7 of Open Watcom C/C++ are recognised by the STLSoft libraries
# endif /* __WATCOMC__ */

#elif defined(_MSC_VER)
 /* ***************************** Visual C++ **************************** */
# define STLSOFT_COMPILER_IS_MSVC
# define STLSOFT_COMPILER_LABEL_STRING          "Visual C++"
# if defined(STLSOFT_FORCE_MSVC_4_2) && (_MSC_VER == 1020)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 4.2"
# elif (_MSC_VER == 1100)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 5.0"
# elif (_MSC_VER == 1200)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 6.0"
# elif (_MSC_VER == 1300)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 7.0"
# elif (_MSC_VER == 1310)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 7.1"
# elif (_MSC_VER == 1400)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 8.0"
# elif (_MSC_VER == 1500)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 9.0"
# elif (_MSC_VER == 1600)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 10.0"
# elif (_MSC_VER == 1700)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 11.0"
# elif (_MSC_VER == 1800)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 12.0"
# elif (_MSC_VER == 1900)
#  define STLSOFT_COMPILER_VERSION_STRING       "Visual C++ 14.0"
# else /* ? _MSC_VER */
#  error Currently only versions 5.0, 6.0, 7.0, 7.1, 8.0, 9.0, 10.0, 11.0, and 12.0 of the Visual C++ compiler are supported by the STLSoft libraries
# endif /* _MSC_VER */

#else /* ? compiler */
 /* ********************** No recognised compiler *********************** */
# if defined(_STLSOFT_FORCE_UNKNOWN_COMPILER) || \
     defined(_STLSOFT_FORCE_ANY_COMPILER)
#  define STLSOFT_COMPILER_LABEL_STRING         "Unknown (forced) compiler"
#  define STLSOFT_COMPILER_VERSION_STRING       "Unknown (forced) compiler"
#  define STLSOFT_COMPILER_IS_UNKNOWN
# else /* ? _STLSOFT_FORCE_UNKNOWN_COMPILER || _STLSOFT_FORCE_ANY_COMPILER */
#  error Compiler is not recognised.
#  error Currently only Borland C++, Comeau C++, Digital Mars C/C++, GNU C/C++,
#  error  Intel C/C++, Metrowerks CodeWarrior, Visual C++ and Watcom C/C++
#  error  compilers are supported by the STLSoft libraries
#  error If you want to use the libraries with your compiler, you may specify the
#  error  _STLSOFT_FORCE_CUSTOM_COMPILER or _STLSOFT_FORCE_ANY_COMPILER preprocessor
#  error  symbols.
#  error _STLSOFT_FORCE_ANY_COMPILER assumes that your compiler can support all
#  error  modern C++ compiler features, and causes the inclusion of the compiler
#  error  features file stlsoft/internal/cccap/unknown.h, which is provided by STLSoft.
#  error _STLSOFT_FORCE_CUSTOM_COMPILER requires that you specify the name of the
#  error  compiler features file in __STLSOFT_CF_CUSTOM_COMPILER_INCLUDE_NAME.
#  error The idea is to use _STLSOFT_FORCE_ANY_COMPILER, to determine what language
#  error  features your compiler can support, and then copy, edit and use that file
#  error  via _STLSOFT_FORCE_CUSTOM_COMPILER and __STLSOFT_CF_CUSTOM_COMPILER_INCLUDE_NAME.
# endif /* _STLSOFT_FORCE_ANY_COMPILER */

#endif /* compiler tag */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
/** \def STLSOFT_COMPILER_LABEL_STRING
 * \brief A nul-terminated C-style string denoting the name of the compiler.
 */
# define STLSOFT_COMPILER_LABEL_STRING          "Acme Compiler"
/** \def STLSOFT_COMPILER_VERSION_STRING
 * \brief A nul-terminated C-style string denoting the name and version of the compiler.
 */
# define STLSOFT_COMPILER_VERSION_STRING        "Acme Compiler v1.0"
#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */
# define __STLSOFT_COMPILER_LABEL_STRING        STLSOFT_COMPILER_LABEL_STRING
# define __STLSOFT_COMPILER_VERSION_STRING      STLSOFT_COMPILER_VERSION_STRING
#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Compiler language feature support
 *
 * Various compilers support the language differently (or not at all), so these
 * features are discriminated here and utilised by various means within the code
 * in order to minimise the use of the preprocessor conditionals in the other
 * libraries' source code.
 */

#if defined(_STLSOFT_INCLUDE_UNDEFS) || \
    (  defined(STLSOFT_STRICT) && \
       !defined(STLSOFT_NO_UNDEFS))
# include <stlsoft/internal/_undefs.h>
#endif /* _STLSOFT_INCLUDE_UNDEFS) || (STLSOFT_STRICT) && !STLSOFT_NO_UNDEFS) */

/* Now we include the appropriate compiler-specific header */

#if defined(STLSOFT_COMPILER_IS_CUSTOM)
# include __STLSOFT_CF_CUSTOM_COMPILER_INCLUDE_NAME
#elif defined(STLSOFT_COMPILER_IS_UNKNOWN)
# include <stlsoft/internal/cccap/unknown.h>
#elif defined(STLSOFT_COMPILER_IS_BORLAND)
# include <stlsoft/internal/cccap/borland.h>
#elif defined(STLSOFT_COMPILER_IS_COMO)
# include <stlsoft/internal/cccap/como.h>
#elif defined(STLSOFT_COMPILER_IS_DMC)
# include <stlsoft/internal/cccap/dmc.h>
#elif defined(STLSOFT_COMPILER_IS_GCC)
# include <stlsoft/internal/cccap/gcc.h>
#elif defined(STLSOFT_COMPILER_IS_INTEL)
# include <stlsoft/internal/cccap/intel.h>
#elif defined(STLSOFT_COMPILER_IS_MSVC)
# include <stlsoft/internal/cccap/msvc.h>
#elif defined(STLSOFT_COMPILER_IS_MWERKS)
# include <stlsoft/internal/cccap/mwerks.h>
#elif defined(STLSOFT_COMPILER_IS_SUNPRO)
# include <stlsoft/internal/cccap/sunpro.h>
#elif defined(STLSOFT_COMPILER_IS_VECTORC)
# include <stlsoft/internal/cccap/vectorc.h>
#elif defined(STLSOFT_COMPILER_IS_WATCOM)
# include <stlsoft/internal/cccap/watcom.h>
#else /* ? compiler */
# error Compiler not correctly discriminated
#endif /* compiler */

/* pragma message support */

/* backwards-compatibility : _STLSOFT_COMPILE_VERBOSE => STLSOFT_COMPILE_VERBOSE */

#if defined(_STLSOFT_COMPILE_VERBOSE) && \
    !defined(STLSOFT_COMPILE_VERBOSE)
# define STLSOFT_COMPILE_VERBOSE
#endif

/* backwards-compatibility : STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT => STLSOFT_PPF_pragma_message_SUPPORT */
#if defined(STLSOFT_CF_PRAGMA_MESSAGE_SUPPORT) && \
    !defined(STLSOFT_PPF_pragma_message_SUPPORT)
# define STLSOFT_PPF_pragma_message_SUPPORT
#endif

#if defined(STLSOFT_COMPILE_VERBOSE) && \
    !defined(STLSOFT_PPF_pragma_message_SUPPORT)
# undef STLSOFT_COMPILE_VERBOSE
#endif /* !STLSOFT_PPF_pragma_message_SUPPORT && STLSOFT_COMPILE_VERBOSE */

#if defined(STLSOFT_NO_COMPILE_VERBOSE) && \
    defined(STLSOFT_COMPILE_VERBOSE)
# undef STLSOFT_COMPILE_VERBOSE
#endif /* STLSOFT_NO_COMPILE_VERBOSE && STLSOFT_COMPILE_VERBOSE */

#ifdef STLSOFT_COMPILE_VERBOSE
# pragma message(STLSOFT_COMPILER_VERSION_STRING)
#endif /* STLSOFT_COMPILE_VERBOSE */


/* backwards-compatibility : STLSOFT_COMPILE_VERBOSE => _STLSOFT_COMPILE_VERBOSE */

#if defined(STLSOFT_COMPILE_VERBOSE)
# if defined(_STLSOFT_COMPILE_VERBOSE)
#  if defined(STLSOFT_PPF_pragma_message_SUPPORT)
#   pragma message("You have defined _STLSOFT_COMPILE_VERBOSE. This symbol is now deprecated, and may be ignored by a future release : instead define STLSOFT_COMPILE_VERBOSE")
#  endif
# else /* ? _STLSOFT_COMPILE_VERBOSE */
   /* Must define it here, for backwards-compatibility with client code that uses it */
#  define _STLSOFT_COMPILE_VERBOSE
# endif /* _STLSOFT_COMPILE_VERBOSE */
#endif /* STLSOFT_COMPILE_VERBOSE */





/* __FUNCTION__ support */

/** \def STLSOFT_FUNCTION_SYMBOL
 *
 * The symbol to be used where __FUNCTION__ may be used, taking into account
 * any compiler-specific alternative forms
 *
 * \note Not defined if no appropriate symbol is available
 */

#if defined(STLSOFT_CF_FUNCTION_SYMBOL_SUPPORT)
# define STLSOFT_FUNCTION_SYMBOL    __FUNCTION__
#elif defined(STLSOFT_CF_func_SYMBOL_SUPPORT)
# define STLSOFT_FUNCTION_SYMBOL    __func__
#endif /* __FUNCTION__ or __func__ */

/* /////////////////////////////////////////////////////////////////////////
 * Sanity checks - 2
 *
 * Must have the sizes of basic integral types defined
 */

#ifndef _STLSOFT_SIZEOF_CHAR
# error _STLSOFT_SIZEOF_CHAR not defined
#endif /* !_STLSOFT_SIZEOF_CHAR */

#ifndef _STLSOFT_SIZEOF_SHORT
# error _STLSOFT_SIZEOF_SHORT not defined
#endif /* !_STLSOFT_SIZEOF_SHORT */

#ifndef _STLSOFT_SIZEOF_INT
# error _STLSOFT_SIZEOF_INT not defined
#endif /* !_STLSOFT_SIZEOF_INT */

#ifndef _STLSOFT_SIZEOF_LONG
# error _STLSOFT_SIZEOF_LONG not defined
#endif /* !_STLSOFT_SIZEOF_LONG */

#ifdef STLSOFT_CF_BUILTIN_long_long_SUPPORT
# ifndef _STLSOFT_SIZEOF_LONG_LONG
#  error _STLSOFT_SIZEOF_LONG_LONG not defined
# endif /* !_STLSOFT_SIZEOF_LONG_LONG */
#endif /* STLSOFT_CF_BUILTIN_long_long_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Sanity checks - 3
 *
 */

/* Template support */
#if defined(__cplusplus) && \
    !defined(STLSOFT_CF_TEMPLATE_SUPPORT)
# error Template support not detected. STLSoft libraries are template-based and require this support.
#endif /* STLSOFT_CF_TEMPLATE_SUPPORT */


/* Native 64-bit integer support */
#if !defined(STLSOFT_CF_64BIT_INT_SUPPORT) && \
    (   defined(STLSOFT_CF_64BIT_INT_IS___int64) || \
        defined(STLSOFT_CF_64BIT_INT_IS_long_long))
# error Definition for 64-bit support is not correct. STLSOFT_CF_64BIT_INT_SUPPORT must be defined for 64-bit support
#endif /* !STLSOFT_CF_64BIT_INT_SUPPORT && (STLSOFT_CF_64BIT_INT_IS___int64 || STLSOFT_CF_64BIT_INT_IS_long_long) */

/* Out-of-memory throws bad_alloc.
 *
 * Discriminated symbol is STLSOFT_CF_NOTHROW_BAD_ALLOC
 *
 * By default, compilations with the Borland, and Watcom compilers throw
 * bad_alloc in conditions of memory exhaustion, and those with Digital Mars
 * and Microsoft do not.
 *
 * The Microsoft compilers do not throw bad_alloc for long established reasons,
 * though they can be made to do so (see Matthew Wilson, "Generating
 * Out-Of-Memory Exceptions", Windows Developer's Journal, Vol 12 Number 5, May
 * 2001). This feature may be added in a forthcoming release of the libraries.
 *
 * The Digital Mars compiler appears to ship without any header files that
 * define bad_alloc (whether in std or not), so it is therefore assumed that
 * operator new will not throw exceptions in out of memory conditions.
 *
 * Define STLSOFT_CF_THROW_BAD_ALLOC to force Digital Mars/Microsoft to do so.
 * Define STLSOFT_CF_NOTHROW_BAD_ALLOC to prevent Borland/Comeau/
 * GCC/Metrowerks/Watcom from doing so.
 */


 /* STLSOFT_CF_EXCEPTION_SUPPORT */

# if defined(__STLSOFT_CF_EXCEPTION_SUPPORT) && \
     !defined(STLSOFT_CF_EXCEPTION_SUPPORT)
#  error Configuration error: deprecated symbol __STLSOFT_CF_EXCEPTION_SUPPORT is defined when STLSOFT_CF_EXCEPTION_SUPPORT is not!
# else /* ? STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#  ifdef STLSOFT_CF_EXCEPTION_SUPPORT
#   define __STLSOFT_CF_EXCEPTION_SUPPORT
#  endif /* STLSOFT_CF_EXCEPTION_SUPPORT */
# endif /* STLSOFT_CF_EXCEPTION_SUPPORT */

 /* STLSOFT_CF_EXCEPTION_SUPPORT */

# if defined(__STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT) && \
     !defined(STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT)
#  error Configuration error: deprecated symbol __STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT is defined when STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT is not!
# else /* ? STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#  ifdef STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
#   define __STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT
#  endif /* STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT */
# endif /* STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT */

#ifndef STLSOFT_CF_EXCEPTION_SUPPORT
 /** \def STLSOFT_CF_NOTHROW_BAD_ALLOC
  * \brief Define if you've overridden <code>operator new</code> to return
  *   <code>NULL</code> on allocation failure (which is non-standard
  *   behaviour).
  */
# define STLSOFT_CF_NOTHROW_BAD_ALLOC
#endif /* !STLSOFT_CF_EXCEPTION_SUPPORT */

#ifdef STLSOFT_CF_NOTHROW_BAD_ALLOC
# ifdef STLSOFT_CF_THROW_BAD_ALLOC
#  undef STLSOFT_CF_THROW_BAD_ALLOC
# endif /* STLSOFT_CF_THROW_BAD_ALLOC */
#else /* ? STLSOFT_CF_NOTHROW_BAD_ALLOC */
 /* Leave it to whatever the compiler's capability discrimination has determined */
#endif /* STLSOFT_CF_NOTHROW_BAD_ALLOC */



/* Template specialisation syntax support
 */
/** \def STLSOFT_TEMPLATE_SPECIALISATION
 * \brief Resolves to <code>template <></code>  for compilers that support correct
 *  template specialisation syntax, and to nothing for those that do not.
 */
#ifdef STLSOFT_TEMPLATE_SPECIALISATION
# undef STLSOFT_TEMPLATE_SPECIALISATION
#endif /* STLSOFT_TEMPLATE_SPECIALISATION */

#ifdef STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX
# define STLSOFT_TEMPLATE_SPECIALISATION                template <>
#else /* ? STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX */
# define STLSOFT_TEMPLATE_SPECIALISATION
#endif /* STLSOFT_CF_TEMPLATE_SPECIALISATION_SYNTAX */


/* Keyword support.
 *
 * Define _STLSOFT_FORCE_ALL_KEYWORDS to force the assumption of compiler
 * support for all keywords.
 *
 * Define _STLSOFT_FORCE_KEYWORD_EXPLICIT to force the assumption of compiler
 * support for the explicit keyword
 *
 * Define _STLSOFT_FORCE_KEYWORD_MUTABLE to force the assumption of compiler
 * support for the mutable keyword
 *
 * Define _STLSOFT_FORCE_KEYWORD_TYPENAME to force the assumption of compiler
 * support for the typename keyword
 */

#ifdef _STLSOFT_FORCE_ALL_KEYWORDS
# define _STLSOFT_FORCE_KEYWORD_EXPLICIT
# define _STLSOFT_FORCE_KEYWORD_MUTABLE
# define _STLSOFT_FORCE_KEYWORD_TYPENAME
#endif /* _STLSOFT_FORCE_ALL_KEYWORDS */

#if !defined(STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT) && \
    defined(_STLSOFT_FORCE_KEYWORD_EXPLICIT)
# define STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT
#endif /* !STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT && _STLSOFT_FORCE_KEYWORD_EXPLICIT */

#if !defined(STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT) && \
    defined(_STLSOFT_FORCE_KEYWORD_MUTABLE)
# define STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT
#endif /* !STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT && _STLSOFT_FORCE_KEYWORD_MUTABLE */

#if !defined(STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT) && \
    defined(_STLSOFT_FORCE_KEYWORD_TYPENAME)
# define STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT
#endif /* !STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT && _STLSOFT_FORCE_KEYWORD_TYPENAME */

#if !defined(STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT) && \
    defined(_STLSOFT_FORCE_KEYWORD_TYPENAME)
# define STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT
#endif /* !STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT && _STLSOFT_FORCE_KEYWORD_TYPENAME */

#if !defined(STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT) && \
    defined(_STLSOFT_FORCE_KEYWORD_TYPENAME)
# define STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT
#endif /* !STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT && _STLSOFT_FORCE_KEYWORD_TYPENAME */

#if !defined(STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT) && \
    defined(_STLSOFT_FORCE_KEYWORD_TYPENAME)
# define STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT
#endif /* !STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT && _STLSOFT_FORCE_KEYWORD_TYPENAME */

#if !defined(STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT) && \
    defined(_STLSOFT_FORCE_KEYWORD_TYPENAME)
# define STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT
#endif /* !STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT && _STLSOFT_FORCE_KEYWORD_TYPENAME */

/* /////////////////////////////////////////////////////////////////////////
 * Calling convention
 */

/* Calling convention support symbols */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION

/** \def STLSOFT_CF_CDECL_SUPPORTED
 * \brief When defined, indicates that the compiler supports the <b>cdecl</b> calling convention.
 */
# define    STLSOFT_CF_CDECL_SUPPORTED

/** \def STLSOFT_CF_FASTCALL_SUPPORTED
 * \brief When defined, indicates that the compiler supports the <b>fastcall</b> calling convention.
 */
# define    STLSOFT_CF_FASTCALL_SUPPORTED

/** \def STLSOFT_CF_STDCALL_SUPPORTED
 * \brief When defined, indicates that the compiler supports the <b>stdcall</b> calling convention.
 */
# define    STLSOFT_CF_STDCALL_SUPPORTED

#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */


/* Calling convention keyword abstraction */

/** \def STLSOFT_CDECL
 * \brief Resolves to the <b>cdecl</b> keyword for the current compiler, or to nothing for compilers
 *  that do not support any calling conventions.
 */

#if !defined(STLSOFT_CDECL)
# define                        STLSOFT_CDECL
#endif /* !STLSOFT_CDECL */

/** \def STLSOFT_FASTCALL
 * \brief Resolves to the <b>fastcall</b> keyword for the current compiler
 */

#if !defined(STLSOFT_FASTCALL)
# if defined(STLSOFT_CF_FASTCALL_SUPPORTED)
#  error Compiler discrimination must define STLSOFT_FASTCALL if STLSOFT_CF_FASTCALL_SUPPORTED is defined
# else /* ? STLSOFT_CF_FASTCALL_SUPPORTED */
#  define                       STLSOFT_FASTCALL
# endif /* STLSOFT_CF_FASTCALL_SUPPORTED */
#endif /* !STLSOFT_FASTCALL */

/** \def STLSOFT_STDCALL
 * \brief Resolves to the <b>stdcall</b> keyword for the current compiler
 */

#if !defined(STLSOFT_STDCALL)
# if defined(STLSOFT_CF_STDCALL_SUPPORTED)
#  error Compiler discrimination must define STLSOFT_STDCALL if STLSOFT_CF_STDCALL_SUPPORTED is defined
# else /* ? STLSOFT_CF_STDCALL_SUPPORTED */
#  define                       STLSOFT_STDCALL
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */
#endif /* !STLSOFT_STDCALL */


/* Calling convention facility values */

/** \def STLSOFT_CC_CDECL_VALUE
 *
 * An unique (flag) integer value set to
 * <code>0x01</code>
 * if the
 * <strong>cdecl</strong>
 * calling convention is supported, otherwise set to
 * <code>0</code>.
 */

/** \def STLSOFT_CC_FASTCALL_VALUE
 *
 * An unique (flag) integer value set to
 * <code>0x02</code>
 * if the
 * <strong>fastcall</strong>
 * calling convention is supported, otherwise set to
 * <code>0</code>.
 */

/** \def STLSOFT_CC_STDCALL_VALUE
 *
 * An unique (flag) integer value set to
 * <code>0x04</code>
 * if the
 * <strong>stdcall</strong>
 * calling convention is supported, otherwise set to
 * <code>0</code>.
 */

/** \def STLSOFT_CC_COMBINED_VALUE
 *
 * Combination of STLSOFT_CC_CDECL_VALUE, STLSOFT_CC_FASTCALL_VALUE
 * and STLSOFT_CC_STDCALL_VALUE.
 */

#if defined(STLSOFT_CF_CDECL_SUPPORTED)
# define STLSOFT_CDECL_VALUE            (1)     /*!< \deprecated This symbol is deprecated, and will be removed from a future version */
# define STLSOFT_CC_CDECL_VALUE         (0x01)
#else
# define STLSOFT_CC_CDECL_VALUE         (0)
#endif /* STLSOFT_CF_CDECL_SUPPORTED */

#if defined(STLSOFT_CF_FASTCALL_SUPPORTED)
# define STLSOFT_FASTCALL_VALUE         (2)     /*!< \deprecated This symbol is deprecated, and will be removed from a future version */
# define STLSOFT_CC_FASTCALL_VALUE      (0x02)
#else
# define STLSOFT_CC_FASTCALL_VALUE      (0)
#endif /* STLSOFT_CF_FASTCALL_SUPPORTED */

#if defined(STLSOFT_CF_STDCALL_SUPPORTED)
# define STLSOFT_STDCALL_VALUE          (3)     /*!< \deprecated This symbol is deprecated, and will be removed from a future version */
# define STLSOFT_CC_STDCALL_VALUE       (0x04)
#else
# define STLSOFT_CC_STDCALL_VALUE       (0)
#endif /* STLSOFT_CF_STDCALL_SUPPORTED */

#define STLSOFT_CC_COMBINED_VALUE   (STLSOFT_CC_CDECL_VALUE | STLSOFT_CC_FASTCALL_VALUE | STLSOFT_CC_STDCALL_VALUE)


/* /////////////////////////////////////////////////////////////////////////
 * operator bool()
 *
 * If the symbol STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT
 * is defined, operator bool should be defined as follows:
 *
 *  class X
 *  {
 *  private:
 *    struct boolean { int i; }
 *    typedef int boolean::*boolean_t;
 *  public:
 *    operator boolean_t () const;
 *
 * otherwise it should be
 *
 *  class X
 *  {
 *  private:
 *    typedef ss_bool_t boolean_t;
 *  public:
 *    operator boolean_t () const;
 *
 *
 * If the symbol STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT
 * is defined, it means that (!x) can de deduced by the compiler, otherwise it
 * will need to be provided
 *
 * If STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT is not defined
 * then STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT should not be
 * defined, so we do a check here.
 *
 */

#if !defined(STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT) && \
    defined(STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT)
# error Cannot rely on use of boolean as pointer to member for operator !
# error Undefine STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT when
# error STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT is not defined
#endif /* !STLSOFT_CF_OPERATOR_BOOL_AS_OPERATOR_POINTER_TO_MEMBER_SUPPORT && STLSOFT_CF_OPERATOR_NOT_VIA_OPERATOR_POINTER_TO_MEMBER_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Obsolete symbol definitions
 *
 * Define _STLSOFT_INCLUDE_OBSOLETE to include the definitions of symbols prior
 * to version 1.5.1
 */

/* Verify that the significant changes to STLSoft 1.5.1 are checked with respect
 * to other previously released projects
 */

#if (   defined(_ATLSTL_VER) && \
        _ATLSTL_VER <= 0x00010204) || \
    (   defined(_COMSTL_VER) && \
        _COMSTL_VER <= 0x00010201) || \
    (   defined(_MFCSTL_VER) && \
        _MFCSTL_VER <= 0x00010202) || \
    (   defined(_UNIXSTL_VER) && \
        _UNIXSTL_VER <= 0x00000901) || \
    (   defined(_WINSTL_VER) && \
        _WINSTL_VER <= 0x00010201)
# ifdef STLSOFT_STRICT
#  error You are using an old version of one or more of ATLSTL, COMSTL, MFCSTL, UNIXSTL and WinSTL. Please upgrade all dependent projects in line with the STLSoft version you are using
# else /* ? STLSOFT_STRICT */
#  ifdef STLSOFT_COMPILE_VERBOSE
#   pragma message("You are using an old version of one or more of ATLSTL, COMSTL, MFCSTL, UNIXSTL and WinSTL. _STLSOFT_INCLUDE_OBSOLETE will be defined (but is not guaranteed to work!)")
#  endif /* STLSOFT_COMPILE_VERBOSE */
#  ifndef _STLSOFT_INCLUDE_OBSOLETE
#   define _STLSOFT_INCLUDE_OBSOLETE
#  endif /* !_STLSOFT_INCLUDE_OBSOLETE */
# endif /* STLSOFT_STRICT */
#endif /* sub-project versions */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
# ifdef _STLSOFT_NO_STD_INCLUDES
#  undef _STLSOFT_NO_STD_INCLUDES
# endif /* _STLSOFT_NO_STD_INCLUDES */
#endif /* STLSOFT_DOCUMENTATION_SKIP_SECTION */

#ifndef _STLSOFT_NO_STD_INCLUDES
# ifndef STLSOFT_INCL_H_STDDEF
#  define STLSOFT_INCL_H_STDDEF
#  include <stddef.h>    /* standard types */
# endif /* !STLSOFT_INCL_H_STDDEF */
# ifndef STLSOFT_INCL_H_STDLIB
#  define STLSOFT_INCL_H_STDLIB
#  include <stdlib.h>    /* standard constants */
# endif /* !STLSOFT_INCL_H_STDLIB */
#endif /* !_STLSOFT_NO_STD_INCLUDES */

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcement
 *
 * The macro STLSOFT_ASSERT provides standard debug-mode assert functionality.
 */

/** \defgroup assertion_macros Assertion Macros
 * \ingroup group__project__stlsoft
 * \brief These macros are used for debugging / contract-enforcement
 * @{
 */

#if defined(_STLSOFT_NO_ASSERT) && \
    defined(STLSOFT_CF_ASSERT_SUPPORT)
# undef STLSOFT_CF_ASSERT_SUPPORT
#endif /* _STLSOFT_NO_ASSERT && STLSOFT_CF_ASSERT_SUPPORT */

/** \def STLSOFT_ASSERT(expr)
 * \brief Defines an assertion construct for runtime verification.
 *
 * \param expr Must be non-zero, or an assertion will be fired
 *
 * \note By default, the macro resolves to the standard macro
 *  <code>assert()</code> or the equivalent default assertion macro
 *  for the compiler (e.g. <code>_ASSERTE()</code> for Visual C++).
 *  This behaviour can be overriden by not defining: see the
 *  compiler capability file for your compiler (e.g.
 *  <code>include/stlsoft/internal/cccap/gcc.h</code>) for further
 *  details.
 */
#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define STLSOFT_ASSERT(expr)                   assert(expr)
#elif defined(STLSOFT_CF_ASSERT_SUPPORT)
# ifdef __STLSOFT_CF_USE_cassert
  /* Using the standard assertion mechanism, located in <cassert> */
#  ifdef __cplusplus
#   include <cassert>
#  else /* ? __cplusplus */
#   include <assert.h>
#  endif /* __cplusplus */
#  define STLSOFT_ASSERT(expr)                  assert(expr)
# else /* ? __STLSOFT_CF_USE_cassert */
  /* Using either a custom or proprietary assertion mechanism, so must
   * provide the header include name
   */
#  ifndef __STLSOFT_CF_ASSERT_INCLUDE_NAME
#   error Must supply an assert include filename with custom or proprietary assertion mechanism
#  else /* ? __STLSOFT_CF_ASSERT_INCLUDE_NAME */
#   include __STLSOFT_CF_ASSERT_INCLUDE_NAME
#  endif /* !__STLSOFT_CF_ASSERT_INCLUDE_NAME */
# endif /* __STLSOFT_CF_USE_cassert */
# if !defined(STLSOFT_ASSERT) && \
     defined(stlsoft_assert)
#  define STLSOFT_ASSERT                        stlsoft_assert
# elif defined(STLSOFT_ASSERT) && \
       defined(stlsoft_assert)
#  error Your compiler discrimination file cannot define both STLSOFT_ASSERT and stlsoft_assert. The former is the new version, and replaces the latter
# endif /* STLSOFT_ASSERT / stlsoft_assert */
# ifndef STLSOFT_ASSERT
#  error If your compiler discrimination file supports assertions, it must define STLSOFT_ASSERT() (taking a single parameter)
# endif /* !STLSOFT_ASSERT */
#endif /* !STLSOFT_CF_ASSERT_SUPPORT */


#ifdef STLSOFT_DOCUMENTATION_SKIP_SECTION
# undef stlsoft_assert
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \def stlsoft_assert(expr)
 *
 * \brief [DEPRECATED] Defines a runtime assertion
 *
 * \param expr Must be non-zero, or an assertion will be fired
 *
 * \deprecated This is deprecated in favour of \ref STLSOFT_ASSERT().
 *
 * \note This is a simple \#define for STLSOFT_ASSERT()
 */
#ifndef stlsoft_assert
# define stlsoft_assert(expr)                   STLSOFT_ASSERT(expr)
#endif /* !stlsoft_assert */


/** \def STLSOFT_MESSAGE_ASSERT(msg, expr)
 * \brief Defines a runtime assertion, with message
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 */
#if defined(STLSOFT_CF_ASSERT_SUPPORT)
# if defined(__WATCOMC__)
#  define STLSOFT_MESSAGE_ASSERT(msg, expr)     STLSOFT_ASSERT(expr)
# elif defined(__COMO__) || \
       defined(__GNUC__) || \
       defined(__MWERKS__)
#  define STLSOFT_MESSAGE_ASSERT(msg, expr)     STLSOFT_ASSERT((msg && (expr)))
# else /* ? compiler */
#  define STLSOFT_MESSAGE_ASSERT(msg, expr)     STLSOFT_ASSERT((msg, expr))
# endif /* __WATCOMC__ */
#else /* ? STLSOFT_CF_ASSERT_SUPPORT */
# define STLSOFT_MESSAGE_ASSERT(msg, expr)
#endif /* STLSOFT_CF_ASSERT_SUPPORT */

/** \def stlsoft_message_assert(expr)
 *
 * \brief [DEPRECATED] Defines a runtime assertion, with message
 *
 * \param expr Must be non-zero, or an assertion will be fired
 * \param msg The literal character string message to be included in the assertion
 *
 * \deprecated This is deprecated in favour of \ref STLSOFT_MESSAGE_ASSERT().
 *
 * \note This is a simple \#define for STLSOFT_MESSAGE_ASSERT()
 */
#define stlsoft_message_assert(msg, expr)       STLSOFT_MESSAGE_ASSERT(msg, expr)

/*
 * TODO: decide on a form of static_assert with message (as the C++11 static_assert does)
 */

/** \def STLSOFT_STATIC_ASSERT(expr)
 *
 * \brief Defines an assertion construct for compile-time verification.
 *
 * \param expr A compile-time evaluatable condition that must be non-zero, or compilation will fail.
 */
#if defined(STLSOFT_CF_static_assert_SUPPORT)
# define STLSOFT_STATIC_ASSERT(expr)        static_assert((expr), #expr)
#elif defined(STLSOFT_CF_STATIC_ASSERT_SUPPORT)
# if defined(STLSOFT_COMPILER_IS_GCC) || \
      defined(STLSOFT_COMPILER_IS_INTEL)
#  define STLSOFT_STATIC_ASSERT(expr)       do { typedef int ai[(expr) ? 1 : -1]; } while(0)
# else /* ? compiler */
#  define STLSOFT_STATIC_ASSERT(expr)       do { typedef int ai[(expr) ? 1 : 0]; } while(0)
# endif /* compiler */
#else /* ? STLSOFT_CF_STATIC_ASSERT_SUPPORT */
# define STLSOFT_STATIC_ASSERT(expr)        STLSOFT_MESSAGE_ASSERT("Static assertion failed: ", (expr))
#endif /* STLSOFT_CF_STATIC_ASSERT_SUPPORT */

/** \def stlsoft_static_assert(expr)
 *
 * \brief [DEPRECATED] Defines a compile-time assertion
 *
 * \param expr Must be non-zero, or compilation will fail
 *
 * \note This is a simple \#define for STLSOFT_STATIC_ASSERT()
 *
 * \deprecated This is <b>heavily</b> deprecated in favour of \ref STLSOFT_STATIC_ASSERT().
 */
#define stlsoft_static_assert(expr)         STLSOFT_STATIC_ASSERT(expr)

/** @} */

/* /////////////////////////////////////////////////////////////////////////
 * Deprecation
 *
 */

/** \def STLSOFT_DECLARE_DEPRECATION()
 *
 * Marks the succeeding symbol as being deprecated in a compiler-dependent
 * manner.
 *
 * \see STLSOFT_DECLARE_DEPRECATION_MESSAGE
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF
 *
 * \ingroup group__utility
 */

/** \def STLSOFT_DECLARE_DEPRECATION_MESSAGE(message)
 *
 * Used to attach a custom deprecation message to the succeeding symbol, for
 * supporting compilers, in a compiler-dependent manner.
 *
 * \param message The custom deprecation message to attach to the symbol.
 *
 * \see STLSOFT_DECLARE_DEPRECATION
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF
 *
 * \ingroup group__utility
 */

/** \def STLSOFT_DECLARE_FUNCTION_DEPRECATION(fn)
 *
 * Used to attach a standard deprecation message to the succeeding function,
 * for supporting compilers, in a compiler-dependent manner.
 *
 * \param fn The name of the symbol.
 *
 * \ingroup group__utility
 *
 * \see STLSOFT_DECLARE_DEPRECATION
 * \see STLSOFT_DECLARE_DEPRECATION_MESSAGE
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF
 */

/** \def STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(oldfn, newfn)
 *
 * Used to attach a standard deprecation message to the succeeding function,
 * stipulating a function to use instead, for supporting compilers, in a
 * compiler-dependent manner.
 *
 * \param oldfn The name of the symbol being deprecated.
 * \param newfn The name of the new symbol to be used instead.
 *
 * \ingroup group__utility
 *
 * \see STLSOFT_DECLARE_DEPRECATION
 * \see STLSOFT_DECLARE_DEPRECATION_MESSAGE
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF
 */

/** \def STLSOFT_DECLARE_MACRO_DEPRECATION(mac)
 *
 * Used to attach a standard deprecation message to the succeeding macro,
 * for supporting compilers, in a compiler-dependent manner.
 *
 * \param mac The name of the symbol.
 *
 * \ingroup group__utility
 *
 * \see STLSOFT_DECLARE_DEPRECATION
 * \see STLSOFT_DECLARE_DEPRECATION_MESSAGE
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF
 */

/** \def STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(oldmac, newmac)
 *
 * Used to attach a standard deprecation message to the succeeding macro,
 * stipulating a macro to use instead, for supporting compilers, in a
 * compiler-dependent manner.
 *
 * \param oldmac The name of the symbol being deprecated.
 * \param newmac The name of the new symbol to be used instead.
 *
 * \ingroup group__utility
 *
 * \see STLSOFT_DECLARE_DEPRECATION
 * \see STLSOFT_DECLARE_DEPRECATION_MESSAGE
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION
 * \see STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF
 * \see STLSOFT_DECLARE_MACRO_DEPRECATION
 */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    STLSOFT_MSVC_VER >= 140050320

# define STLSOFT_DECLARE_DEPRECATION()                                      \
                                                                            \
    __declspec(deprecated)

# define STLSOFT_DECLARE_DEPRECATION_MESSAGE(message)                       \
                                                                            \
    __declspec(deprecated(message))

# define STLSOFT_DECLARE_FUNCTION_DEPRECATION(fn)                           \
                                                                            \
    STLSOFT_DECLARE_DEPRECATION_MESSAGE("The function " STLSOFT_STRINGIZE(fn) " is deprecated and will be removed from a future version of STLSoft")

# define STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(oldfn, newfn)    \
                                                                            \
    STLSOFT_DECLARE_DEPRECATION_MESSAGE("The function " STLSOFT_STRINGIZE(oldfn) " is deprecated and will be removed from a future version of STLSoft; use " STLSOFT_STRINGIZE(newfn) " instead")

# define STLSOFT_DECLARE_MACRO_DEPRECATION(fn)

# define STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(oldfn, newfn)

#else /* ? compiler */
# define STLSOFT_DECLARE_DEPRECATION()
# define STLSOFT_DECLARE_DEPRECATION_MESSAGE(message)
# define STLSOFT_DECLARE_FUNCTION_DEPRECATION(fn)
# define STLSOFT_DECLARE_FUNCTION_DEPRECATION_IN_FAVOUR_OF(oldfn, newfn)
# define STLSOFT_DECLARE_MACRO_DEPRECATION(mac)
# define STLSOFT_DECLARE_MACRO_DEPRECATION_IN_FAVOUR_OF(oldmac, newmac)
#endif /* compiler */


/* /////////////////////////////////////////////////////////////////////////
 * Namespace
 *
 * The STLSoft uses namespaces by default, unless the _STLSOFT_NO_NAMESPACES
 * preprocessor symbol is defined, in which case all elements are placed within
 * the global namespace.
 *
 * The macro stlsoft_ns_qual() macro can be used to refer to elements in the
 * STLSoft libraries irrespective of whether they are in the stlsoft namespace
 * or in the global namespace.
 *
 * Some compilers do not support the standard library in the std namespace, so
 * the stlsoft_ns_qual_std() macro can be used to refer to elements in the
 * STLSoft libraries irrespective of whether they are in the std namespace or
 * in the global namespace.
 */

/* No C++ compilation means no namespaces */
#if !defined(__cplusplus)
# if !defined(_STLSOFT_NO_NAMESPACES)
#  define _STLSOFT_NO_NAMESPACES
# endif /* !_STLSOFT_NO_NAMESPACES */
#endif /* !__cplusplus */

/* No STLSoft namespaces means no stlsoft namespace */
#if defined(_STLSOFT_NO_NAMESPACES)
# if !defined(_STLSOFT_NO_NAMESPACE)
#  define _STLSOFT_NO_NAMESPACE
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* _STLSOFT_NO_NAMESPACES */

#ifndef _STLSOFT_NO_NAMESPACE
/** \brief The <code class="namespace">stlsoft</code> namespace contains
 *   all components in the \ref group__project__stlsoft "STLSoft" project,
 *   and is the root namespace within which all the other
 *   \ref group__projects "STLSoft sub-projects" reside.
 *
 * \note If either/both of the symbols <code>_STLSOFT_NO_NAMESPACES</code>
 * and <code>_STLSOFT_NO_NAMESPACE</code> are defined, all
 * \ref group__project__stlsoft "STLSoft" components will be defined in the
 * global namespace. The difference between the two is that definition of
 * <code>_STLSOFT_NO_NAMESPACES</code> causes all STLSoft library
 * components (i.e. those of <b>all</b>
 * \ref group__projects "STLSoft sub-projects") to be defined in the
 * global namespace, whereas <code>_STLSOFT_NO_NAMESPACES</code> has this
 * effect on only those within the main
 * \ref group__project__stlsoft "STLSoft" project.
 *
 * \note This is a vestige of compatibility with compilers with
 * no (or no sensible) namespace support that is maintained for reasons of
 * backwards compatiblity and because it is, in <i>rare circumstances</i>, a
 * useful facility.
 */
namespace stlsoft
{
#endif /* !_STLSOFT_NO_NAMESPACE */

/** \def stlsoft_ns_qual(x)
 * \brief Qualifies with <b>stlsoft::</b> if STLSoft is using namespaces or, if not, does not qualify
 */

/** \def stlsoft_ns_using(x)
 * \brief Declares a using directive (with respect to <b>stlsoft</b>) if STLSoft is using namespaces or, if not, does nothing
 */

#if !defined(_STLSOFT_NO_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define stlsoft_ns_qual(x)          ::stlsoft::x
# define stlsoft_ns_using(x)         using ::stlsoft::x;
#else /* ? _STLSOFT_NO_NAMESPACE */
# define stlsoft_ns_qual(x)          x
# define stlsoft_ns_using(x)
#endif /* !_STLSOFT_NO_NAMESPACE */

/** \def stlsoft_ns_qual_std(x)
 * \brief Qualifies with <b>std::</b> if STLSoft is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does not qualify
 */

/** \def stlsoft_ns_using_std(x)
 * \brief Declares a using directive (with respect to <b>std</b>) if STLSoft is being translated in the context of the standard library being within the <b>std</b> namespace or, if not, does nothing
 */

#if defined(STLSOFT_CF_std_NAMESPACE) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define stlsoft_ns_qual_std(x)      ::std::x
# define stlsoft_ns_qual_std_(x)     std::x
# define stlsoft_ns_using_std(x)     using ::std::x;
#else /* ? STLSOFT_CF_std_NAMESPACE */
# define stlsoft_ns_qual_std(x)      x
# define stlsoft_ns_using_std(x)
#endif /* !STLSOFT_CF_std_NAMESPACE */

/** \def STLSOFT_NS_GLOBAL(X)
 * \brief Qualifies <b>X</b> with <b>::</b> if compiling C++, otherwise just resolves to X
 */

#if defined(__cplusplus) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define STLSOFT_NS_GLOBAL(x)       ::x
#else /* ? __cplusplus */
# define STLSOFT_NS_GLOBAL(x)       x
#endif /* __cplusplus */

/* This stuff allows worker namespaces to be declared and used regardless of
 * whether namespaces are supported and allowed
 */
#if !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) && \
    !defined(_STLSOFT_NO_NAMESPACES)
# define STLSOFT_OPEN_WORKER_NS_(ns)        namespace ns {
# define STLSOFT_CLOSE_WORKER_NS_(ns)       }
# define STLSOFT_WORKER_NS_QUAL_(ns, x)     ns::x
#else /* ? namespaces supported */
# define STLSOFT_OPEN_WORKER_NS_(ns)
# define STLSOFT_CLOSE_WORKER_NS_(ns)
# define STLSOFT_WORKER_NS_QUAL_(ns, x)     x
#endif /* namespaces supported */

/* /////////////////////////////////////////////////////////////////////////
 * 64-bit warning support
 */

#if defined(STLSOFT_COMPILER_IS_MSVC) && \
    _MSC_VER >= 1310 && \
    !defined(_WIN64) && \
    defined(_Wp64)
# define STLSOFT_WARN_64                    __w64
#else /* ? compiler */
# define STLSOFT_WARN_64
#endif /* compiler */

/* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 *
 * The STLSoft uses a number of typedefs to aid in compiler-independence in the
 * libraries' main code.
 */

/* Type definitions - precursors */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

 /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */

# if defined(__STLSOFT_CF_NATIVE_BOOL_SUPPORT) && \
     !defined(STLSOFT_CF_NATIVE_BOOL_SUPPORT)
#  error Configuration error: deprecated symbol __STLSOFT_CF_NATIVE_BOOL_SUPPORT is defined when STLSOFT_CF_NATIVE_BOOL_SUPPORT is not!
# else /* ? STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#  ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
#   define __STLSOFT_CF_NATIVE_BOOL_SUPPORT
#  endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */
# endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */

 /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

# if defined(__STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT) && \
     !defined(STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT)
#  error Configuration error: deprecated symbol __STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT is defined when STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT is not!
# else /* ? STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#  ifdef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
#   define __STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
#  endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */
# endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

 /* STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT */

# if defined(__STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT) && \
     !defined(STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT)
#  error Configuration error: deprecated symbol __STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT is defined when STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT is not!
# else /* ? STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#  ifdef STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
#   define __STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT
#  endif /* STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT */
# endif /* STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT */

 /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

# if (  defined(STLSOFT_CF_INT_DISTINCT_TYPE) || \
        defined(__STLSOFT_CF_INT_DISTINCT_TYPE)) && \
     !defined(STLSOFT_CF_INT_DISTINCT_INT_TYPE)
#  error Configuration error: one or both of the deprecated symbols STLSOFT_CF_INT_DISTINCT_TYPE or __STLSOFT_CF_INT_DISTINCT_TYPE is defined when STLSOFT_CF_INT_DISTINCT_INT_TYPE is not!
# else /* ? STLSOFT_CF_INT_DISTINCT_INT_TYPE */
#  ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
#   define STLSOFT_CF_INT_DISTINCT_TYPE
#   define __STLSOFT_CF_INT_DISTINCT_TYPE
#  endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */
# endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

 /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

# if (  defined(STLSOFT_CF_LONG_DISTINCT_TYPE) || \
        defined(__STLSOFT_CF_LONG_DISTINCT_TYPE)) && \
     !defined(STLSOFT_CF_LONG_DISTINCT_INT_TYPE)
#  error Configuration error: one or both of the deprecated symbols STLSOFT_CF_LONG_DISTINCT_TYPE or __STLSOFT_CF_LONG_DISTINCT_TYPE is defined when STLSOFT_CF_LONG_DISTINCT_INT_TYPE is not!
# else /* ? STLSOFT_CF_LONG_DISTINCT_INT_TYPE */
#  ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
#   define STLSOFT_CF_LONG_DISTINCT_TYPE
#   define __STLSOFT_CF_LONG_DISTINCT_TYPE
#  endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */
# endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */



/* wchar_t
 *
 * wchar_t is either a built-in type, or is defined to an unsigned 16-bit value
 */

#ifdef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
 /* It's some kind of compiler native type. */
# ifndef STLSOFT_NATIVE_WCHAR_T
  /* either wchar_t itself */
#  define STLSOFT_WCHAR_T_BASE_TYPE_    wchar_t
# else /* ? STLSOFT_NATIVE_WCHAR_T */
  /* or a compiler-specific type */
#  define STLSOFT_WCHAR_T_BASE_TYPE_    STLSOFT_NATIVE_WCHAR_T
# endif /* !STLSOFT_NATIVE_WCHAR_T */
#elif defined(STLSOFT_CF_TYPEDEF_WCHAR_T_SUPPORT)
#  define STLSOFT_WCHAR_T_BASE_TYPE_    wchar_t
#else /* ? wchar_t support */
 /* It's some kind of library-defined type. */
# ifndef _STLSOFT_NO_STD_INCLUDES
#  define STLSOFT_WCHAR_T_BASE_TYPE_    wchar_t
# else /* ? _STLSOFT_NO_STD_INCLUDES */
#  define STLSOFT_WCHAR_T_BASE_TYPE_    unsigned short
# endif /* _STLSOFT_NO_STD_INCLUDES */
#endif /* !STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

/* 8-bit */
#ifdef STLSOFT_CF_8BIT_INT_SUPPORT
 typedef STLSOFT_SI08_T_BASE_TYPE       STLSOFT_I_08_T_BASE_TYPE_;
 typedef STLSOFT_SI08_T_BASE_TYPE       STLSOFT_SI08_T_BASE_TYPE_;
 typedef STLSOFT_UI08_T_BASE_TYPE       STLSOFT_UI08_T_BASE_TYPE_;
#else /* ? STLSOFT_CF_8BIT_INT_SUPPORT */
# error STLSoft requires 8-bit integer support
#endif /* STLSOFT_CF_8BIT_INT_SUPPORT */

/* 16-bit */
#ifdef STLSOFT_CF_16BIT_INT_SUPPORT
 typedef STLSOFT_SI16_T_BASE_TYPE       STLSOFT_I_16_T_BASE_TYPE_;
 typedef STLSOFT_SI16_T_BASE_TYPE       STLSOFT_SI16_T_BASE_TYPE_;
 typedef STLSOFT_UI16_T_BASE_TYPE       STLSOFT_UI16_T_BASE_TYPE_;
#else /* ? STLSOFT_CF_16BIT_INT_SUPPORT */
# error STLSoft requires 16-bit integer support
#endif /* STLSOFT_CF_16BIT_INT_SUPPORT */

/* 32-bit */
#ifdef STLSOFT_CF_32BIT_INT_SUPPORT
 typedef STLSOFT_SI32_T_BASE_TYPE       STLSOFT_I_32_T_BASE_TYPE_;
 typedef STLSOFT_SI32_T_BASE_TYPE       STLSOFT_SI32_T_BASE_TYPE_;
 typedef STLSOFT_UI32_T_BASE_TYPE       STLSOFT_UI32_T_BASE_TYPE_;
#else /* ? STLSOFT_CF_32BIT_INT_SUPPORT */
# error STLSoft requires 32-bit integer support
#endif /* STLSOFT_CF_32BIT_INT_SUPPORT */

/* 64-bit */
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
 typedef STLSOFT_SI64_T_BASE_TYPE       STLSOFT_I_64_T_BASE_TYPE_;
 typedef STLSOFT_SI64_T_BASE_TYPE       STLSOFT_SI64_T_BASE_TYPE_;
 typedef STLSOFT_UI64_T_BASE_TYPE       STLSOFT_UI64_T_BASE_TYPE_;
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* Type definitions - proper */

typedef char                            ss_char_a_t;        /*!< Ansi char type             */
typedef STLSOFT_WCHAR_T_BASE_TYPE_      ss_char_w_t;        /*!< Unicode char type          */
typedef STLSOFT_I_08_T_BASE_TYPE_       ss_int8_t;          /*!< 8-bit integer              */
typedef STLSOFT_SI08_T_BASE_TYPE_       ss_sint8_t;         /*!< 8-bit signed integer       */
typedef STLSOFT_UI08_T_BASE_TYPE_       ss_uint8_t;         /*!< 8-bit unsigned integer     */
typedef STLSOFT_I_16_T_BASE_TYPE_       ss_int16_t;         /*!< 16-bit integer             */
typedef STLSOFT_SI16_T_BASE_TYPE_       ss_sint16_t;        /*!< 16-bit signed integer      */
typedef STLSOFT_UI16_T_BASE_TYPE_       ss_uint16_t;        /*!< 16-bit unsigned integer    */
typedef STLSOFT_I_32_T_BASE_TYPE_       ss_int32_t;         /*!< 32-bit integer             */
typedef STLSOFT_SI32_T_BASE_TYPE_       ss_sint32_t;        /*!< 32-bit signed integer      */
typedef STLSOFT_UI32_T_BASE_TYPE_       ss_uint32_t;        /*!< 32-bit unsigned integer    */
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
 typedef STLSOFT_I_64_T_BASE_TYPE_      ss_int64_t;         /*!< 64-bit integer             */
 typedef STLSOFT_SI64_T_BASE_TYPE_      ss_sint64_t;        /*!< 64-bit signed integer      */
 typedef STLSOFT_UI64_T_BASE_TYPE_      ss_uint64_t;        /*!< 64-bit unsigned integer    */
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef short                           ss_short_t;         /*!< short integer              */
typedef int STLSOFT_WARN_64             ss_int_t;           /*!< integer                    */
typedef signed int STLSOFT_WARN_64      ss_sint_t;          /*!< signed integer             */
typedef unsigned int STLSOFT_WARN_64    ss_uint_t;          /*!< unsigned integer           */
typedef long STLSOFT_WARN_64            ss_long_t;          /*!< long integer               */
typedef unsigned long STLSOFT_WARN_64   ss_ulong_t;         /*!< long integer               */
typedef ss_uint8_t                      ss_byte_t;          /*!< Byte                       */
#if defined(__cplusplus)
# ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
typedef bool                            ss_bool_t;          /*!< Boolean type               */
# else /* ? STLSOFT_CF_NATIVE_BOOL_SUPPORT */
typedef unsigned int                    ss_bool_t;
# endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */
#endif /* __cplusplus */
#ifndef _STLSOFT_NO_STD_INCLUDES
 typedef ptrdiff_t                      ss_ptrdiff_t;       /*!< ptr diff                   */
 typedef size_t STLSOFT_WARN_64         ss_size_t;          /*!< size                       */
#else /* ? _STLSOFT_NO_STD_INCLUDES */
 typedef unsigned int STLSOFT_WARN_64   ss_size_t;
 typedef int                            ss_ptrdiff_t;
#endif /* !_STLSOFT_NO_STD_INCLUDES */
typedef long                            ss_streampos_t;     /*!< streampos                  */
typedef long                            ss_streamoff_t;     /*!< streamoff                  */

#ifndef _STLSOFT_NO_NAMESPACE
typedef ss_char_a_t                 char_a_t;           /*!< Ansi char type             */
typedef ss_char_w_t                 char_w_t;           /*!< Unicode char type          */
typedef ss_int8_t                   int8_t;             /*!< 8-bit integer              */
typedef ss_sint8_t                  sint8_t;            /*!< 8-bit signed integer       */
typedef ss_uint8_t                  uint8_t;            /*!< 8-bit unsigned integer     */
typedef ss_int16_t                  int16_t;            /*!< 16-bit integer             */
typedef ss_sint16_t                 sint16_t;           /*!< 16-bit signed integer      */
typedef ss_uint16_t                 uint16_t;           /*!< 16-bit unsigned integer    */
typedef ss_int32_t                  int32_t;            /*!< 32-bit integer             */
typedef ss_sint32_t                 sint32_t;           /*!< 32-bit signed integer      */
typedef ss_uint32_t                 uint32_t;           /*!< 32-bit unsigned integer    */
# ifdef STLSOFT_CF_64BIT_INT_SUPPORT
typedef ss_int64_t                  int64_t;            /*!< 64-bit integer             */
typedef ss_sint64_t                 sint64_t;           /*!< 64-bit signed integer      */
typedef ss_uint64_t                 uint64_t;           /*!< 64-bit unsigned integer    */
# endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
typedef ss_short_t                  short_t;            /*!< short integer              */
typedef ss_int_t STLSOFT_WARN_64    int_t;              /*!< integer                    */
typedef ss_sint_t STLSOFT_WARN_64   sint_t;             /*!< signed integer             */
typedef ss_uint_t STLSOFT_WARN_64   uint_t;             /*!< unsigned integer           */
typedef ss_long_t STLSOFT_WARN_64   long_t;             /*!< long integer               */
typedef ss_byte_t                   byte_t;             /*!< Byte                       */
# if defined(__cplusplus)
typedef ss_bool_t                   bool_t;             /*!< bool                       */
# endif /* __cplusplus */
# if !defined(STLSOFT_COMPILER_IS_DMC)
typedef ss_streampos_t              streampos_t;        /*!< streampos                  */
typedef ss_streamoff_t              streamoff_t;        /*!< streamoff                  */
# endif /* compiler */
#endif /* !_STLSOFT_NO_NAMESPACE */


#if 0
template <ss_size_t N>
struct uintp_traits;

STLSOFT_GEN_TRAIT_SPECIALISATION
struct uintp_traits<1>
{
    typedef uint8_t     unsigned_type;
}

typedef size_traits<sizeof(void*)>::signed_type     sintp_t;
typedef size_traits<sizeof(void*)>::unsigned_type   uintp_t;

#endif /* 0 */



#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# if defined(__cplusplus)

struct native_wchar_t_checker
{
    /* Character types. */

    void check(char)
    {}
#ifdef STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT
    void check(wchar_t)
    {}
#endif /* STLSOFT_CF_NATIVE_WCHAR_T_SUPPORT */

    /* Sized integer types. */

    void check(ss_sint8_t)
    {}
    void check(ss_uint8_t)
    {}
    void check(ss_sint16_t)
    {}
    void check(ss_uint16_t)
    {}
    void check(ss_sint32_t)
    {}
    void check(ss_uint32_t)
    {}
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
    void check(ss_sint64_t)
    {}
    void check(ss_uint64_t)
    {}
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

    /* Natural integer types. */

#ifdef STLSOFT_CF_CHAR_DISTINCT_INT_TYPE
    void check(signed char)
    {}
    void check(unsigned char)
    {}
#endif /* STLSOFT_CF_CHAR_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_SHORT_DISTINCT_INT_TYPE
    void check(signed short)
    {}
    void check(unsigned short)
    {}
#endif /* STLSOFT_CF_SHORT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_INT_DISTINCT_INT_TYPE
    void check(signed int)
    {}
    void check(unsigned int)
    {}
#endif /* STLSOFT_CF_INT_DISTINCT_INT_TYPE */

#ifdef STLSOFT_CF_LONG_DISTINCT_INT_TYPE
    void check(signed long)
    {}
    void check(unsigned long)
    {}
#endif /* STLSOFT_CF_LONG_DISTINCT_INT_TYPE */

    ~native_wchar_t_checker()
    {
        /* Character types. */
        check(char());
        check(wchar_t());

        /* Sized integer types. */
        check(ss_sint8_t());
        check(ss_uint8_t());
        check(ss_sint16_t());
        check(ss_uint16_t());
        check(ss_sint32_t());
        check(ss_uint32_t());
#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
        check(ss_sint64_t());
        check(ss_uint64_t());
#endif /* STLSOFT_CF_64BIT_INT_SUPPORT */

        /* Natural integer types. */
        check(static_cast<signed char>(0));
        check(static_cast<unsigned char>(0));
        check(static_cast<signed short>(0));
        check(static_cast<unsigned short>(0));
        check(static_cast<signed int>(0));
        check(static_cast<unsigned int>(0));
        check(static_cast<signed long>(0));
        check(static_cast<unsigned long>(0));
    }
};

struct stlsoft_size_checker
{
#  ifdef STLSOFT_COMPILER_IS_GCC
protected: /* GCC is too "helpful" in this case, so must declare as protected */
#  else /* ? compiler */
private:
#  endif /* compiler */
    stlsoft_size_checker();
    ~stlsoft_size_checker()
    {
        /* Char types */
        STLSOFT_STATIC_ASSERT(sizeof(ss_char_a_t) >= 1);
        STLSOFT_STATIC_ASSERT(sizeof(ss_char_w_t) >= 2);
        /* 8-bit types */
        STLSOFT_STATIC_ASSERT(sizeof(ss_int8_t)   == 1);
        STLSOFT_STATIC_ASSERT(sizeof(ss_sint8_t)  == sizeof(ss_int8_t));
        STLSOFT_STATIC_ASSERT(sizeof(ss_uint8_t)  == sizeof(ss_int8_t));
        /* 16-bit types */
        STLSOFT_STATIC_ASSERT(sizeof(ss_int16_t)  == 2);
        STLSOFT_STATIC_ASSERT(sizeof(ss_sint16_t) == sizeof(ss_int16_t));
        STLSOFT_STATIC_ASSERT(sizeof(ss_uint16_t) == sizeof(ss_int16_t));
        /* 32-bit types */
        STLSOFT_STATIC_ASSERT(sizeof(ss_int32_t)  == 4);
        STLSOFT_STATIC_ASSERT(sizeof(ss_sint32_t) == sizeof(ss_int32_t));
        STLSOFT_STATIC_ASSERT(sizeof(ss_uint32_t) == sizeof(ss_int32_t));
        /* 64-bit types */
#  ifdef STLSOFT_CF_64BIT_INT_SUPPORT
        STLSOFT_STATIC_ASSERT(sizeof(ss_int64_t)  == 8);
        STLSOFT_STATIC_ASSERT(sizeof(ss_sint64_t) == sizeof(ss_int64_t));
        STLSOFT_STATIC_ASSERT(sizeof(ss_uint64_t) == sizeof(ss_int64_t));
#  endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
        /* Integer types */
        STLSOFT_STATIC_ASSERT(sizeof(ss_int_t)    >= 1);
        STLSOFT_STATIC_ASSERT(sizeof(ss_sint_t)   == sizeof(ss_int_t));
        STLSOFT_STATIC_ASSERT(sizeof(ss_uint_t)   == sizeof(ss_int_t));
        STLSOFT_STATIC_ASSERT(sizeof(ss_long_t)   >= sizeof(ss_int_t));
        /* byte type */
        STLSOFT_STATIC_ASSERT(sizeof(ss_byte_t)   == 1);
        /* Boolean type */
        STLSOFT_STATIC_ASSERT(sizeof(ss_bool_t)   >= 1);
        /* Other types */
        STLSOFT_STATIC_ASSERT(sizeof(ss_size_t)   >= 1);
        STLSOFT_STATIC_ASSERT(sizeof(ss_ptrdiff_t) >= 1);
        STLSOFT_STATIC_ASSERT(sizeof(ss_streampos_t) >= 1);
        STLSOFT_STATIC_ASSERT(sizeof(ss_streamoff_t) >= 1);
    }
};
# endif /* __cplusplus */
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Keywords
 *
 * The STLSoft uses a number of preprocessor symbols to aid in compiler
 * compatibility in the libraries' code.
 *
 * ss_explicit_k            -   explicit, or nothing
 * ss_mutable_k             -   mutable, or nothing
 * ss_typename_type_k       -   typename, or nothing (used within template
 *                              definitions for declaring types derived from
 *                              externally derived types)
 * ss_typename_param_k      -   typename or class (used for template parameters)
 * ss_typename_type_def_k   -   typename qualifier in template default parameters
 * ss_typename_type_mil_k   -   typename qualifier in constructor initialiser lists
 * ss_typename_type_ret_k   -   typename qualifier in return types
 */

/** \defgroup pseudo_keyword_macros Pseudo-keyword Macros
 * \ingroup group__project__stlsoft
 * \brief These macros are used to handle difference in support of certain keywords between translators
 * @{
 */

/** \def ss_explicit_k
 *
 * \brief Evaluates to <b>explicit</b> on translators that support the keyword, otherwise to nothing.
 */
#if defined(STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define ss_explicit_k              explicit
#else /* ? STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT */
# define ss_explicit_k
#endif /* STLSOFT_CF_EXPLICIT_KEYWORD_SUPPORT */

/** \def ss_mutable_k
 *
 * \brief Evaluates to <b>mutable</b> on translators that support the keyword, otherwise to nothing.
 */
#if defined(STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define ss_mutable_k               mutable
#else /* ? STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT */
# define ss_mutable_k
#endif /* STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT */

/** \def ss_typename_param_k
 *
 * \brief Evaluates to <b>typename</b> on translators that support the keyword, otherwise to <b>class</b>
 *
 * Used for template parameter declarations
 */
#if defined(STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define ss_typename_param_k        typename
#else /* ? STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT */
# define ss_typename_param_k        class
#endif /* STLSOFT_CF_TYPENAME_PARAM_KEYWORD_SUPPORT */

/** \def ss_typename_type_k
 *
 * \brief Evaluates to <b>typename</b> on translators that support the keyword, otherwise to nothing.
 *
 * Used for type disambiguation inside template bodies
 */
#if defined(STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define ss_typename_type_k         typename
#else /* ? STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT */
# define ss_typename_type_k
#endif /* STLSOFT_CF_TYPENAME_TYPE_KEYWORD_SUPPORT */

/** \def ss_typename_type_def_k
 *
 * \brief Evaluates to <b>typename</b> on translators that support the keyword and its application in
 *   the context of default template arguments, otherwise to nothing.
 *
 * Used for type disambiguation in default template arguments
 */
#if defined(STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define ss_typename_type_def_k     typename
#else /* ? STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT */
# define ss_typename_type_def_k
#endif /* STLSOFT_CF_TYPENAME_TYPE_DEF_KEYWORD_SUPPORT */

/** \def ss_typename_type_mil_k
 *
 * \brief Evaluates to <b>typename</b> on translators that support the keyword and its application in
 *   the context of member initialisation lists, otherwise to nothing.
 *
 * Used for type disambiguation inside initialiser lists in class template constructors
 */
#if defined(STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define ss_typename_type_mil_k     typename
#else /* ? STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT */
# define ss_typename_type_mil_k
#endif /* STLSOFT_CF_TYPENAME_TYPE_MIL_KEYWORD_SUPPORT */

/** \def ss_typename_type_ret_k
 *
 * \brief Evaluates to <b>typename</b> on translators that support the keyword and its application in
 *   the context of member function/method return types, otherwise to nothing.
 *
 * Used for type disambiguation the return types in templates
 */
#if defined(STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define ss_typename_type_ret_k     typename
#else /* ? STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT */
# define ss_typename_type_ret_k
#endif /* STLSOFT_CF_TYPENAME_TYPE_RET_KEYWORD_SUPPORT */

/** \def ss_template_qual_k
 *
 * \brief Evaluates to <b>template</b> on translators that support its use for type qualification
 */
#if defined(STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT) || \
    defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define ss_template_qual_k         template
#else /* ? STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT */
# define ss_template_qual_k
#endif /* STLSOFT_CF_TEMPLATE_QUALIFIER_KEYWORD_SUPPORT */

/** @} */

/* /////////////////////////////////////////////////////////////////////////
 * Values
 *
 * Since the boolean type may not be supported natively on all compilers, the
 * values of true and false may also not be provided. Hence the values of
 * ss_true_v and ss_false_v are defined, and are used in all code.
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#ifdef STLSOFT_CF_NATIVE_BOOL_SUPPORT
# define ss_true_v       (true)
# define ss_false_v      (false)
#else /* ? STLSOFT_CF_NATIVE_BOOL_SUPPORT */
# define ss_true_v       (1)
# define ss_false_v      (0)
#endif /* STLSOFT_CF_NATIVE_BOOL_SUPPORT */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */
/* /////////////////////////////////////////////////////////////////////////
 * Exception signature macro(s)
 */

#ifdef __cplusplus

/** \brief Throws the given exception
 *
 * \note This is needed because Visual C++ 5.0 goes into an infinite loop if
 *        the exception instance is passed to throw. The apparent workaround
 *        (which only took me six years to discover!) is to apply *& to the
 *        instance.
 */
# if defined(STLSOFT_COMPILER_IS_BORLAND)

template <ss_typename_param_k X>
inline
void
throw_x(
    X const& x
) /* throw(X) */
{
    X   x2(x);

    throw x2;
}

#  define STLSOFT_THROW_X(x)    stlsoft_ns_qual(throw_x)(x)

# else

template <ss_typename_param_k X>
inline
void
throw_x(
    X const& x
)
{
#  if   defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1200
    X const* px =   &x;

    throw *px;
#  else /* ? compiler */
    throw x;
#  endif /* compiler */
}

#  define STLSOFT_THROW_X(x)    stlsoft_ns_qual(throw_x)(x)

# endif /* compiler */

#endif /* __cplusplus */

/** \defgroup group__project__stlsoft__code_modification_macros Code Modification Macros
 * \ingroup group__project__stlsoft
 * \brief These macros are used to help out where compiler differences are
 * so great as to cause great disgusting messes in the class/function implementations
 * @{
 */

/** \def stlsoft_throw_0
 * \brief Indicates that the given function/method does not throw any exceptions.
 */
/** \def stlsoft_throw_1
 * \brief Indicates that the given function/method throws the named type.
 */
/** \def stlsoft_throw_2
 * \brief Indicates that the given function/method throws the two named types.
 */
/** \def stlsoft_throw_3
 * \brief Indicates that the given function/method throws the three named types.
 */
/** \def stlsoft_throw_4
 * \brief Indicates that the given function/method throws the four named types.
 */
/** \def stlsoft_throw_5
 * \brief Indicates that the given function/method throws the five named types.
 */
/** \def stlsoft_throw_6
 * \brief Indicates that the given function/method throws the six named types.
 */
/** \def stlsoft_throw_7
 * \brief Indicates that the given function/method throws the seven named types.
 */
/** \def stlsoft_throw_8
 * \brief Indicates that the given function/method throws the eight named types.
 */

/* Exception signatures. */
#if !defined(STLSOFT_NO_USE_EXCEPTION_SPECIFICATIONS) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) && \
    (   defined(STLSOFT_USE_EXCEPTION_SPECIFICATIONS) || \
        defined(STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT))
# ifndef STLSOFT_USE_EXCEPTION_SPECIFICATIONS
#  define STLSOFT_USE_EXCEPTION_SPECIFICATIONS
# endif /* !STLSOFT_USE_EXCEPTION_SPECIFICATIONS */
# define stlsoft_throw_0()                                  throw ()
# define stlsoft_throw_1(x1)                                throw (x1)
# define stlsoft_throw_2(x1, x2)                            throw (x1, x2)
# define stlsoft_throw_3(x1, x2, x3)                        throw (x1, x2, x3)
# define stlsoft_throw_4(x1, x2, x3, x4)                    throw (x1, x2, x3, x4)
# define stlsoft_throw_5(x1, x2, x3, x4, x5)                throw (x1, x2, x3, x4, x5)
# define stlsoft_throw_6(x1, x2, x3, x4, x5, x6)            throw (x1, x2, x3, x4, x5, x6)
# define stlsoft_throw_7(x1, x2, x3, x4, x5, x6, x7)        throw (x1, x2, x3, x4, x5, x6, x7)
# define stlsoft_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)    throw (x1, x2, x3, x4, x5, x6, x7, x8)
#else /* ? exception specifications */
# ifdef STLSOFT_USE_EXCEPTION_SPECIFICATIONS
#  undef STLSOFT_USE_EXCEPTION_SPECIFICATIONS
# endif /* STLSOFT_USE_EXCEPTION_SPECIFICATIONS */
# define stlsoft_throw_0()
# define stlsoft_throw_1(x1)
# define stlsoft_throw_2(x1, x2)
# define stlsoft_throw_3(x1, x2, x3)
# define stlsoft_throw_4(x1, x2, x3, x4)
# define stlsoft_throw_5(x1, x2, x3, x4, x5)
# define stlsoft_throw_6(x1, x2, x3, x4, x5, x6)
# define stlsoft_throw_7(x1, x2, x3, x4, x5, x6, x7)
# define stlsoft_throw_8(x1, x2, x3, x4, x5, x6, x7, x8)
#endif /* STLSOFT_CF_EXCEPTION_SIGNATURE_SUPPORT && !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Array size determination macro(s)
 */

/** \def STLSOFT_NUM_ELEMENTS(ar)
 *
 * \brief Evaluates, at compile time, to the number of elements within the given vector entity
 *
 * \param ar The array
 *
 * Is it used as follows:
 *
\code
int               ai[20];
int               i     = 32;
int               *pi   = &i;
std::vector<int>  vi;
size_t            s_ai  = STLSOFT_NUM_ELEMENTS(ai);   // Ok
size_t            s_i   = STLSOFT_NUM_ELEMENTS(i);    // Error
size_t            s_pi  = STLSOFT_NUM_ELEMENTS(pi);   // Error
size_t            s_vi  = STLSOFT_NUM_ELEMENTS(vi);   // Error
\endcode
 *
 * \note For most of the supported compilers, this macro will reject application to pointer
 * types, or to class types providing <code>operator []</code>. This helps to avoid the common
 * gotcha whereby <code>(sizeof(ar) / sizeof(ar[0]))</code> is applied to such types, without
 * causing a compiler error.
 *
 * \note From STLSoft 1.8.3 onwards, the underlying ss_static_array_size function is changed to return reference to const
 * ss_array_size_struct, rather than ss_array_size_struct, so as to avoid Visual C++ (7.1)'s C4686 warning
 */
#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_NUM_ELEMENTS_(ar)                      (sizeof(ar) / sizeof(0[(ar)]))

# if defined(__cplusplus) && \
     defined(STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT)
#  if 0/* defined(STLSOFT_COMPILER_IS_GCC) */
#   pragma pack(push, 1)
template <int N>
struct ss_array_size_struct
{
    ss_byte_t c[N];
};
#   pragma pack(pop)

template <class T, int N>
ss_array_size_struct<N> const& ss_static_array_size(T (&)[N]);

#   define STLSOFT_NUM_ELEMENTS(ar)                     sizeof(stlsoft_ns_qual(ss_static_array_size)(ar))
#  else /* ? 0 */
template <int N>
struct ss_array_size_struct
{
    ss_byte_t c[N];
};

template <class T, int N>
ss_array_size_struct<N> const& ss_static_array_size(T (&)[N]);

#if defined(STLSOFT_COMPILER_IS_VECTORC)
template <class T, int N>
ss_array_size_struct<N> const& ss_static_array_size(T const (&)[N]);
#endif /* compiler */


#   define STLSOFT_NUM_ELEMENTS(ar)                     sizeof(stlsoft_ns_qual(ss_static_array_size)(ar).c)
#  endif /* 0 */
# else /* ? __cplusplus && STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */
#  define STLSOFT_NUM_ELEMENTS(ar)                      STLSOFT_NUM_ELEMENTS_(ar)
# endif /* __cplusplus && STLSOFT_CF_STATIC_ARRAY_SIZE_DETERMINATION_SUPPORT */
#else /* ? STLSOFT_DOCUMENTATION_SKIP_SECTION */
# define STLSOFT_NUM_ELEMENTS(ar)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \def stlsoft_num_elements(ar)
 *
 * \brief Evaluates, at compile time, to the number of elements within the given vector entity
 *
 * \param ar The array
 *
 * \deprecated This is deprecated in favour of \ref STLSOFT_NUM_ELEMENTS().
 *
 * \note This is a simple \#define for STLSOFT_NUM_ELEMENTS()
 */
#define stlsoft_num_elements(ar)                        STLSOFT_NUM_ELEMENTS(ar)

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define stlsoft_num_elements_(ar)                      STLSOFT_NUM_ELEMENTS_(ar)
#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Structure element access macro(s)
 */

#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
# define STLSOFT_RAW_OFFSETOF_1(S, M)         \
                                              \
   stlsoft_static_cast(                       \
                                              \
    stlsoft_ns_qual(ss_size_t),               \
                                              \
     stlsoft_reinterpret_cast(                \
      stlsoft_ns_qual(ss_ptrdiff_t),          \
      stlsoft_reinterpret_cast(char const*,   \
       &                                      \
        (   stlsoft_reinterpret_cast(S*, stlsoft_static_cast(char*, 1)) /* Cast 0 to S */ \
        + 1)   /* Add 1 */                    \
          ->M) /* Take M addr */              \
      )                                       \
      -                                       \
     stlsoft_reinterpret_cast(                \
      stlsoft_ns_qual(ss_ptrdiff_t),          \
      stlsoft_reinterpret_cast(char const*,   \
        ( stlsoft_reinterpret_cast(S*, stlsoft_static_cast(char*, 1)) /* Cast 0 to S */   \
          + 1)) /* Add 1 */                   \
      )                                       \
    )

# define STLSOFT_RAW_OFFSETOF_2(S, M)                                           \
                                                                                \
    (reinterpret_cast<stlsoft_ns_qual(ss_size_t)>(&reinterpret_cast<S*>(1)->M) - 1)

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */


/** \def STLSOFT_RAW_OFFSETOF(S, M)
 *
 * \brief Evaluates, at compile time, the offset of the member \c m in the structure \c s
 *
 * \param S The type of the structure/class
 * \param M The name of the member
 */
#if defined(RECLS_COMPILER_IS_GCC)
# define STLSOFT_RAW_OFFSETOF(S, M)                     STLSOFT_RAW_OFFSETOF_2(S, M)
#elif defined(_STLSOFT_NO_STD_INCLUDES)
# define STLSOFT_RAW_OFFSETOF(S, M)                     offsetof(S, M)
#else /* ? _STLSOFT_NO_STD_INCLUDES */
# define STLSOFT_RAW_OFFSETOF(S, M)                     stlsoft_reinterpret_cast(stlsoft_ns_qual(ss_size_t), &stlsoft_static_cast(S*, 0)->M)
#endif /* !_STLSOFT_NO_STD_INCLUDES */

/** \def stlsoft_raw_offsetof(s, m)
 *
 * \brief Evaluates, at compile time, the offset of a structure/class member
 *
 * \deprecated This is deprecated in favour of \ref STLSOFT_RAW_OFFSETOF().
 *
 * \note This is a simple \#define for STLSOFT_RAW_OFFSETOF()
 */
#define stlsoft_raw_offsetof(s, m)                      STLSOFT_RAW_OFFSETOF(s, m)


#ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

# ifdef STLSOFT_CF_USE_RAW_OFFSETOF_IN_STATIC_ASSERT
#  undef STLSOFT_CF_USE_RAW_OFFSETOF_IN_STATIC_ASSERT
# endif /* STLSOFT_CF_USE_RAW_OFFSETOF_IN_STATIC_ASSERT */

# if !defined(STLSOFT_COMPILER_IS_COMO) && \
     !defined(STLSOFT_COMPILER_IS_GCC) && \
     !defined(STLSOFT_COMPILER_IS_INTEL) && \
     !defined(STLSOFT_COMPILER_IS_WATCOM)
#  define STLSOFT_CF_USE_RAW_OFFSETOF_IN_STATIC_ASSERT
# endif /* compiler */

#endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Destruction function(s)/macro(s)
 */

/* destroy function */
#if defined(__cplusplus)

# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_typename_param_k T>
inline
void
stlsoft_destroy_instance_fn(
    T *p
)
{
    p->~T();

    /* SSCB: Borland C++ and Visual C++ remove the dtor for basic
     * structs, and then warn that p is unused. This reference
     * suppresses that warning.
     */
#if defined(STLSOFT_COMPILER_IS_WATCOM)
    p = p;
#else /* ? compiler */
    static_cast<void>(p);
#endif /* compiler */
}
# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \def STLSOFT_DESTROY_INSTANCE(T1, T2, P)
 *
 * \param T1 The type to be destroyed, as a template parameter (e.g. T)
 * \param T2 The type to be destroyed, as a typedef (e.g. value_type)
 * \param P Pointer (T*) to the instance to be explicitly destroyed
 *
 * Destroys the given instance \c P of the given type (\c T1 and \c T2)
 */
# if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) || \
     defined(STLSOFT_COMPILER_IS_DMC)
#  define STLSOFT_DESTROY_INSTANCE(T1, T2, P)           do { (P)->~T1(); } while(0)
# else /* ? compiler */
#  define STLSOFT_DESTROY_INSTANCE(T1, T2, P)           stlsoft_ns_qual(stlsoft_destroy_instance_fn)((P))
# endif /* compiler */
#endif /* __cplusplus */

/** \def stlsoft_destroy_instance(T1, T2, P)
 *
 * \brief Explicitly destroys an instance
 *
 * \deprecated This is deprecated in favour of \ref STLSOFT_DESTROY_INSTANCE().
 *
 * \note This is a simple \#define for STLSOFT_DESTROY_INSTANCE()
 */
#define stlsoft_destroy_instance(T1, T2, P)             STLSOFT_DESTROY_INSTANCE(T1, T2, P)

/* /////////////////////////////////////////////////////////////////////////
 * Handle generator macro(s)
 */

/** \brief Generates an opaque type with the name \c type
 *
 * For example, the following defines two distinct opaque types:
 *
\code
STLSOFT_GEN_OPAQUE(HThread)
STLSOFT_GEN_OPAQUE(HProcess)
\endcode
 *
 * The two types are incompatible with each other, and with any other types (except that
 * they are both convertible to <code>void const*</code>
 */
#define STLSOFT_GEN_OPAQUE(type)                        typedef struct __stlsoft_htype##type{ int i;} const* type;

/** \def stlsoft_gen_opaque(Type)
 *
 * \brief Generates an opaque type with the name \c Type
 *
 * \deprecated This is deprecated in favour of \ref STLSOFT_GEN_OPAQUE().
 *
 * \note This is a simple \#define for STLSOFT_GEN_OPAQUE()
 */
#define stlsoft_gen_opaque(Type)                        STLSOFT_GEN_OPAQUE(Type)

/* /////////////////////////////////////////////////////////////////////////
 * Template friend macro(s)
 */

/** \def STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND
 * \ingroup group__project__stlsoft__code_modification_macros
 *
 * \brief Declares a template (class) parameter to be a friend of the template.
 *
 * Is it used as follows:
 *
\code
template<typename T>
class Thing
{
  STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(T);
private:
  int m_member; // Thing<T>::m_member visible to T
};
\endcode
 *
 * \note This is contrary to the C++-98 standard. Section 7.1.5.3(2) notes: <i>"...within a class
 * template with a template type-parameter T, the declaration ["]friend class T;["] is ill-formed."</i>
 * However, it gives the expected behaviour for all compilers currently supported by STLSoft
 */
#if defined(STLSOFT_DOCUMENTATION_SKIP_SECTION) || \
    defined(STLSOFT_COMPILER_IS_BORLAND) || \
    defined(STLSOFT_COMPILER_IS_COMO) || \
    defined(STLSOFT_COMPILER_IS_DMC) || \
    (   defined(STLSOFT_COMPILER_IS_GCC) && \
        __GNUC__ < 3) || \
    defined(STLSOFT_COMPILER_IS_INTEL) || \
    defined(STLSOFT_COMPILER_IS_MSVC) || \
    defined(STLSOFT_COMPILER_IS_VECTORC) || \
    defined(STLSOFT_COMPILER_IS_SUNPRO) || \
    defined(STLSOFT_COMPILER_IS_WATCOM)
# define    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     friend T
#elif defined(STLSOFT_COMPILER_IS_MWERKS)
# define    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)     friend class T
#elif defined(STLSOFT_COMPILER_IS_GCC) && \
      __GNUC__ >= 3

# if __GNUC__ >= 4 || \
     (  __GNUC__ == 3 && \
        __GNUC_MINOR__ >= 3 && \
        !defined(__APPLE__))

  /* This version of the technique for GCC 3.3+ was suggested by Lars Rune Nøstdal,
   * from Norway. Many thanks, Lars!
   */

#  define    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)    \
                                                            \
    struct friend_maker                                     \
    {                                                       \
        typedef T T2;                                       \
    };                                                      \
                                                            \
    friend class friend_maker::T2

# else /* ? __GNUC_MINOR__ */

#  define    STLSOFT_DECLARE_TEMPLATE_PARAM_AS_FRIEND(T)    \
                                                            \
    struct friend_maker                                     \
    {                                                       \
        typedef T T2;                                       \
    };                                                      \
                                                            \
    typedef typename friend_maker::T2 friend_type;          \
                                                            \
    friend friend_type

# endif /* __GNUC_MINOR__ */

#else /* ? compiler */
# error Compiler not discriminated
#endif /* compiler */


/* /////////////////////////////////////////////////////////////////////////
 * Code suppression macro(s)
 */

/** \def STLSOFT_SUPPRESS_UNUSED
 * \ingroup group__project__stlsoft__code_modification_macros
 *
 * \brief Used to suppress unused variable warnings
 */
#if defined(__cplusplus) && \
    (   defined(STLSOFT_COMPILER_IS_COMO) || \
        /* defined(STLSOFT_COMPILER_IS_DMCx) || */ \
        (   defined(STLSOFT_COMPILER_IS_GCC) && \
            __GNUC__ >= 3) || \
        defined(STLSOFT_COMPILER_IS_INTEL) || \
        defined(STLSOFT_COMPILER_IS_WATCOM))
template<ss_typename_param_k T>
inline void suppress_unused_func(T const volatile &)
{}

# if defined(STLSOFT_CF_CDECL_SUPPORTED)
template<ss_typename_param_k R>
inline void suppress_unused_func(R (STLSOFT_CDECL *)())
{}
template<ss_typename_param_k R, ss_typename_param_k A0>
inline void suppress_unused_func(R (STLSOFT_CDECL *)(A0))
{}
# endif /* STLSOFT_CF_CDECL_SUPPORTED */
# if defined(STLSOFT_CF_FASTCALL_SUPPORTED)
template<ss_typename_param_k R>
inline void suppress_unused_func(R (STLSOFT_FASTCALL *)())
{}
template<ss_typename_param_k R, ss_typename_param_k A0>
inline void suppress_unused_func(R (STLSOFT_FASTCALL *)(A0))
{}
# endif /* STLSOFT_CF_FASTCALL_SUPPORTED */
# if defined(STLSOFT_CF_STDCALL_SUPPORTED)
template<ss_typename_param_k R>
inline void suppress_unused_func(R (STLSOFT_STDCALL *)())
{}
template<ss_typename_param_k R, ss_typename_param_k A0>
inline void suppress_unused_func(R (STLSOFT_STDCALL *)(A0))
{}
# endif /* STLSOFT_CF_STDCALL_SUPPORTED */

# define STLSOFT_SUPPRESS_UNUSED(x)     stlsoft_ns_qual(suppress_unused_func)(x)
#else /* ? compiler */
# ifdef __cplusplus
#  define STLSOFT_SUPPRESS_UNUSED(x)    (static_cast<void>(x))
# else /* ? __cplusplus */
#  define STLSOFT_SUPPRESS_UNUSED(x)    ((void)(x))
# endif /* __cplusplus */
#endif /* compiler */

/** \def STLSOFT_UNNAMED_PARAM
 * \ingroup group__project__stlsoft__code_modification_macros
 *
 * \brief Used to define an unused parameter for C compilation and/or documentation processing, but not for C++ compilation
 */
#if defined(__cplusplus) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define STLSOFT_UNNAMED_PARAM(p)
#else /* ? __cplusplus */
# define STLSOFT_UNNAMED_PARAM(p)           p
#endif /* __cplusplus */

/** \def STLSOFT_SUPPRESS_UNNAMED_PARAM
 * \ingroup group__project__stlsoft__code_modification_macros
 *
 * \brief Used to suppress unused parameter warnings (in C compilation) for parameters defined by STLSOFT_UNNAMED_PARAM()
 */
#if defined(__cplusplus) && \
    !defined(STLSOFT_DOCUMENTATION_SKIP_SECTION)
# define STLSOFT_SUPPRESS_UNNAMED_PARAM(p)
#else /* ? __cplusplus */
# define STLSOFT_SUPPRESS_UNNAMED_PARAM(p)  ((void)p);
#endif /* __cplusplus */

/** @} */

/* /////////////////////////////////////////////////////////////////////////
 * Cast macro(s)
 */

/** \defgroup cast_macros Cast Macros
 * \ingroup group__project__stlsoft
 * \brief These macros abstract casts, representing the C++ casts in C++ compilation
 * units, and C-casts in C compilation units
 * @{
 */

/** \def stlsoft_c_cast(T, E)
 * \brief C-cast macro; equivalent to ((T)(E)), but more searchable
 */
#define stlsoft_c_cast(T, E)                ((T)(E))

/** \def stlsoft_static_cast(T, E)
 * \brief C-cast macro; equivalent to <code>static_cast<T, E></code> in C++, and stlsoft_c_cast(T, E) in C
 */

/** \def stlsoft_const_cast(T, E)
 * \brief C-cast macro; equivalent to <code>const_cast<T, E></code> in C++, and stlsoft_c_cast(T, E) in C
 */

/** \def stlsoft_volatile_cast(T, E)
 * \brief C-cast macro; equivalent to <code>const_cast<T, E></code> in C++, and stlsoft_c_cast(T, E) in C
 */

/** \def stlsoft_reinterpret_cast(T, E)
 * \brief C-cast macro; equivalent to <code>reinterpret_cast<T, E></code> in C++, and stlsoft_c_cast(T, E) in C
 */

/** \def stlsoft_dynamic_cast(T, E)
 * \brief C-cast macro; equivalent to <code>dynamic_cast<T, E></code> in C++, and stlsoft_c_cast(T, E) in C
 */

#if defined(__cplusplus)
# define stlsoft_static_cast(T, E)          static_cast< T>(E)
#else /* ? __cplusplus */
# define stlsoft_static_cast(T, E)          stlsoft_c_cast(T, E)
#endif /* __cplusplus */

#if defined(__cplusplus)
# define stlsoft_const_cast(T, E)           const_cast< T>(E)
#else /* ? __cplusplus */
# define stlsoft_const_cast(T, E)           stlsoft_c_cast(T, E)
#endif /* __cplusplus */

#if defined(__cplusplus)
# define stlsoft_volatile_cast(T, E)        const_cast< T>(E)
#else /* ? __cplusplus */
# define stlsoft_volatile_cast(T, E)        stlsoft_c_cast(T, E)
#endif /* __cplusplus */

#if defined(__cplusplus)
# define stlsoft_reinterpret_cast(T, E)     reinterpret_cast< T>(E)
#else /* ? __cplusplus */
# define stlsoft_reinterpret_cast(T, E)     stlsoft_c_cast(T, E)
#endif /* __cplusplus */

#if defined(__cplusplus)
# define stlsoft_dynamic_cast(T, E)         dynamic_cast< T>(E)
#else /* ? __cplusplus */
# define stlsoft_dynamic_cast(T, E)         stlsoft_c_cast(T, E)
#endif /* __cplusplus */

/** @} */

/* /////////////////////////////////////////////////////////////////////////
 * Inlining macro(s)
 */

/** \defgroup inlining_macros Inlining Macros
 * \ingroup group__project__stlsoft
 * \brief These macros abstract inline
 * @{
 */

/** \def STLSOFT_INLINE
 * \brief inline pseudo-keyword
 *
 * Resolves to \c inline in C++ compilation. In C, resolves to \c inline for
 * compilers that support C99, or otherwise \c STLSOFT_CUSTOM_C_INLINE if
 * defined, or otherwise defaults to \c static
 */
#if defined(__cplusplus)
# define STLSOFT_INLINE                     inline
#else /* ? __cplusplus */
# if defined(STLSOFT_CF_C99_INLINE)
#  define STLSOFT_INLINE                    inline
# elif defined(STLSOFT_CUSTOM_C_INLINE)
#  define STLSOFT_INLINE                    STLSOFT_CUSTOM_C_INLINE
# else /* ? C inline */
#  define STLSOFT_INLINE                    static
# endif /* C inline */
#endif /* __cplusplus */

/** @} */

/* /////////////////////////////////////////////////////////////////////////
 * Sanity checks - 4
 *
 * This section checks that the discriminated sizes of various types are
 * correct.
 */

#ifdef STLSOFT_CF_STATIC_ASSERT_SUPPORT
# ifdef __cplusplus
namespace internal_gcc
{
# endif /* __cplusplus */

# if defined(__cplusplus) /* || defined(STLSOFT_CF_C99_INLINE) */

STLSOFT_INLINE void _stlsoft_internal_verify_integral_type_sizes(void)
{
    STLSOFT_STATIC_ASSERT(_STLSOFT_SIZEOF_CHAR == sizeof(char));
    STLSOFT_STATIC_ASSERT(_STLSOFT_SIZEOF_SHORT == sizeof(short));
    STLSOFT_STATIC_ASSERT(_STLSOFT_SIZEOF_INT == sizeof(int));
    STLSOFT_STATIC_ASSERT(_STLSOFT_SIZEOF_LONG == sizeof(long));
#  ifdef STLSOFT_CF_BUILTIN_long_long_SUPPORT
    STLSOFT_STATIC_ASSERT(_STLSOFT_SIZEOF_LONG_LONG == sizeof(long long));
#  endif /* STLSOFT_CF_BUILTIN_long_long_SUPPORT */

    STLSOFT_STATIC_ASSERT(1 == sizeof(ss_sint8_t));
    STLSOFT_STATIC_ASSERT(1 == sizeof(ss_uint8_t));
    STLSOFT_STATIC_ASSERT(2 == sizeof(ss_sint16_t));
    STLSOFT_STATIC_ASSERT(2 == sizeof(ss_uint16_t));
    STLSOFT_STATIC_ASSERT(4 == sizeof(ss_sint32_t));
    STLSOFT_STATIC_ASSERT(4 == sizeof(ss_uint32_t));
#  ifdef STLSOFT_CF_64BIT_INT_SUPPORT
    STLSOFT_STATIC_ASSERT(8 == sizeof(ss_sint64_t));
    STLSOFT_STATIC_ASSERT(8 == sizeof(ss_uint64_t));
#  endif /* STLSOFT_CF_64BIT_INT_SUPPORT */
}

# endif /* __cplusplus) || STLSOFT_CF_C99_INLINE */

# ifdef __cplusplus
} /* namespace internal_gcc */
# endif /* __cplusplus */
#endif /* STLSOFT_CF_STATIC_ASSERT_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Pointer manipulation functions(s)
 */

/** \defgroup pointer_manipulation_functions Pointer Manipulation Functions
 * \ingroup group__project__stlsoft
 * \brief These functions assist in calculations with, and the manipulation of, pointers
 * @{
 */

/** \brief Offsets a pointer by a number of bytes
 *
 * \param p The pointer to be offset
 * \param n The number of bytes to offset
 * \result \c p offset by \c bytes, expressed as <code>void const*</code>
 */
#if defined(__cplusplus)

template <ss_typename_param_k T>
inline
void
const*
ptr_byte_offset(
    T const         p
,   ss_ptrdiff_t    n
)
{
# if 0
# if !defined(STLSOFT_COMPILER_IS_BORLAND) && \
     (   !defined(STLSOFT_COMPILER_IS_MSVC) || \
         _MSC_VER > 1200)
    struct InternalPointerChecker
    {
    public:
        static void check(void mutable *)
        {}
        static void check(void const mutable *)
        {}
        template <ss_typename_param_k T2>
        static void check(T2 p)
        {
            ss_size_t n = sizeof(p[0]);

            STLSOFT_SUPPRESS_UNUSED(n);
        }
    };

    /* Check it's a pointer */
    InternalPointerChecker::check(p);
# endif /* compiler */
# endif /* 0 */

    void const*         p1 =   static_cast<void const*>(p);
    ss_byte_t const*    p2 =   static_cast<ss_byte_t const*>(p1);
    ss_byte_t const*    p3 =   p2 + n;
    void const*         p4 =   static_cast<void const*>(p3);
    void const*         p5 =   static_cast<void const*>(p);

    STLSOFT_ASSERT((static_cast<char const*>(p4) - static_cast<char const*>(p5)) == n);

    STLSOFT_SUPPRESS_UNUSED(p5);    /* VC++ 7.1 can't see this. ?? */

    return p4;
}

/** \brief Offsets a pointer by a number of elements
 *
 * \param p The pointer to be offset
 * \param n The number of elements to offset
 * \result \c p offset by \c elements
 */
template <ss_typename_param_k T>
inline
T const*
ptr_offset(
    T const*        p
,   ss_ptrdiff_t    n
)
{
    return p + n;
}

/** \brief Get the difference in bytes between two pointers
 */
template<
    ss_typename_param_k T1
,   ss_typename_param_k T2
>
inline
ss_ptrdiff_t
ptr_byte_diff(
    T1 const*   p1
,   T2 const*   p2
)
{
    return static_cast<ss_byte_t const*>(static_cast<void const*>(p1)) - static_cast<ss_byte_t const*>(static_cast<void const*>(p2));
}

/** \brief Get the difference in elements between two pointers
 */
template<
    ss_typename_param_k T1
,   ss_typename_param_k T2
>
inline
ss_ptrdiff_t
ptr_diff(
    T1 const*   p1
,   T2 const*   p2
)
{
    return p1 - p2;
}

#endif /* __cplusplus */

/** @} end of group pointer_manipulation_functions */

/* /////////////////////////////////////////////////////////////////////////
 * cv qualifier functions(s)
 */

/** \defgroup cv_qualifier_functions cv-Qualifier Functions
 * \ingroup group__project__stlsoft
 * \brief These functions assist in the manipulation of cv-qualifiers
 * @{
 */

#if defined(__cplusplus)

/** \brief Remove const-qualifier from an instance.
 *
 * \warning Using this function can result in undefined behaviour. As such,
 * the advice is: <b>Use With Care!</b>
 */
template <ss_typename_param_k T>
inline
T&
remove_const(T const& t)
{
    return const_cast<T&>(t);
}

/** \brief Remove const-qualifier from an instance.
 *
 * \warning Using this function can result in undefined behaviour. As such,
 * the advice is: <b>Use With Care!</b>
 */
template <ss_typename_param_k T>
inline
T*
remove_const_ptr(T const* t)
{
    return const_cast<T*>(t);
}

/** \brief Adds const-qualifier to an instance.
 */
template <ss_typename_param_k T>
inline
T const&
apply_const(T& t)
{
    return t;
}

/** \brief Adds const-qualifier to an instance.
 */
template <ss_typename_param_k T>
inline
T const*
apply_const_ptr(T* t)
{
    return t;
}


/* Mutable support */
# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION
template <ss_typename_param_k T>
#  ifdef STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT
inline T& mutable_access(T& t)
#  else /* ? STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT */
inline T& mutable_access(T const& t)
#  endif /* STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT */
{
#  ifdef STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT
    return t;
#  else /* ? STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT */
    return const_cast<T&>(t);
#  endif /* STLSOFT_CF_MUTABLE_KEYWORD_SUPPORT */
}
# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/* Move constructor support */
# ifndef STLSOFT_DOCUMENTATION_SKIP_SECTION

#  ifdef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
#   define stlsoft_define_move_rhs_type(t)           t &
#  else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
#   define stlsoft_define_move_rhs_type(t)           t const&
#  endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */

template <ss_typename_param_k T>
inline T& move_lhs_from_rhs(stlsoft_define_move_rhs_type(T) t)
{
#  ifdef STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT
    return t;
#  else /* ? STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
    return const_cast<T&>(t);
#  endif /* STLSOFT_CF_MOVE_CONSTRUCTOR_SUPPORT */
}

# endif /* !STLSOFT_DOCUMENTATION_SKIP_SECTION */

/** \brief Takes the address of a variable
 *
 * \warning This function is used to take the address of a variable,
 * possibly a temporary, and allows the code to skip the reasonable
 * reservations of compilers to not want to compile code that uses the
 * address of temporaries. As such, the advice is: <b>Use With Care!</b>
 */
template <ss_typename_param_k T>
inline T const* address(T const& t)
{
    return &t;
}

#endif /* __cplusplus */

/** @} */

/* /////////////////////////////////////////////////////////////////////////
 * Memory
 */

#if defined(__cplusplus)

/* function operator new
 *
 * When namespaces are being used, stlsoft provides its own placement new,
 * otherwise it includes <new> in order to access the global version.
 */

#ifndef new
# ifdef _STLSOFT_NO_NAMESPACE
#  if defined(STLSOFT_COMPILER_IS_BORLAND) && \
      __BORLANDC__ < 0x0550
#   include <new.h>
#  else /* ? compiler */
#   include <new>
#  endif /* compiler */
# else /* ? _STLSOFT_NO_NAMESPACE */
#  if ( defined(STLSOFT_COMPILER_IS_DMC) && \
        __DMC__ < 0x0833) || \
      ( defined(STLSOFT_COMPILER_IS_MSVC) && \
        _MSC_VER < 1300)
inline
void*
operator new(
    ss_size_t   /* si */
,   void*       pv
)
{
    return pv;
}
#  endif /* compiler */
# endif /* !_STLSOFT_NO_NAMESPACE */
#endif /* !new */

#endif /* __cplusplus */

/* ////////////////////////////////////////////////////////////////////// */

#ifndef _STLSOFT_NO_NAMESPACE
} /* namespace stlsoft */
#endif /* !_STLSOFT_NO_NAMESPACE */

/* /////////////////////////////////////////////////////////////////////////
 * Inclusion
 */

#ifdef STLSOFT_CF_PRAGMA_ONCE_SUPPORT
# pragma once
#endif /* STLSOFT_CF_PRAGMA_ONCE_SUPPORT */

/* /////////////////////////////////////////////////////////////////////////
 * Unit-testing
 */

#ifdef STLSOFT_UNITTEST

# ifndef STLSOFT_INCL_UNITTEST_HPP_UNITTEST
#  include <unittest/unittest.hpp>
# endif /* !STLSOFT_INCL_UNITTEST_HPP_UNITTEST */

# if defined(STLSOFT_COMPILER_IS_MSVC) && \
     _MSC_VER == 1200
#  include <stlsoft/synch/concepts.hpp> /* Avoids a VC6 ICE. */
# endif /* compiler */

#endif /* STLSOFT_UNITTEST */

/* ////////////////////////////////////////////////////////////////////// */

#endif /* !STLSOFT_INCL_STLSOFT_H_STLSOFT */

/* ///////////////////////////// end of file //////////////////////////// */
