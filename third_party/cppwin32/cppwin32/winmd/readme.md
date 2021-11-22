This is a fork of https://github.com/microsoft/winmd adding some needed features. Things that weren't needed in winmd for winrt types:
- Reading pointers from signatures (ELEMENT_TYPE_PTR)
- Reading ELEMENT_TYPE_VOID from signatures
- Nested types and the NestedClass table

So far, these changes are purely additive and won't harm other winmd consumers in any way, and will just improve the completeness of the the winmd library.

As long as this holds, we should aim to integrate these additions back into winmd.

One thing to consider is that the support for arrays and pointers in signatures is a little underwhelming, and some aspects of the signature blob design are awkward and wordy. There's a case to be made to revise and improve this part of the winmd library, but we'll have to see if its few consumers can tolerate or welcome such a change.
