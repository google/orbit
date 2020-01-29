//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

<<<<<<< HEAD
=======
#define INITGUID
>>>>>>> e2026243958763cdad022f2d7f6e830eec149208
#include <Guiddef.h>
#include <evntrace.h>

DEFINE_GUID(/* 45d8cccd-539f-4b72-a8b7-5c683142609a */
            ALPCGuid, 0x45d8cccd, 0x539f, 0x4b72, 0xa8, 0xb7, 0x5c, 0x68, 0x31,
            0x42, 0x60, 0x9a);

DEFINE_GUID(/* 3d6fa8d4-fe05-11d0-9dda-00c04fd7ba7c */
            DiskIoGuid, 0x3d6fa8d4, 0xfe05, 0x11d0, 0x9d, 0xda, 0x00, 0xc0,
            0x4f, 0xd7, 0xba, 0x7c);

DEFINE_GUID(/* 90cbdc39-4a3e-11d1-84f4-0000f80464e3 */
            FileIoGuid, 0x90cbdc39, 0x4a3e, 0x11d1, 0x84, 0xf4, 0x00, 0x00,
            0xf8, 0x04, 0x64, 0xe3);

DEFINE_GUID(/* 2cb15d1d-5fc1-11d2-abe1-00a0c911f518 */
            ImageLoadGuid, 0x2cb15d1d, 0x5fc1, 0x11d2, 0xab, 0xe1, 0x00, 0xa0,
            0xc9, 0x11, 0xf5, 0x18);

DEFINE_GUID(/* 3d6fa8d3-fe05-11d0-9dda-00c04fd7ba7c */
            PageFaultGuid, 0x3d6fa8d3, 0xfe05, 0x11d0, 0x9d, 0xda, 0x00, 0xc0,
            0x4f, 0xd7, 0xba, 0x7c);

DEFINE_GUID(/* ce1dbfb4-137e-4da6-87b0-3f59aa102cbc */
            PerfInfoGuid, 0xce1dbfb4, 0x137e, 0x4da6, 0x87, 0xb0, 0x3f, 0x59,
            0xaa, 0x10, 0x2c, 0xbc);

DEFINE_GUID(/* 3d6fa8d0-fe05-11d0-9dda-00c04fd7ba7c */
            ProcessGuid, 0x3d6fa8d0, 0xfe05, 0x11d0, 0x9d, 0xda, 0x00, 0xc0,
            0x4f, 0xd7, 0xba, 0x7c);

DEFINE_GUID(/* AE53722E-C863-11d2-8659-00C04FA321A1 */
            RegistryGuid, 0xae53722e, 0xc863, 0x11d2, 0x86, 0x59, 0x0, 0xc0,
            0x4f, 0xa3, 0x21, 0xa1);

DEFINE_GUID(/* d837ca92-12b9-44a5-ad6a-3a65b3578aa8 */
            SplitIoGuid, 0xd837ca92, 0x12b9, 0x44a5, 0xad, 0x6a, 0x3a, 0x65,
            0xb3, 0x57, 0x8a, 0xa8);

DEFINE_GUID(/* 9a280ac0-c8e0-11d1-84e2-00c04fb998a2 */
            TcpIpGuid, 0x9a280ac0, 0xc8e0, 0x11d1, 0x84, 0xe2, 0x00, 0xc0, 0x4f,
            0xb9, 0x98, 0xa2);

DEFINE_GUID(/* 3d6fa8d1-fe05-11d0-9dda-00c04fd7ba7c */
            ThreadGuid, 0x3d6fa8d1, 0xfe05, 0x11d0, 0x9d, 0xda, 0x00, 0xc0,
            0x4f, 0xd7, 0xba, 0x7c);

DEFINE_GUID(/* bf3a50c5-a9c9-4988-a005-2df0b7c80f80 */
            UdpIpGuid, 0xbf3a50c5, 0xa9c9, 0x4988, 0xa0, 0x05, 0x2d, 0xf0, 0xb7,
            0xc8, 0x0f, 0x80);

DEFINE_GUID(/* def2fe46-7bd6-4b80-bd 94-f5 7f e2 0d 0c e3 */
            StackWalkGuid, 0xdef2fe46, 0x7bd6, 0x4b80, 0xbd, 0x94, 0xf5, 0x7f,
            0xe2, 0x0d, 0x0c, 0xe3);

//-------------------------------------------------------------------------
#define EVENT_GUID_STRING(x) \
  if (a_GUID == x) return #x

//-----------------------------------------------------------------------------
struct EventGuid {
  //-------------------------------------------------------------------------
  static inline char* GetName(const GUID& a_GUID) {
    EVENT_GUID_STRING(ALPCGuid);
    EVENT_GUID_STRING(DiskIoGuid);
    EVENT_GUID_STRING(EventTraceConfigGuid);
    EVENT_GUID_STRING(FileIoGuid);
    EVENT_GUID_STRING(ImageLoadGuid);
    EVENT_GUID_STRING(PageFaultGuid);
    EVENT_GUID_STRING(PerfInfoGuid);
    EVENT_GUID_STRING(ProcessGuid);
    EVENT_GUID_STRING(RegistryGuid);
    EVENT_GUID_STRING(SplitIoGuid);
    EVENT_GUID_STRING(TcpIpGuid);
    EVENT_GUID_STRING(ThreadGuid);
    EVENT_GUID_STRING(UdpIpGuid);
    return "UnknownGUID";
  }

  //-------------------------------------------------------------------------
  static inline ULONG64 Hash(const GUID& a_Guid) {
    return XXH64(&a_Guid, sizeof(GUID), 0x1d1d57ac);
  }
};