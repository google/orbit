// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SyntaxHighlighter/X86Assembly.h"

#include <QColor>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QString>
#include <QTextBlock>
#include <QTextBlockUserData>
#include <QTextCharFormat>

#include "SyntaxHighlighter/HighlightingMetadata.h"

namespace orbit_syntax_highlighter {

namespace {
const QColor kCommentColor{0x99, 0x99, 0x99};
const QColor kPlatformColor{0x99, 0x99, 0x99};
const QColor kProgramCounterColor{0x61, 0x96, 0xcc};
const QColor kOpcodeColor{0xf8, 0xc5, 0x55};
const QColor kNumberColor{0xf0, 0x8d, 0x49};
const QColor kRegisterColor{0x7e, 0xc6, 0x99};
const QColor kKeywordColor{0xcc, 0x99, 0xcd};
const QColor kCallTargetColor{0x80, 0x80, 0x00};

namespace AssemblyRegex {
const QRegularExpression kCharacterRegex{"(\\S)"};
const QRegularExpression kNumberRegex{"\\s((0x)?[\\da-f]+)\\b"};
const QRegularExpression kProgramCounterRegex{"^(0x[0-9a-f]+:)"};
const QRegularExpression kOpCodeRegex{
    "\\b(aaa|aad|aam|aas|adc|add|and|arpl|bb0_reset|bb1_reset|bound|bsf|bsr|bswap|bt|btc|btr|"
    "bts|call|cbw|cdq|cdqe|clc|cld|cli|clts|cmc|cmp|cmpsb|cmpsd|cmpsq|cmpsw|cmpxchg|cmpxchg486|"
    "cmpxchg8b|cmpxchg16b|cpuid|cpu_read|cpu_write|cqo|cwd|cwde|daa|das|dec|div|dmint|emms|enter|"
    "equ|f2xm1|fabs|fadd|faddp|fbld|fbstp|fchs|fclex|fcmovb|fcmovbe|fcmove|fcmovnb|fcmovnbe|"
    "fcmovne|fcmovnu|fcmovu|fcom|fcomi|fcomip|fcomp|fcompp|fcos|fdecstp|fdisi|fdiv|fdivp|fdivr|"
    "fdivrp|femms|feni|ffree|ffreep|fiadd|ficom|ficomp|fidiv|fidivr|fild|fimul|fincstp|finit|"
    "fist|fistp|fisttp|fisub|fisubr|fld|fld1|fldcw|fldenv|fldl2e|fldl2t|fldlg2|fldln2|fldpi|fldz|"
    "fmul|fmulp|fnclex|fndisi|fneni|fninit|fnop|fnsave|fnstcw|fnstenv|fnstsw|fpatan|fprem|fprem1|"
    "fptan|frndint|frstor|fsave|fscale|fsetpm|fsin|fsincos|fsqrt|fst|fstcw|fstenv|fstp|fstsw|"
    "fsub|fsubp|fsubr|fsubrp|ftst|fucom|fucomi|fucomip|fucomp|fucompp|fxam|fxch|fxtract|fyl2x|"
    "fyl2xp1|hlt|ibts|icebp|idiv|imul|in|inc|incbin|insb|insd|insw|int|int01|int1|int03|int3|"
    "into|invd|invpcid|invlpg|invlpga|iret|iretd|iretq|iretw|jcxz|jecxz|jrcxz|jmp|jmpe|lahf|lar|"
    "lds|lea|leave|les|lfence|lfs|lgdt|lgs|lidt|lldt|lmsw|loadall|loadall286|lodsb|lodsd|lodsq|"
    "lodsw|loop|loope|loopne|loopnz|loopz|lsl|lss|ltr|mfence|monitor|mov|movd|movq|movsb|movsd|"
    "movsq|movsw|movsx|movsxd|movzx|mul|mwait|neg|nop|not|or|out|outsb|outsd|outsw|packssdw|"
    "packsswb|packuswb|paddb|paddd|paddsb|paddsiw|paddsw|paddusb|paddusw|paddw|pand|pandn|pause|"
    "paveb|pavgusb|pcmpeqb|pcmpeqd|pcmpeqw|pcmpgtb|pcmpgtd|pcmpgtw|pdistib|pf2id|pfacc|pfadd|"
    "pfcmpeq|pfcmpge|pfcmpgt|pfmax|pfmin|pfmul|pfrcp|pfrcpit1|pfrcpit2|pfrsqit1|pfrsqrt|pfsub|"
    "pfsubr|pi2fd|pmachriw|pmaddwd|pmagw|pmulhriw|pmulhrwa|pmulhrwc|pmulhw|pmullw|pmvgezb|pmvlzb|"
    "pmvnzb|pmvzb|pop|popa|popad|popaw|popf|popfd|popfq|popfw|por|prefetch|prefetchw|pslld|psllq|"
    "psllw|psrad|psraw|psrld|psrlq|psrlw|psubb|psubd|psubsb|psubsiw|psubsw|psubusb|psubusw|psubw|"
    "punpckhbw|punpckhdq|punpckhwd|punpcklbw|punpckldq|punpcklwd|push|pusha|pushad|pushaw|pushf|"
    "pushfd|pushfq|pushfw|pxor|rcl|rcr|rdshr|rdmsr|rdpmc|rdtsc|rdtscp|ret|retf|retn|rol|ror|rdm|"
    "rsdc|rsldt|rsm|rsts|sahf|sal|salc|sar|sbb|scasb|scasd|scasq|scasw|sfence|sgdt|shl|shld|shr|"
    "shrd|sidt|sldt|skinit|smi|smint|smintold|smsw|stc|std|sti|stosb|stosd|stosq|stosw|str|sub|"
    "svdc|svldt|svts|swapgs|syscall|sysenter|sysexit|sysret|test|ud0|ud1|ud2b|ud2|ud2a|umov|verr|"
    "verw|fwait|wbinvd|wrshr|wrmsr|xadd|xbts|xchg|xlatb|xlat|xor|cmove|cmovz|cmovne|cmovnz|cmova|"
    "cmovnbe|cmovae|cmovnb|cmovb|cmovnae|cmovbe|cmovna|cmovg|cmovnle|cmovge|cmovnl|cmovl|cmovnge|"
    "cmovle|cmovng|cmovc|cmovnc|cmovo|cmovno|cmovs|cmovns|cmovp|cmovpe|cmovnp|cmovpo|je|jz|jne|"
    "jnz|ja|jnbe|jae|jnb|jb|jnae|jbe|jna|jg|jnle|jge|jnl|jl|jnge|jle|jng|jc|jnc|jo|jno|js|jns|"
    "jpo|jnp|jpe|jp|sete|setz|setne|setnz|seta|setnbe|setae|setnb|setnc|setb|setnae|setcset|"
    "setbe|setna|setg|setnle|setge|setnl|setl|setnge|setle|setng|sets|setns|seto|setno|setpe|"
    "setp|setpo|setnp|addps|addss|andnps|andps|cmpeqps|cmpeqss|cmpleps|cmpless|cmpltps|cmpltss|"
    "cmpneqps|cmpneqss|cmpnleps|cmpnless|cmpnltps|cmpnltss|cmpordps|cmpordss|cmpunordps|"
    "cmpunordss|cmpps|cmpss|comiss|cvtpi2ps|cvtps2pi|cvtsi2ss|cvtss2si|cvttps2pi|cvttss2si|divps|"
    "divss|ldmxcsr|maxps|maxss|minps|minss|mova.s|movhps|movlhps|movlps|movhlps|movmskps|movntps|"
    "movss|movups|mulps|mulss|orps|rcpps|rcpss|rsqrtps|rsqrtss|shufps|sqrtps|sqrtss|stmxcsr|"
    "subps|subss|ucomiss|unpckhps|unpcklps|xorps|fxrstor|fxrstor64|fxsave|fxsave64|xgetbv|xsetbv|"
    "xsave|xsave64|xsaveopt|xsaveopt64|xrstor|xrstor64|prefetchnta|prefetcht0|prefetcht1|"
    "prefetcht2|maskmovq|movntq|pavgb|pavgw|pextrw|pinsrw|pmaxsw|pmaxub|pminsw|pminub|pmovmskb|"
    "pmulhuw|psadbw|pshufw|pf2iw|pfnacc|pfpnacc|pi2fw|pswapd|maskmovdqu|clflush|movntdq|movnti|"
    "movntpd|movdqa|movdqu|movdq2q|movq2dq|paddq|pmuludq|pshufd|pshufhw|pshuflw|pslldq|psrldq|"
    "psubq|punpckhqdq|punpcklqdq|addpd|addsd|andnpd|andpd|cmpeqpd|cmpeqsd|cmplepd|cmplesd|"
    "cmpltpd|cmpltsd|cmpneqpd|cmpneqsd|cmpnlepd|cmpnlesd|cmpnltpd|cmpnltsd|cmpordpd|cmpordsd|"
    "cmpunordpd|cmpunordsd|cmppd|comisd|cvtdq2pd|cvtdq2ps|cvtpd2dq|cvtpd2pi|cvtpd2ps|cvtpi2pd|"
    "cvtps2dq|cvtps2pd|cvtsd2si|cvtsd2ss|cvtsi2sd|cvtss2sd|cvttpd2pi|cvttpd2dq|cvttps2dq|"
    "cvttsd2si|divpd|divsd|maxpd|maxsd|minpd|minsd|movapd|movhpd|movlpd|movmskpd|movupd|mulpd|"
    "mulsd|orpd|shufpd|sqrtpd|sqrtsd|subpd|subsd|ucomisd|unpckhpd|unpcklpd|xorpd|addsubpd|"
    "addsubps|haddpd|haddps|hsubpd|hsubps|lddqu|movddup|movshdup|movsldup|clgi|stgi|vmcall|"
    "vmclear|vmfunc|vmlaunch|vmload|vmmcall|vmptrld|vmptrst|vmread|vmresume|vmrun|vmsave|vmwrite|"
    "vmxoff|vmxon|invept|invvpid|pabsb|pabsw|pabsd|palignr|phaddw|phaddd|phaddsw|phsubw|phsubd|"
    "phsubsw|pmaddubsw|pmulhrsw|pshufb|psignb|psignw|psignd|extrq|insertq|movntsd|movntss|lzcnt|"
    "blendpd|blendps|blendvpd|blendvps|dppd|dpps|extractps|insertps|movntdqa|mpsadbw|packusdw|"
    "pblendvb|pblendw|pcmpeqq|pextrb|pextrd|pextrq|phminposuw|pinsrb|pinsrd|pinsrq|pmaxsb|pmaxsd|"
    "pmaxud|pmaxuw|pminsb|pminsd|pminud|pminuw|pmovsxbw|pmovsxbd|pmovsxbq|pmovsxwd|pmovsxwq|"
    "pmovsxdq|pmovzxbw|pmovzxbd|pmovzxbq|pmovzxwd|pmovzxwq|pmovzxdq|pmuldq|pmulld|ptest|roundpd|"
    "roundps|roundsd|roundss|crc32|pcmpestri|pcmpestrm|pcmpistri|pcmpistrm|pcmpgtq|popcnt|getsec|"
    "pfrcpv|pfrsqrtv|movbe|aesenc|aesenclast|aesdec|aesdeclast|aesimc|aeskeygenassist|vaesenc|"
    "vaesenclast|vaesdec|vaesdeclast|vaesimc|vaeskeygenassist|vaddpd|vaddps|vaddsd|vaddss|"
    "vaddsubpd|vaddsubps|vandpd|vandps|vandnpd|vandnps|vblendpd|vblendps|vblendvpd|vblendvps|"
    "vbroadcastss|vbroadcastsd|vbroadcastf128|vcmpeq_ospd|vcmpeqpd|vcmplt_ospd|vcmpltpd|vcmple_"
    "ospd|vcmplepd|vcmpunord_qpd|vcmpunordpd|vcmpneq_uqpd|vcmpneqpd|vcmpnlt_uspd|vcmpnltpd|"
    "vcmpnle_uspd|vcmpnlepd|vcmpord_qpd|vcmpordpd|vcmpeq_uqpd|vcmpnge_uspd|vcmpngepd|vcmpngt_"
    "uspd|vcmpngtpd|vcmpfalse_oqpd|vcmpfalsepd|vcmpneq_oqpd|vcmpge_ospd|vcmpgepd|vcmpgt_ospd|"
    "vcmpgtpd|vcmptrue_uqpd|vcmptruepd|vcmplt_oqpd|vcmple_oqpd|vcmpunord_spd|vcmpneq_uspd|"
    "vcmpnlt_uqpd|vcmpnle_uqpd|vcmpord_spd|vcmpeq_uspd|vcmpnge_uqpd|vcmpngt_uqpd|vcmpfalse_ospd|"
    "vcmpneq_ospd|vcmpge_oqpd|vcmpgt_oqpd|vcmptrue_uspd|vcmppd|vcmpeq_osps|vcmpeqps|vcmplt_osps|"
    "vcmpltps|vcmple_osps|vcmpleps|vcmpunord_qps|vcmpunordps|vcmpneq_uqps|vcmpneqps|vcmpnlt_usps|"
    "vcmpnltps|vcmpnle_usps|vcmpnleps|vcmpord_qps|vcmpordps|vcmpeq_uqps|vcmpnge_usps|vcmpngeps|"
    "vcmpngt_usps|vcmpngtps|vcmpfalse_oqps|vcmpfalseps|vcmpneq_oqps|vcmpge_osps|vcmpgeps|vcmpgt_"
    "osps|vcmpgtps|vcmptrue_uqps|vcmptrueps|vcmplt_oqps|vcmple_oqps|vcmpunord_sps|vcmpneq_usps|"
    "vcmpnlt_uqps|vcmpnle_uqps|vcmpord_sps|vcmpeq_usps|vcmpnge_uqps|vcmpngt_uqps|vcmpfalse_osps|"
    "vcmpneq_osps|vcmpge_oqps|vcmpgt_oqps|vcmptrue_usps|vcmpps|vcmpeq_ossd|vcmpeqsd|vcmplt_ossd|"
    "vcmpltsd|vcmple_ossd|vcmplesd|vcmpunord_qsd|vcmpunordsd|vcmpneq_uqsd|vcmpneqsd|vcmpnlt_ussd|"
    "vcmpnltsd|vcmpnle_ussd|vcmpnlesd|vcmpord_qsd|vcmpordsd|vcmpeq_uqsd|vcmpnge_ussd|vcmpngesd|"
    "vcmpngt_ussd|vcmpngtsd|vcmpfalse_oqsd|vcmpfalsesd|vcmpneq_oqsd|vcmpge_ossd|vcmpgesd|vcmpgt_"
    "ossd|vcmpgtsd|vcmptrue_uqsd|vcmptruesd|vcmplt_oqsd|vcmple_oqsd|vcmpunord_ssd|vcmpneq_ussd|"
    "vcmpnlt_uqsd|vcmpnle_uqsd|vcmpord_ssd|vcmpeq_ussd|vcmpnge_uqsd|vcmpngt_uqsd|vcmpfalse_ossd|"
    "vcmpneq_ossd|vcmpge_oqsd|vcmpgt_oqsd|vcmptrue_ussd|vcmpsd|vcmpeq_osss|vcmpeqss|vcmplt_osss|"
    "vcmpltss|vcmple_osss|vcmpless|vcmpunord_qss|vcmpunordss|vcmpneq_uqss|vcmpneqss|vcmpnlt_usss|"
    "vcmpnltss|vcmpnle_usss|vcmpnless|vcmpord_qss|vcmpordss|vcmpeq_uqss|vcmpnge_usss|vcmpngess|"
    "vcmpngt_usss|vcmpngtss|vcmpfalse_oqss|vcmpfalsess|vcmpneq_oqss|vcmpge_osss|vcmpgess|vcmpgt_"
    "osss|vcmpgtss|vcmptrue_uqss|vcmptruess|vcmplt_oqss|vcmple_oqss|vcmpunord_sss|vcmpneq_usss|"
    "vcmpnlt_uqss|vcmpnle_uqss|vcmpord_sss|vcmpeq_usss|vcmpnge_uqss|vcmpngt_uqss|vcmpfalse_osss|"
    "vcmpneq_osss|vcmpge_oqss|vcmpgt_oqss|vcmptrue_usss|vcmpss|vcomisd|vcomiss|vcvtdq2pd|"
    "vcvtdq2ps|vcvtpd2dq|vcvtpd2ps|vcvtps2dq|vcvtps2pd|vcvtsd2si|vcvtsd2ss|vcvtsi2sd|vcvtsi2ss|"
    "vcvtss2sd|vcvtss2si|vcvttpd2dq|vcvttps2dq|vcvttsd2si|vcvttss2si|vdivpd|vdivps|vdivsd|vdivss|"
    "vdppd|vdpps|vextractf128|vextractps|vhaddpd|vhaddps|vhsubpd|vhsubps|vinsertf128|vinsertps|"
    "vlddqu|vldqqu|vldmxcsr|vmaskmovdqu|vmaskmovps|vmaskmovpd|vmaxpd|vmaxps|vmaxsd|vmaxss|vminpd|"
    "vminps|vminsd|vminss|vmovapd|vmovaps|vmovd|vmovq|vmovddup|vmovdqa|vmovqqa|vmovdqu|vmovqqu|"
    "vmovhlps|vmovhpd|vmovhps|vmovlhps|vmovlpd|vmovlps|vmovmskpd|vmovmskps|vmovntdq|vmovntqq|"
    "vmovntdqa|vmovntpd|vmovntps|vmovsd|vmovshdup|vmovsldup|vmovss|vmovupd|vmovups|vmpsadbw|"
    "vmulpd|vmulps|vmulsd|vmulss|vorpd|vorps|vpabsb|vpabsw|vpabsd|vpacksswb|vpackssdw|vpackuswb|"
    "vpackusdw|vpaddb|vpaddw|vpaddd|vpaddq|vpaddsb|vpaddsw|vpaddusb|vpaddusw|vpalignr|vpand|"
    "vpandn|vpavgb|vpavgw|vpblendvb|vpblendw|vpcmpestri|vpcmpestrm|vpcmpistri|vpcmpistrm|"
    "vpcmpeqb|vpcmpeqw|vpcmpeqd|vpcmpeqq|vpcmpgtb|vpcmpgtw|vpcmpgtd|vpcmpgtq|vpermilpd|vpermilps|"
    "vperm2f128|vpextrb|vpextrw|vpextrd|vpextrq|vphaddw|vphaddd|vphaddsw|vphminposuw|vphsubw|"
    "vphsubd|vphsubsw|vpinsrb|vpinsrw|vpinsrd|vpinsrq|vpmaddwd|vpmaddubsw|vpmaxsb|vpmaxsw|"
    "vpmaxsd|vpmaxub|vpmaxuw|vpmaxud|vpminsb|vpminsw|vpminsd|vpminub|vpminuw|vpminud|vpmovmskb|"
    "vpmovsxbw|vpmovsxbd|vpmovsxbq|vpmovsxwd|vpmovsxwq|vpmovsxdq|vpmovzxbw|vpmovzxbd|vpmovzxbq|"
    "vpmovzxwd|vpmovzxwq|vpmovzxdq|vpmulhuw|vpmulhrsw|vpmulhw|vpmullw|vpmulld|vpmuludq|vpmuldq|"
    "vpor|vpsadbw|vpshufb|vpshufd|vpshufhw|vpshuflw|vpsignb|vpsignw|vpsignd|vpslldq|vpsrldq|"
    "vpsllw|vpslld|vpsllq|vpsraw|vpsrad|vpsrlw|vpsrld|vpsrlq|vptest|vpsubb|vpsubw|vpsubd|vpsubq|"
    "vpsubsb|vpsubsw|vpsubusb|vpsubusw|vpunpckhbw|vpunpckhwd|vpunpckhdq|vpunpckhqdq|vpunpcklbw|"
    "vpunpcklwd|vpunpckldq|vpunpcklqdq|vpxor|vrcpps|vrcpss|vrsqrtps|vrsqrtss|vroundpd|vroundps|"
    "vroundsd|vroundss|vshufpd|vshufps|vsqrtpd|vsqrtps|vsqrtsd|vsqrtss|vstmxcsr|vsubpd|vsubps|"
    "vsubsd|vsubss|vtestps|vtestpd|vucomisd|vucomiss|vunpckhpd|vunpckhps|vunpcklpd|vunpcklps|"
    "vxorpd|vxorps|vzeroall|vzeroupper|pclmullqlqdq|pclmulhqlqdq|pclmullqhqdq|pclmulhqhqdq|"
    "pclmulqdq|vpclmullqlqdq|vpclmulhqlqdq|vpclmullqhqdq|vpclmulhqhqdq|vpclmulqdq|vfmadd132ps|"
    "vfmadd132pd|vfmadd312ps|vfmadd312pd|vfmadd213ps|vfmadd213pd|vfmadd123ps|vfmadd123pd|"
    "vfmadd231ps|vfmadd231pd|vfmadd321ps|vfmadd321pd|vfmaddsub132ps|vfmaddsub132pd|"
    "vfmaddsub312ps|vfmaddsub312pd|vfmaddsub213ps|vfmaddsub213pd|vfmaddsub123ps|vfmaddsub123pd|"
    "vfmaddsub231ps|vfmaddsub231pd|vfmaddsub321ps|vfmaddsub321pd|vfmsub132ps|vfmsub132pd|"
    "vfmsub312ps|vfmsub312pd|vfmsub213ps|vfmsub213pd|vfmsub123ps|vfmsub123pd|vfmsub231ps|"
    "vfmsub231pd|vfmsub321ps|vfmsub321pd|vfmsubadd132ps|vfmsubadd132pd|vfmsubadd312ps|"
    "vfmsubadd312pd|vfmsubadd213ps|vfmsubadd213pd|vfmsubadd123ps|vfmsubadd123pd|vfmsubadd231ps|"
    "vfmsubadd231pd|vfmsubadd321ps|vfmsubadd321pd|vfnmadd132ps|vfnmadd132pd|vfnmadd312ps|"
    "vfnmadd312pd|vfnmadd213ps|vfnmadd213pd|vfnmadd123ps|vfnmadd123pd|vfnmadd231ps|vfnmadd231pd|"
    "vfnmadd321ps|vfnmadd321pd|vfnmsub132ps|vfnmsub132pd|vfnmsub312ps|vfnmsub312pd|vfnmsub213ps|"
    "vfnmsub213pd|vfnmsub123ps|vfnmsub123pd|vfnmsub231ps|vfnmsub231pd|vfnmsub321ps|vfnmsub321pd|"
    "vfmadd132ss|vfmadd132sd|vfmadd312ss|vfmadd312sd|vfmadd213ss|vfmadd213sd|vfmadd123ss|"
    "vfmadd123sd|vfmadd231ss|vfmadd231sd|vfmadd321ss|vfmadd321sd|vfmsub132ss|vfmsub132sd|"
    "vfmsub312ss|vfmsub312sd|vfmsub213ss|vfmsub213sd|vfmsub123ss|vfmsub123sd|vfmsub231ss|"
    "vfmsub231sd|vfmsub321ss|vfmsub321sd|vfnmadd132ss|vfnmadd132sd|vfnmadd312ss|vfnmadd312sd|"
    "vfnmadd213ss|vfnmadd213sd|vfnmadd123ss|vfnmadd123sd|vfnmadd231ss|vfnmadd231sd|vfnmadd321ss|"
    "vfnmadd321sd|vfnmsub132ss|vfnmsub132sd|vfnmsub312ss|vfnmsub312sd|vfnmsub213ss|vfnmsub213sd|"
    "vfnmsub123ss|vfnmsub123sd|vfnmsub231ss|vfnmsub231sd|vfnmsub321ss|vfnmsub321sd|rdfsbase|"
    "rdgsbase|rdrand|wrfsbase|wrgsbase|vcvtph2ps|vcvtps2ph|adcx|adox|rdseed|clac|stac|xstore|"
    "xcryptecb|xcryptcbc|xcryptctr|xcryptcfb|xcryptofb|montmul|xsha1|xsha256|llwpcb|slwpcb|"
    "lwpval|lwpins|vfmaddpd|vfmaddps|vfmaddsd|vfmaddss|vfmaddsubpd|vfmaddsubps|vfmsubaddpd|"
    "vfmsubaddps|vfmsubpd|vfmsubps|vfmsubsd|vfmsubss|vfnmaddpd|vfnmaddps|vfnmaddsd|vfnmaddss|"
    "vfnmsubpd|vfnmsubps|vfnmsubsd|vfnmsubss|vfrczpd|vfrczps|vfrczsd|vfrczss|vpcmov|vpcomb|"
    "vpcomd|vpcomq|vpcomub|vpcomud|vpcomuq|vpcomuw|vpcomw|vphaddbd|vphaddbq|vphaddbw|vphadddq|"
    "vphaddubd|vphaddubq|vphaddubw|vphaddudq|vphadduwd|vphadduwq|vphaddwd|vphaddwq|vphsubbw|"
    "vphsubdq|vphsubwd|vpmacsdd|vpmacsdqh|vpmacsdql|vpmacssdd|vpmacssdqh|vpmacssdql|vpmacsswd|"
    "vpmacssww|vpmacswd|vpmacsww|vpmadcsswd|vpmadcswd|vpperm|vprotb|vprotd|vprotq|vprotw|vpshab|"
    "vpshad|vpshaq|vpshaw|vpshlb|vpshld|vpshlq|vpshlw|vbroadcasti128|vpblendd|vpbroadcastb|"
    "vpbroadcastw|vpbroadcastd|vpbroadcastq|vpermd|vpermpd|vpermps|vpermq|vperm2i128|"
    "vextracti128|vinserti128|vpmaskmovd|vpmaskmovq|vpsllvd|vpsllvq|vpsravd|vpsrlvd|vpsrlvq|"
    "vgatherdpd|vgatherqpd|vgatherdps|vgatherqps|vpgatherdd|vpgatherqd|vpgatherdq|vpgatherqq|"
    "xabort|xbegin|xend|xtest|andn|bextr|blci|blcic|blsi|blsic|blcfill|blsfill|blcmsk|blsmsk|"
    "blsr|blcs|bzhi|mulx|pdep|pext|rorx|sarx|shlx|shrx|tzcnt|tzmsk|t1mskc|valignd|valignq|"
    "vblendmpd|vblendmps|vbroadcastf32x4|vbroadcastf64x4|vbroadcasti32x4|vbroadcasti64x4|"
    "vcompresspd|vcompressps|vcvtpd2udq|vcvtps2udq|vcvtsd2usi|vcvtss2usi|vcvttpd2udq|vcvttps2udq|"
    "vcvttsd2usi|vcvttss2usi|vcvtudq2pd|vcvtudq2ps|vcvtusi2sd|vcvtusi2ss|vexpandpd|vexpandps|"
    "vextractf32x4|vextractf64x4|vextracti32x4|vextracti64x4|vfixupimmpd|vfixupimmps|vfixupimmsd|"
    "vfixupimmss|vgetexppd|vgetexpps|vgetexpsd|vgetexpss|vgetmantpd|vgetmantps|vgetmantsd|"
    "vgetmantss|vinsertf32x4|vinsertf64x4|vinserti32x4|vinserti64x4|vmovdqa32|vmovdqa64|"
    "vmovdqu32|vmovdqu64|vpabsq|vpandd|vpandnd|vpandnq|vpandq|vpblendmd|vpblendmq|vpcmpltd|"
    "vpcmpled|vpcmpneqd|vpcmpnltd|vpcmpnled|vpcmpd|vpcmpltq|vpcmpleq|vpcmpneqq|vpcmpnltq|"
    "vpcmpnleq|vpcmpq|vpcmpequd|vpcmpltud|vpcmpleud|vpcmpnequd|vpcmpnltud|vpcmpnleud|vpcmpud|"
    "vpcmpequq|vpcmpltuq|vpcmpleuq|vpcmpnequq|vpcmpnltuq|vpcmpnleuq|vpcmpuq|vpcompressd|"
    "vpcompressq|vpermi2d|vpermi2pd|vpermi2ps|vpermi2q|vpermt2d|vpermt2pd|vpermt2ps|vpermt2q|"
    "vpexpandd|vpexpandq|vpmaxsq|vpmaxuq|vpminsq|vpminuq|vpmovdb|vpmovdw|vpmovqb|vpmovqd|vpmovqw|"
    "vpmovsdb|vpmovsdw|vpmovsqb|vpmovsqd|vpmovsqw|vpmovusdb|vpmovusdw|vpmovusqb|vpmovusqd|"
    "vpmovusqw|vpord|vporq|vprold|vprolq|vprolvd|vprolvq|vprord|vprorq|vprorvd|vprorvq|"
    "vpscatterdd|vpscatterdq|vpscatterqd|vpscatterqq|vpsraq|vpsravq|vpternlogd|vpternlogq|"
    "vptestmd|vptestmq|vptestnmd|vptestnmq|vpxord|vpxorq|vrcp14pd|vrcp14ps|vrcp14sd|vrcp14ss|"
    "vrndscalepd|vrndscaleps|vrndscalesd|vrndscaless|vrsqrt14pd|vrsqrt14ps|vrsqrt14sd|vrsqrt14ss|"
    "vscalefpd|vscalefps|vscalefsd|vscalefss|vscatterdpd|vscatterdps|vscatterqpd|vscatterqps|"
    "vshuff32x4|vshuff64x2|vshufi32x4|vshufi64x2|kandnw|kandw|kmovw|knotw|kortestw|korw|kshiftlw|"
    "kshiftrw|kunpckbw|kxnorw|kxorw|vpbroadcastmb2q|vpbroadcastmw2d|vpconflictd|vpconflictq|"
    "vplzcntd|vplzcntq|vexp2pd|vexp2ps|vrcp28pd|vrcp28ps|vrcp28sd|vrcp28ss|vrsqrt28pd|vrsqrt28ps|"
    "vrsqrt28sd|vrsqrt28ss|vgatherpf0dpd|vgatherpf0dps|vgatherpf0qpd|vgatherpf0qps|vgatherpf1dpd|"
    "vgatherpf1dps|vgatherpf1qpd|vgatherpf1qps|vscatterpf0dpd|vscatterpf0dps|vscatterpf0qpd|"
    "vscatterpf0qps|vscatterpf1dpd|vscatterpf1dps|vscatterpf1qpd|vscatterpf1qps|prefetchwt1|"
    "bndmk|bndcl|bndcu|bndcn|bndmov|bndldx|bndstx|sha1rnds4|sha1nexte|sha1msg1|sha1msg2|"
    "sha256rnds2|sha256msg1|sha256msg2|hint_nop0|hint_nop1|hint_nop2|hint_nop3|hint_nop4|hint_"
    "nop5|hint_nop6|hint_nop7|hint_nop8|hint_nop9|hint_nop10|hint_nop11|hint_nop12|hint_nop13|"
    "hint_nop14|hint_nop15|hint_nop16|hint_nop17|hint_nop18|hint_nop19|hint_nop20|hint_nop21|"
    "hint_nop22|hint_nop23|hint_nop24|hint_nop25|hint_nop26|hint_nop27|hint_nop28|hint_nop29|"
    "hint_nop30|hint_nop31|hint_nop32|hint_nop33|hint_nop34|hint_nop35|hint_nop36|hint_nop37|"
    "hint_nop38|hint_nop39|hint_nop40|hint_nop41|hint_nop42|hint_nop43|hint_nop44|hint_nop45|"
    "hint_nop46|hint_nop47|hint_nop48|hint_nop49|hint_nop50|hint_nop51|hint_nop52|hint_nop53|"
    "hint_nop54|hint_nop55|hint_nop56|hint_nop57|hint_nop58|hint_nop59|hint_nop60|hint_nop61|"
    "hint_nop62|hint_nop63)\\b"};
const QRegularExpression kRegisterRegex{
    "\\b(ip|eip|rip|[abcd][lh]|sil|dil|bpl|spl|r\\d+b|[abcd]x|si|di|bp|sp|r\\d+w|e[abcd]x|esi|"
    "edi|ebp|esp|eip|r\\d+d|r[abcd]x|rsi|rdi|rbp|rsp|r\\d+|[cdefgs]s|st\\d*|[xyz]?mm\\d+|k\\d|"
    "bnd\\d|[cd]?r\\d+[bwhl]?|d[bwdqtoyz]|ddq|res[bwdqtoyz]|resdq|incbin|equ|times|nosplit|rel|"
    "abs|seg|wrt|strict|near|far|a32)\\b"};
const QRegularExpression kKeywordRegex{"\\b(ptr|[xy]mmword|[sdq]?word|byte)\\b"};
const QRegularExpression kCommentRegex{"(;.*)$"};
const QRegularExpression kPlatformRegex{"^(Platform:.*)$"};
const QRegularExpression kCallTargetRegex{R"(\bcall[^\(]*\((.*)\)$)"};
}  // namespace AssemblyRegex
}  // namespace

X86Assembly::X86Assembly() : QSyntaxHighlighter{static_cast<QObject*>(nullptr)} {}

void X86Assembly::highlightBlock(const QString& code) {
  const HighlightingMetadata* const highlighting_metadata =
      dynamic_cast<const HighlightingMetadata*>(currentBlock().userData());
  if (highlighting_metadata == nullptr || highlighting_metadata->IsMainContentLine()) {
    HighlightBlockAssembly(code, [this](int start, int count, const QTextCharFormat& format) {
      setFormat(start, count, format);
    });
  } else {  // Metadata::LineType::kAnnotatingLine
    HighlightAnnotatingBlock(code, [this](int start, int count, const QTextCharFormat& format) {
      setFormat(start, count, format);
    });
  }
}

// Highlight every character with the same default_color
void HighlightAnnotatingBlock(
    const QString& code, const std::function<void(int, int, const QTextCharFormat&)>& set_format,
    const QColor& default_color) {
  const auto apply = [&code, &set_format](const QRegularExpression& expression,
                                          const QColor& color) {
    QTextCharFormat format{};
    format.setForeground(color);

    for (auto it = expression.globalMatch(code); it.hasNext();) {
      const auto match = it.next();
      // We use the first / outermost capture group, as this gives more flexibility for the match
      // without being highlighted. In particular this allows variable length matches before the
      // part of interest (in contrast to fixed length lookaheads).
      set_format(match.capturedStart(1), match.capturedLength(1), format);
    }
  };

  apply(AssemblyRegex::kCharacterRegex, default_color);
}

void HighlightBlockAssembly(
    const QString& code, const std::function<void(int, int, const QTextCharFormat&)>& set_format) {
  const auto apply = [&code, &set_format](const QRegularExpression& expression,
                                          const QColor& color) {
    QTextCharFormat format{};
    format.setForeground(color);

    for (auto it = expression.globalMatch(code); it.hasNext();) {
      const auto match = it.next();
      // We use the first / outermost capture group, as this gives more flexibility for the match
      // without being highlighted. In particular this allows variable length matches before the
      // part of interest (in contrast to fixed length lookaheads).
      set_format(match.capturedStart(1), match.capturedLength(1), format);
    }
  };

  apply(AssemblyRegex::kNumberRegex, kNumberColor);
  apply(AssemblyRegex::kProgramCounterRegex, kProgramCounterColor);
  apply(AssemblyRegex::kOpCodeRegex, kOpcodeColor);
  apply(AssemblyRegex::kRegisterRegex, kRegisterColor);
  apply(AssemblyRegex::kKeywordRegex, kKeywordColor);
  apply(AssemblyRegex::kCommentRegex, kCommentColor);
  apply(AssemblyRegex::kPlatformRegex, kPlatformColor);
  apply(AssemblyRegex::kCallTargetRegex, kCallTargetColor);
}
}  // namespace orbit_syntax_highlighter
