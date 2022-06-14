// Copyright (C) 2019-2022 Intel Corporation
// SPDX-License-Identifier: MIT

#include "Debug.hpp"

#if DEBUG_VERBOSE

#include "PresentMonTraceConsumer.hpp"

#include "ETW/Microsoft_Windows_D3D9.h"
#include "ETW/Microsoft_Windows_Dwm_Core.h"
#include "ETW/Microsoft_Windows_DXGI.h"
#include "ETW/Microsoft_Windows_DxgKrnl.h"
#include "ETW/Microsoft_Windows_Win32k.h"

#include <assert.h>
#include <dxgi.h>
#include <initializer_list>

namespace {

PresentEvent const* gModifiedPresent = nullptr;
PresentEvent gOriginalPresentValues(EVENT_HEADER{}, Runtime::Other);

bool gDebugDone = false;
bool gDebugTrace = false;
LARGE_INTEGER* gFirstTimestamp = nullptr;
LARGE_INTEGER gTimestampFrequency = {};

uint64_t ConvertTimestampDeltaToNs(uint64_t timestampDelta)
{
    return 1000000000ull * timestampDelta / gTimestampFrequency.QuadPart;
}

uint64_t ConvertTimestampToNs(uint64_t timestamp)
{
    return ConvertTimestampDeltaToNs(timestamp - gFirstTimestamp->QuadPart);
}

char* AddCommas(uint64_t t)
{
    static char buf[128];
    auto r = sprintf_s(buf, "%llu", t);

    auto commaCount = r == 0 ? 0 : ((r - 1) / 3);
    for (int i = 0; i < commaCount; ++i) {
        auto p = r + commaCount - 4 * i;
        auto q = r - 3 * i;
        buf[p - 1] = buf[q - 1];
        buf[p - 2] = buf[q - 2];
        buf[p - 3] = buf[q - 3];
        buf[p - 4] = ',';
    }

    r += commaCount;
    buf[r] = '\0';
    return buf;
}

void PrintU32(uint32_t value) { printf("%u", value); }
void PrintU64(uint64_t value) { printf("%llu", value); }
void PrintU64x(uint64_t value) { printf("%llx", value); }
void PrintTime(uint64_t value) { printf("%s", value == 0 ? "0" : AddCommas(ConvertTimestampToNs(value))); }
void PrintTimeDelta(uint64_t value) { printf("%s", value == 0 ? "0" : AddCommas(ConvertTimestampDeltaToNs(value))); }
void PrintBool(bool value) { printf("%s", value ? "true" : "false"); }
void PrintRuntime(Runtime value)
{
    switch (value) {
    case Runtime::DXGI:  printf("DXGI");  break;
    case Runtime::D3D9:  printf("D3D9");  break;
    case Runtime::Other: printf("Other"); break;
    default:             printf("Unknown (%u)", value); assert(false); break;
    }
}
void PrintPresentMode(PresentMode value)
{
    switch (value) {
    case PresentMode::Unknown:                              printf("Unknown"); break;
    case PresentMode::Hardware_Legacy_Flip:                 printf("Hardware_Legacy_Flip"); break;
    case PresentMode::Hardware_Legacy_Copy_To_Front_Buffer: printf("Hardware_Legacy_Copy_To_Front_Buffer"); break;
    case PresentMode::Hardware_Independent_Flip:            printf("Hardware_Independent_Flip"); break;
    case PresentMode::Composed_Flip:                        printf("Composed_Flip"); break;
    case PresentMode::Composed_Copy_GPU_GDI:                printf("Composed_Copy_GPU_GDI"); break;
    case PresentMode::Composed_Copy_CPU_GDI:                printf("Composed_Copy_CPU_GDI"); break;
    case PresentMode::Hardware_Composed_Independent_Flip:   printf("Hardware_Composed_Independent_Flip"); break;
    default:                                                printf("Unknown (%u)", value); assert(false); break;
    }
}
void PrintPresentResult(PresentResult value)
{
    switch (value) {
    case PresentResult::Unknown:   printf("Unknown");   break;
    case PresentResult::Presented: printf("Presented"); break;
    case PresentResult::Discarded: printf("Discarded"); break;
    default:                       printf("Unknown (%u)", value); assert(false); break;
    }
}
void PrintPresentHistoryModel(uint32_t model)
{
    using namespace Microsoft_Windows_DxgKrnl;
    switch (model) {
    case PresentModel::D3DKMT_PM_UNINITIALIZED:          printf("UNINITIALIZED");          break;
    case PresentModel::D3DKMT_PM_REDIRECTED_GDI:         printf("REDIRECTED_GDI");         break;
    case PresentModel::D3DKMT_PM_REDIRECTED_FLIP:        printf("REDIRECTED_FLIP");        break;
    case PresentModel::D3DKMT_PM_REDIRECTED_BLT:         printf("REDIRECTED_BLT");         break;
    case PresentModel::D3DKMT_PM_REDIRECTED_VISTABLT:    printf("REDIRECTED_VISTABLT");    break;
    case PresentModel::D3DKMT_PM_SCREENCAPTUREFENCE:     printf("SCREENCAPTUREFENCE");     break;
    case PresentModel::D3DKMT_PM_REDIRECTED_GDI_SYSMEM:  printf("REDIRECTED_GDI_SYSMEM");  break;
    case PresentModel::D3DKMT_PM_REDIRECTED_COMPOSITION: printf("REDIRECTED_COMPOSITION"); break;
    case PresentModel::D3DKMT_PM_SURFACECOMPLETE:        printf("SURFACECOMPLETE");        break;
    case PresentModel::D3DKMT_PM_FLIPMANAGER:            printf("FLIPMANAGER");            break;
    default:                                             printf("Unknown (%u)", model); assert(false); break;
    }
}
void PrintTokenState(uint32_t state)
{
    using namespace Microsoft_Windows_Win32k;
    switch (state) {
    case TokenState::Completed: printf("Completed"); break;
    case TokenState::InFrame:   printf("InFrame");   break;
    case TokenState::Confirmed: printf("Confirmed"); break;
    case TokenState::Retired:   printf("Retired");   break;
    case TokenState::Discarded: printf("Discarded"); break;
    default:                    printf("Unknown (%u)", state); assert(false); break;
    }
}
void PrintQueuePacketType(uint32_t type)
{
    using namespace Microsoft_Windows_DxgKrnl;
    switch (type) {
    case QueuePacketType::DXGKETW_RENDER_COMMAND_BUFFER:   printf("RENDER"); break;
    case QueuePacketType::DXGKETW_DEFERRED_COMMAND_BUFFER: printf("DEFERRED"); break;
    case QueuePacketType::DXGKETW_SYSTEM_COMMAND_BUFFER:   printf("SYSTEM"); break;
    case QueuePacketType::DXGKETW_MMIOFLIP_COMMAND_BUFFER: printf("MMIOFLIP"); break;
    case QueuePacketType::DXGKETW_WAIT_COMMAND_BUFFER:     printf("WAIT"); break;
    case QueuePacketType::DXGKETW_SIGNAL_COMMAND_BUFFER:   printf("SIGNAL"); break;
    case QueuePacketType::DXGKETW_DEVICE_COMMAND_BUFFER:   printf("DEVICE"); break;
    case QueuePacketType::DXGKETW_SOFTWARE_COMMAND_BUFFER: printf("SOFTWARE"); break;
    case QueuePacketType::DXGKETW_PAGING_COMMAND_BUFFER:   printf("PAGING"); break;
    default:                                               printf("Unknown (%u)", type); assert(false); break;
    }
}
void PrintPresentFlags(uint32_t flags)
{
    if (flags & DXGI_PRESENT_TEST) printf("TEST");
}

void PrintEventHeader(EVENT_HEADER const& hdr)
{
    printf("%16s %5u %5u ", AddCommas(ConvertTimestampToNs(hdr.TimeStamp.QuadPart)), hdr.ProcessId, hdr.ThreadId);
}

void PrintEventHeader(EVENT_HEADER const& hdr, char const* name)
{
    PrintEventHeader(hdr);
    printf("%s\n", name);
}

void PrintEventHeader(EVENT_RECORD* eventRecord, EventMetadata* metadata, char const* name, std::initializer_list<void*> props)
{
    assert((props.size() % 2) == 0);

    PrintEventHeader(eventRecord->EventHeader);
    printf("%s", name);
    for (auto ii = props.begin(), ie = props.end(); ii != ie; ++ii) {
        auto propName = (wchar_t const*) *ii; ++ii;
        auto propFunc = *ii;

        printf(" %ls=", propName);

             if (propFunc == PrintU32)                  PrintU32(metadata->GetEventData<uint32_t>(eventRecord, propName));
        else if (propFunc == PrintU64)                  PrintU64(metadata->GetEventData<uint64_t>(eventRecord, propName));
        else if (propFunc == PrintU64x)                 PrintU64x(metadata->GetEventData<uint64_t>(eventRecord, propName));
        else if (propFunc == PrintTime)                 PrintTime(metadata->GetEventData<uint64_t>(eventRecord, propName));
        else if (propFunc == PrintTimeDelta)            PrintTimeDelta(metadata->GetEventData<uint64_t>(eventRecord, propName));
        else if (propFunc == PrintTokenState)           PrintTokenState(metadata->GetEventData<uint32_t>(eventRecord, propName));
        else if (propFunc == PrintQueuePacketType)      PrintQueuePacketType(metadata->GetEventData<uint32_t>(eventRecord, propName));
        else if (propFunc == PrintPresentFlags)         PrintPresentFlags(metadata->GetEventData<uint32_t>(eventRecord, propName));
        else if (propFunc == PrintPresentHistoryModel)  PrintPresentHistoryModel(metadata->GetEventData<uint32_t>(eventRecord, propName));
        else assert(false);
    }
    printf("\n");
}

void PrintUpdateHeader(uint64_t id, int indent=0)
{
    printf("%*sp%llu", 17 + 6 + 6 + indent*4, "", id);
}

void FlushModifiedPresent()
{
    if (gModifiedPresent == nullptr) return;

    uint32_t changedCount = 0;
#define FLUSH_MEMBER(_Fn, _Name) \
    if (gModifiedPresent->_Name != gOriginalPresentValues._Name) { \
        if (changedCount++ == 0) PrintUpdateHeader(gModifiedPresent->Id); \
        printf(" " #_Name "="); \
        _Fn(gOriginalPresentValues._Name); \
        printf("->"); \
        _Fn(gModifiedPresent->_Name); \
    }
    FLUSH_MEMBER(PrintTimeDelta,     TimeTaken)
    FLUSH_MEMBER(PrintTime,          ReadyTime)
    FLUSH_MEMBER(PrintTime,          ScreenTime)
    FLUSH_MEMBER(PrintU64x,          SwapChainAddress)
    FLUSH_MEMBER(PrintU32,           SyncInterval)
    FLUSH_MEMBER(PrintU32,           PresentFlags)
    FLUSH_MEMBER(PrintU64x,          Hwnd)
    FLUSH_MEMBER(PrintU64x,          DxgkPresentHistoryToken)
    FLUSH_MEMBER(PrintU32,           QueueSubmitSequence)
    FLUSH_MEMBER(PrintU32,           DriverBatchThreadId)
    FLUSH_MEMBER(PrintPresentMode,   PresentMode)
    FLUSH_MEMBER(PrintPresentResult, FinalState)
    FLUSH_MEMBER(PrintBool,          SupportsTearing)
    FLUSH_MEMBER(PrintBool,          MMIO)
    FLUSH_MEMBER(PrintBool,          SeenDxgkPresent)
    FLUSH_MEMBER(PrintBool,          SeenWin32KEvents)
    FLUSH_MEMBER(PrintBool,          DwmNotified)
    FLUSH_MEMBER(PrintBool,          IsCompleted)
    FLUSH_MEMBER(PrintBool,          IsLost)
    FLUSH_MEMBER(PrintU32,           DeferredCompletionWaitCount)
#undef FLUSH_MEMBER
    if (changedCount > 0) {
        printf("\n");
    }

    gModifiedPresent = nullptr;
}

}

void DebugInitialize(LARGE_INTEGER* firstTimestamp, LARGE_INTEGER const& timestampFrequency)
{
    gDebugDone = false;
    gFirstTimestamp = firstTimestamp;
    gTimestampFrequency = timestampFrequency;

    printf("       Time (ns)   PID   TID EVENT\n");
}

bool DebugDone()
{
    return gDebugDone;
}

void DebugEvent(EVENT_RECORD* eventRecord, EventMetadata* metadata)
{
    auto const& hdr = eventRecord->EventHeader;
    auto id = hdr.EventDescriptor.Id;

    FlushModifiedPresent();

    auto t = ConvertTimestampToNs(hdr.TimeStamp.QuadPart);
    if (t >= DEBUG_START_TIME_NS) {
        gDebugTrace = true;
    }

    if (t >= DEBUG_STOP_TIME_NS) {
        gDebugTrace = false;
        gDebugDone = true;
    }

    if (!gDebugTrace) {
        return;
    }

    if (hdr.ProviderId == Microsoft_Windows_D3D9::GUID) {
        using namespace Microsoft_Windows_D3D9;
        switch (id) {
        case Present_Start::Id: PrintEventHeader(hdr, "D3D9PresentStart"); break;
        case Present_Stop::Id:  PrintEventHeader(hdr, "D3D9PresentStop"); break;
        }
        return;
    }

    if (hdr.ProviderId == Microsoft_Windows_DXGI::GUID) {
        using namespace Microsoft_Windows_DXGI;
        switch (id) {
        case Present_Start::Id:                  PrintEventHeader(eventRecord, metadata, "DXGIPresent_Start",    { L"Flags", PrintPresentFlags, }); break;
        case PresentMultiplaneOverlay_Start::Id: PrintEventHeader(eventRecord, metadata, "DXGIPresentMPO_Start", { L"Flags", PrintPresentFlags, }); break;
        case Present_Stop::Id:                   PrintEventHeader(hdr, "DXGIPresent_Stop"); break;
        case PresentMultiplaneOverlay_Stop::Id:  PrintEventHeader(hdr, "DXGIPresentMPO_Stop"); break;
        }
        return;
    }

    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::BLT_GUID)            { PrintEventHeader(hdr, "Win7::BLT"); return; }
    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::FLIP_GUID)           { PrintEventHeader(hdr, "Win7::FLIP"); return; }
    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::PRESENTHISTORY_GUID) { PrintEventHeader(hdr, "Win7::PRESENTHISTORY"); return; }
    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::QUEUEPACKET_GUID)    { PrintEventHeader(hdr, "Win7::QUEUEPACKET"); return; }
    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::VSYNCDPC_GUID)       { PrintEventHeader(hdr, "Win7::VSYNCDPC"); return; }
    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::Win7::MMIOFLIP_GUID)       { PrintEventHeader(hdr, "Win7::MMIOFLIP"); return; }

    if (hdr.ProviderId == Microsoft_Windows_DxgKrnl::GUID) {
        using namespace Microsoft_Windows_DxgKrnl;
        switch (id) {
        case Blit_Info::Id:                     PrintEventHeader(hdr, "Blit_Info"); break;
        case BlitCancel_Info::Id:               PrintEventHeader(hdr, "BlitCancel_Info"); break;
        case FlipMultiPlaneOverlay_Info::Id:    PrintEventHeader(hdr, "FlipMultiPlaneOverlay_Info"); break;
        case HSyncDPCMultiPlane_Info::Id:       PrintEventHeader(hdr, "HSyncDPCMultiPlane_Info"); break;
        case VSyncDPCMultiPlane_Info::Id:       PrintEventHeader(hdr, "VSyncDPCMultiPlane_Info"); break;
        case MMIOFlip_Info::Id:                 PrintEventHeader(hdr, "MMIOFlip_Info"); break;
        case Present_Info::Id:                  PrintEventHeader(hdr, "DxgKrnl_Present_Info"); break;

        case Flip_Info::Id:                     PrintEventHeader(eventRecord, metadata, "Flip_Info",                    { L"FlipInterval",   PrintU32,
                                                                                                                          L"MMIOFlip",       PrintBool, }); break;
        case IndependentFlip_Info::Id:          PrintEventHeader(eventRecord, metadata, "IndependentFlip_Info",         { L"SubmitSequence", PrintU32,
                                                                                                                          L"FlipInterval",   PrintU32, }); break;
        case PresentHistory_Start::Id:          PrintEventHeader(eventRecord, metadata, "PresentHistory_Start",         { L"Token",          PrintU64x,
                                                                                                                          L"Model",          PrintPresentHistoryModel, }); break;
        case PresentHistory_Info::Id:           PrintEventHeader(eventRecord, metadata, "PresentHistory_Info",          { L"Token",          PrintU64x,
                                                                                                                          L"Model",          PrintPresentHistoryModel, }); break;
        case PresentHistoryDetailed_Start::Id:  PrintEventHeader(eventRecord, metadata, "PresentHistoryDetailed_Start", { L"Token",          PrintU64x,
                                                                                                                          L"Model",          PrintPresentHistoryModel, }); break;
        case QueuePacket_Start::Id:             PrintEventHeader(eventRecord, metadata, "QueuePacket_Start",            { L"hContext",       PrintU64x,
                                                                                                                          L"SubmitSequence", PrintU32,
                                                                                                                          L"PacketType",     PrintQueuePacketType,
                                                                                                                          L"bPresent",       PrintU32, }); break;
        case QueuePacket_Stop::Id:              PrintEventHeader(eventRecord, metadata, "QueuePacket_Stop",             { L"hContext",       PrintU64x,
                                                                                                                          L"SubmitSequence", PrintU32, }); break;
        case VSyncDPC_Info::Id:                 PrintEventHeader(eventRecord, metadata, "VSyncDPC_Info",                { L"FlipFenceId",    PrintU64x, }); break;

        case MMIOFlipMultiPlaneOverlay_Info::Id:
            PrintEventHeader(hdr);
            printf("DXGKrnl_MMIOFlipMultiPlaneOverlay_Info FlipSubmitSequence=%llx", metadata->GetEventData<uint64_t>(eventRecord, L"FlipSubmitSequence"));
            if (hdr.EventDescriptor.Version >= 2) {
                switch (metadata->GetEventData<uint32_t>(eventRecord, L"FlipEntryStatusAfterFlip")) {
                case FlipEntryStatus::FlipWaitVSync:    printf(" FlipWaitVSync"); break;
                case FlipEntryStatus::FlipWaitComplete: printf(" FlipWaitComplete"); break;
                case FlipEntryStatus::FlipWaitHSync:    printf(" FlipWaitHSync"); break;
                }
            }
            printf("\n");
            break;
        }
        return;
    }

    if (hdr.ProviderId == Microsoft_Windows_Dwm_Core::GUID ||
        hdr.ProviderId == Microsoft_Windows_Dwm_Core::Win7::GUID) {
        using namespace Microsoft_Windows_Dwm_Core;
        switch (id) {
        case MILEVENT_MEDIA_UCE_PROCESSPRESENTHISTORY_GetPresentHistory_Info::Id:
                                              PrintEventHeader(hdr, "DWM_GetPresentHistory"); break;
        case SCHEDULE_PRESENT_Start::Id:      PrintEventHeader(hdr, "DWM_SCHEDULE_PRESENT_Start"); break;
        case FlipChain_Pending::Id:           PrintEventHeader(hdr, "DWM_FlipChain_Pending"); break;
        case FlipChain_Complete::Id:          PrintEventHeader(hdr, "DWM_FlipChain_Complete"); break;
        case FlipChain_Dirty::Id:             PrintEventHeader(hdr, "DWM_FlipChain_Dirty"); break;
        case SCHEDULE_SURFACEUPDATE_Info::Id: PrintEventHeader(hdr, "DWM_Schedule_SurfaceUpdate"); break;
        }
        return;
    }

    if (hdr.ProviderId == Microsoft_Windows_Win32k::GUID) {
        using namespace Microsoft_Windows_Win32k;
        switch (id) {
        case TokenCompositionSurfaceObject_Info::Id: PrintEventHeader(hdr, "Win32k_TokenCompositionSurfaceObject"); break;
        case TokenStateChanged_Info::Id:             PrintEventHeader(eventRecord, metadata, "Win32K_TokenStateChanged", { L"NewState", PrintTokenState, }); break;
        }
        return;
    }

    assert(false);
}

void DebugModifyPresent(PresentEvent const* p)
{
    if (!gDebugTrace) return;
    if (gModifiedPresent != p) {
        FlushModifiedPresent();
        gModifiedPresent = p;
        if (p != nullptr) {
            gOriginalPresentValues = *p;
        }
    }
}

void DebugCreatePresent(PresentEvent const& p)
{
    if (!gDebugTrace) return;
    FlushModifiedPresent();
    PrintUpdateHeader(p.Id);
    printf(" CreatePresent");
    printf(" ProcessId=%u", p.ProcessId);
    printf(" SwapChainAddress=%llx", p.SwapChainAddress);
    printf(" PresentFlags=%x", p.PresentFlags);
    printf(" SyncInterval=%u", p.SyncInterval);
    printf(" Runtime=");
    PrintRuntime(p.Runtime);
    printf("\n");
}

#endif // if DEBUG_VERBOSE
