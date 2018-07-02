#pragma once

//-----------------------------------------------------------------------------
enum OrbitPrologOffset
{
    Prolog_OriginalFunction = 0,
    Prolog_CallbackAddress,
    Prolog_EpilogAddress,
    Prolog_OriginalAddress,
    Prolog_NumOffsets
};

//-----------------------------------------------------------------------------
struct Prolog
{
    byte*  m_Code;
    size_t m_Size;
    size_t m_Offsets[Prolog_NumOffsets];
};

//-----------------------------------------------------------------------------
enum OrbitEpilogOffset
{
    Epilog_CallbackAddress = 0,
    Epilog_NumOffsets
};

//-----------------------------------------------------------------------------
struct Epilog
{
    byte*  m_Code;
    size_t m_Size;
    size_t m_Offsets[Epilog_NumOffsets];
};

//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
const struct Prolog* GetOrbitProlog();
const struct Epilog* GetOrbitEpilog();
#ifdef __cplusplus
}
#endif