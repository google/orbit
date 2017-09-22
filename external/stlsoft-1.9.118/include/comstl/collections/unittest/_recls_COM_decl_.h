
// Created: 13th December 2005
// Updated: 10th August 2009

#ifndef COMSTL_INCL_COMSTL_UNITTEST_H__RECLS_COM_DECL_
#define COMSTL_INCL_COMSTL_UNITTEST_H__RECLS_COM_DECL_

#ifndef COMSTL_INCL_COMSTL_H_COMSTL
# include <comstl/comstl.h>
#endif /* !COMSTL_INCL_COMSTL_H_COMSTL */
#ifndef COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS
# include <comstl/util/interface_traits.hpp>
#endif /* !COMSTL_INCL_COMSTL_UTIL_HPP_INTERFACE_TRAITS */

/* ////////////////////////////////////////////////////////////////////// */

namespace recls_COM
{
    enum RECLS_FLAG_
        {   RECLS_F_FILES_  = 0x1,
        RECLS_F_DIRECTORIES_    = 0x2,
        RECLS_F_LINKS_  = 0x4,
        RECLS_F_DEVICES_    = 0x8,
        RECLS_F_TYPEMASK_   = 0xfff,
        RECLS_F_RECURSIVE_  = 0x10000,
        RECLS_F_NO_FOLLOW_LINKS_    = 0x20000,
        RECLS_F_DIRECTORY_PARTS_    = 0x40000,
        RECLS_F_DETAILS_LATER_  = 0x80000,
        RECLS_F_MARK_DIRS_  = 0x200000,
        RECLS_F_ALLOW_REPARSE_DIRS_ = 0x400000
        };


    const IID IID_IFileEntry_ = { 0x50F21F68, 0x1A94, 0x4c70, { 0xA2, 0x0F, 0x40, 0x8B, 0xB1, 0x18, 0x08, 0xD0 } };
    MIDL_INTERFACE("50F21F68-1A94-4c70-A20F-408BB11808D0")
    IFileEntry_ : public IDispatch
    {
    public:
        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Path(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Drive(
            /* [retval][out] */ OLECHAR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Directory(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DirectoryPath(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_DirectoryParts(
            /* [retval][out] */ LPUNKNOWN __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_File(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_ShortFile(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_FileName(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_FileExt(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_CreationTime(
            /* [retval][out] */ DATE __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_ModificationTime(
            /* [retval][out] */ DATE __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_LastAccessTime(
            /* [retval][out] */ DATE __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_LastStatusChangeTime(
            /* [retval][out] */ DATE __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Size(
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsReadOnly(
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_IsDirectory(
            /* [retval][out] */ BOOL __RPC_FAR *pVal) = 0;

    };

    const IID IID_IEnumFileEntry_ = { 0x29F36E3A, 0xC42E, 0x47c0, { 0xA9, 0xB9, 0x2F, 0x70, 0xB7, 0x67, 0x22, 0xED } };
    MIDL_INTERFACE("29F36E3A-C42E-47c0-A9B9-2F70B76722ED")
    IEnumFileEntry_ : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE Next(
            /* [in] */ ULONG celt,
            /* [length_is][size_is][out] */ IFileEntry_* __RPC_FAR *rgVar,
            /* [out] */ ULONG __RPC_FAR *pceltFetched) = 0;

        virtual HRESULT STDMETHODCALLTYPE Skip(
            /* [in] */ ULONG celt) = 0;

        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE Clone(
            /* [out] */ IEnumFileEntry_ __RPC_FAR *__RPC_FAR *ppenum) = 0;

    };

    const IID IID_ISearchCollection_ = { 0x2CCEE26C, 0xB94B, 0x4352, { 0xA2, 0x69, 0xA4, 0xEE, 0x84, 0x90, 0x83, 0x67 } };
    MIDL_INTERFACE("2CCEE26C-B94B-4352-A269-A4EE84908367")
    ISearchCollection_ : public IDispatch
    {
    public:
        virtual /* [hidden][restricted][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum(
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *pVal) = 0;

    };

    const IID IID_IDirectoryPartsCollection_ = { 0x7151ACC6, 0x3A28, 0x4BB0, { 0xBD, 0x48, 0xEF, 0xF7, 0xFD, 0x30, 0x3F, 0x6B } };
    MIDL_INTERFACE("7151ACC6-3A28-4BB0-BD48-EFF7FD303F6B")
    IDirectoryPartsCollection_ : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Count(
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Item(
            long index,
            /* [retval][out] */ VARIANT __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][hidden][restricted][id][propget] */ HRESULT STDMETHODCALLTYPE get__NewEnum(
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppenum) = 0;

    };

    const IID IID_IFileSearch_ = { 0x10115D3C, 0x84B8, 0x41BE, { 0x8F, 0x59, 0x46, 0x94, 0x11, 0x92, 0xDE, 0xBA } };
    MIDL_INTERFACE("10115D3C-84B8-41BE-8F59-46941192DEBA")
    IFileSearch_ : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Search(
            /* [in] */ BSTR searchRoot,
            /* [in] */ BSTR pattern,
            /* [in] */ long flags,
            /* [retval][out] */ IUnknown __RPC_FAR *__RPC_FAR *results) = 0;

        virtual /* [helpstring][propget][id] */ HRESULT STDMETHODCALLTYPE get_Roots(
            /* [retval][out] */ LPUNKNOWN __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PathSeparator(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PathNameSeparator(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_WildcardsAll(
            /* [retval][out] */ BSTR __RPC_FAR *pVal) = 0;

    };

    const CLSID CLSID_FileSearch_ = { 0xCB3A4FF2, 0xECCE, 0x4912, { 0xA3, 0xE0, 0x5F, 0x44, 0x67, 0xF4, 0x1D, 0xB7 } };

    inline void use_IIDs_()
    {
        STLSOFT_SUPPRESS_UNUSED(IID_IFileEntry_);
        STLSOFT_SUPPRESS_UNUSED(IID_IEnumFileEntry_);
        STLSOFT_SUPPRESS_UNUSED(IID_ISearchCollection_);
        STLSOFT_SUPPRESS_UNUSED(IID_IDirectoryPartsCollection_);
        STLSOFT_SUPPRESS_UNUSED(IID_IFileSearch_);
    }

} // recls_COM

namespace stlsoft
{
    namespace comstl_project
    {
        COMSTL_IID_TRAITS_DEFINE_NS(IFileEntry_, ::recls_COM)
        COMSTL_IID_TRAITS_DEFINE_NS(IEnumFileEntry_, ::recls_COM)
        COMSTL_IID_TRAITS_DEFINE_NS(ISearchCollection_, ::recls_COM)
        COMSTL_IID_TRAITS_DEFINE_NS(IDirectoryPartsCollection_, ::recls_COM)
        COMSTL_IID_TRAITS_DEFINE_NS(IFileSearch_, ::recls_COM)
    }
}

/* ////////////////////////////////////////////////////////////////////// */

#endif /* COMSTL_INCL_COMSTL_UNITTEST_H__RECLS_COM_DECL_ */

/* ///////////////////////////// end of file //////////////////////////// */
