//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

// clang-format off
#include "Platform.h"
#include "BaseTypes.h"
// clang-format on

#include "EventUtils.h"

#include <evntrace.h>
#include <evntcons.h>
#include <in6addr.h>
#include <stdio.h>
#include <tdh.h>
#include <wbemidl.h>
#include <wmistr.h>

#pragma comment(lib, "tdh.lib")

//-----------------------------------------------------------------------------
DWORD GetEventInformation(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO& pInfo);
PBYTE PrintProperties(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo,
                      DWORD PointerSize, USHORT i, PBYTE pUserData,
                      PBYTE pEndOfUserData);
DWORD GetPropertyLength(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo, USHORT i,
                        PUSHORT PropertyLength);
DWORD GetArraySize(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo, USHORT i,
                   PUSHORT ArraySize);
DWORD GetMapInfo(PEVENT_RECORD pEvent, LPWSTR pMapName, DWORD DecodingSource,
                 PEVENT_MAP_INFO& pMapInfo);
void RemoveTrailingSpace(PEVENT_MAP_INFO pMapInfo);

//-----------------------------------------------------------------------------
void EventUtils::OutputDebugEvent(PEVENT_RECORD pEvent) {
  DWORD status = ERROR_SUCCESS;
  PTRACE_EVENT_INFO pInfo = NULL;
  LPWSTR pwsEventGuid = NULL;
  PBYTE pUserData = NULL;
  PBYTE pEndOfUserData = NULL;
  DWORD PointerSize = 0;
  ULONGLONG TimeStamp = 0;
  ULONGLONG Nanoseconds = 0;
  SYSTEMTIME st;
  SYSTEMTIME stLocal;
  FILETIME ft;

  // Skips the event if it is the event trace header. Log files contain this
  // event but real-time sessions do not. The event contains the same
  // information as the EVENT_TRACE_LOGFILE.LogfileHeader member that you can
  // access when you open the trace.

  if (IsEqualGUID(pEvent->EventHeader.ProviderId, EventTraceGuid) &&
      pEvent->EventHeader.EventDescriptor.Opcode == EVENT_TRACE_TYPE_INFO) {
    ;  // Skip this event.
  } else {
    // Process the event. The pEvent->UserData member is a pointer to
    // the event specific data, if it exists.

    status = GetEventInformation(pEvent, pInfo);

    if (ERROR_SUCCESS != status) {
      LOG("GetEventInformation failed with %lu", status);
      goto cleanup;
    }

    // Determine whether the event is defined by a MOF class, in an
    // instrumentation manifest, or a WPP template; to use TDH to decode
    // the event, it must be defined by one of these three sources.

    if (DecodingSourceWbem == pInfo->DecodingSource)  // MOF class
    {
      HRESULT hr = StringFromCLSID(pInfo->EventGuid, &pwsEventGuid);

      if (FAILED(hr)) {
        LOG("StringFromCLSID failed with 0x%x", hr);
        status = hr;
        goto cleanup;
      }

      LOG("\nEvent GUID: %s", pwsEventGuid);
      CoTaskMemFree(pwsEventGuid);
      pwsEventGuid = NULL;

      LOG("Event version: %d", pEvent->EventHeader.EventDescriptor.Version);
      LOG("Event type: %d", pEvent->EventHeader.EventDescriptor.Opcode);
    } else if (DecodingSourceXMLFile ==
               pInfo->DecodingSource)  // Instrumentation manifest
    {
      LOG("Event ID: %d", pInfo->EventDescriptor.Id);
    } else  // Not handling the WPP case
    {
      goto cleanup;
    }

    // Print the time stamp for when the event occurred.

    ft.dwHighDateTime = pEvent->EventHeader.TimeStamp.HighPart;
    ft.dwLowDateTime = pEvent->EventHeader.TimeStamp.LowPart;

    FileTimeToSystemTime(&ft, &st);
    SystemTimeToTzSpecificLocalTime(NULL, &st, &stLocal);

    TimeStamp = pEvent->EventHeader.TimeStamp.QuadPart;
    Nanoseconds = (TimeStamp % 10000000) * 100;

    LOG("%02d/%02d/%02d %02d:%02d:%02d.%I64u", stLocal.wMonth, stLocal.wDay,
        stLocal.wYear, stLocal.wHour, stLocal.wMinute, stLocal.wSecond,
        Nanoseconds);

    // If the event contains event-specific data use TDH to extract
    // the event data. For this example, to extract the data, the event
    // must be defined by a MOF class or an instrumentation manifest.

    // Need to get the PointerSize for each event to cover the case where you
    // are consuming events from multiple log files that could have been
    // generated on different architectures. Otherwise, you could have accessed
    // the pointer size when you opened the trace above (see
    // pHeader->PointerSize).

    if (EVENT_HEADER_FLAG_32_BIT_HEADER ==
        (pEvent->EventHeader.Flags & EVENT_HEADER_FLAG_32_BIT_HEADER)) {
      PointerSize = 4;
    } else {
      PointerSize = 8;
    }

    pUserData = (PBYTE)pEvent->UserData;
    pEndOfUserData = (PBYTE)pEvent->UserData + pEvent->UserDataLength;

    // Print the event data for all the top-level properties. Metadata for all
    // the top-level properties come before structure member properties in the
    // property information array.

    for (USHORT i = 0; i < pInfo->TopLevelPropertyCount; i++) {
      pUserData = PrintProperties(pEvent, pInfo, PointerSize, i, pUserData,
                                  pEndOfUserData);
      if (NULL == pUserData) {
        LOG("Printing top level properties failed.");
        goto cleanup;
      }
    }
  }

cleanup:

  if (pInfo) {
    free(pInfo);
  }

  // if( ERROR_SUCCESS != status || NULL == pUserData )
  //{
  //    CloseTrace( g_hTrace );
  //}
}

// Print the property.

PBYTE PrintProperties(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo,
                      DWORD PointerSize, USHORT i, PBYTE pUserData,
                      PBYTE pEndOfUserData) {
  TDHSTATUS status = ERROR_SUCCESS;
  USHORT PropertyLength = 0;
  DWORD FormattedDataSize = 0;
  USHORT UserDataConsumed = 0;
  LPWSTR pFormattedData = NULL;
  DWORD LastMember = 0;  // Last member of a structure
  USHORT ArraySize = 0;
  PEVENT_MAP_INFO pMapInfo = NULL;

  // Get the length of the property.

  status = GetPropertyLength(pEvent, pInfo, i, &PropertyLength);
  if (ERROR_SUCCESS != status) {
    LOG("GetPropertyLength failed.");
    pUserData = NULL;
    goto cleanup;
  }

  // Get the size of the array if the property is an array.

  status = GetArraySize(pEvent, pInfo, i, &ArraySize);

  for (USHORT k = 0; k < ArraySize; k++) {
    // If the property is a structure, print the members of the structure.

    if ((pInfo->EventPropertyInfoArray[i].Flags & PropertyStruct) ==
        PropertyStruct) {
      LastMember =
          pInfo->EventPropertyInfoArray[i].structType.StructStartIndex +
          pInfo->EventPropertyInfoArray[i].structType.NumOfStructMembers;

      for (USHORT j =
               pInfo->EventPropertyInfoArray[i].structType.StructStartIndex;
           j < LastMember; j++) {
        pUserData = PrintProperties(pEvent, pInfo, PointerSize, j, pUserData,
                                    pEndOfUserData);
        if (NULL == pUserData) {
          LOG("Printing the members of the structure failed.");
          pUserData = NULL;
          goto cleanup;
        }
      }
    } else {
      // Get the name/value mapping if the property specifies a value map.

      status = GetMapInfo(
          pEvent,
          (PWCHAR)(
              (PBYTE)(pInfo) +
              pInfo->EventPropertyInfoArray[i].nonStructType.MapNameOffset),
          pInfo->DecodingSource, pMapInfo);

      if (ERROR_SUCCESS != status) {
        LOG("GetMapInfo failed");
        pUserData = NULL;
        goto cleanup;
      }

      // Get the size of the buffer required for the formatted data.

      status = TdhFormatProperty(
          pInfo, pMapInfo, PointerSize,
          pInfo->EventPropertyInfoArray[i].nonStructType.InType,
          pInfo->EventPropertyInfoArray[i].nonStructType.OutType,
          PropertyLength, (USHORT)(pEndOfUserData - pUserData), pUserData,
          &FormattedDataSize, pFormattedData, &UserDataConsumed);

      if (ERROR_INSUFFICIENT_BUFFER == status) {
        if (pFormattedData) {
          free(pFormattedData);
          pFormattedData = NULL;
        }

        pFormattedData = (LPWSTR)malloc(FormattedDataSize);
        if (pFormattedData == NULL) {
          LOG("Failed to allocate memory for formatted data (size=%lu).",
              FormattedDataSize);
          status = ERROR_OUTOFMEMORY;
          pUserData = NULL;
          goto cleanup;
        }

        // Retrieve the formatted data.

        status = TdhFormatProperty(
            pInfo, pMapInfo, PointerSize,
            pInfo->EventPropertyInfoArray[i].nonStructType.InType,
            pInfo->EventPropertyInfoArray[i].nonStructType.OutType,
            PropertyLength, (USHORT)(pEndOfUserData - pUserData), pUserData,
            &FormattedDataSize, pFormattedData, &UserDataConsumed);
      }

      if (ERROR_SUCCESS == status) {
        LOG("%s: %s",
            (PWCHAR)((PBYTE)(pInfo) +
                     pInfo->EventPropertyInfoArray[i].NameOffset),
            pFormattedData);

        pUserData += UserDataConsumed;
      } else {
        LOG("TdhFormatProperty failed with %lu.", status);
        pUserData = NULL;
        goto cleanup;
      }
    }
  }

cleanup:

  if (pFormattedData) {
    free(pFormattedData);
    pFormattedData = NULL;
  }

  if (pMapInfo) {
    free(pMapInfo);
    pMapInfo = NULL;
  }

  return pUserData;
}

// Get the length of the property data. For MOF-based events, the size is
// inferred from the data type of the property. For manifest-based events, the
// property can specify the size of the property value using the length
// attribute. The length attribue can specify the size directly or specify the
// name of another property in the event data that contains the size. If the
// property does not include the length attribute, the size is inferred from the
// data type. The length will be zero for variable length, null-terminated
// strings and structures.

DWORD GetPropertyLength(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo, USHORT i,
                        PUSHORT PropertyLength) {
  DWORD status = ERROR_SUCCESS;
  PROPERTY_DATA_DESCRIPTOR DataDescriptor;
  DWORD PropertySize = 0;

  // If the property is a binary blob and is defined in a manifest, the property
  // can specify the blob's size or it can point to another property that
  // defines the blob's size. The PropertyParamLength flag tells you where the
  // blob's size is defined.

  if ((pInfo->EventPropertyInfoArray[i].Flags & PropertyParamLength) ==
      PropertyParamLength) {
    DWORD Length = 0;  // Expects the length to be defined by a UINT16 or UINT32
    DWORD j = pInfo->EventPropertyInfoArray[i].lengthPropertyIndex;
    ZeroMemory(&DataDescriptor, sizeof(PROPERTY_DATA_DESCRIPTOR));
    DataDescriptor.PropertyName = (ULONGLONG)(
        (PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[j].NameOffset);
    DataDescriptor.ArrayIndex = ULONG_MAX;
    status =
        TdhGetPropertySize(pEvent, 0, NULL, 1, &DataDescriptor, &PropertySize);
    status = TdhGetProperty(pEvent, 0, NULL, 1, &DataDescriptor, PropertySize,
                            (PBYTE)&Length);
    *PropertyLength = (USHORT)Length;
  } else {
    if (pInfo->EventPropertyInfoArray[i].length > 0) {
      *PropertyLength = pInfo->EventPropertyInfoArray[i].length;
    } else {
      // If the property is a binary blob and is defined in a MOF class, the
      // extension qualifier is used to determine the size of the blob. However,
      // if the extension is IPAddrV6, you must set the PropertyLength variable
      // yourself because the EVENT_PROPERTY_INFO.length field will be zero.

      if (TDH_INTYPE_BINARY ==
              pInfo->EventPropertyInfoArray[i].nonStructType.InType &&
          TDH_OUTTYPE_IPV6 ==
              pInfo->EventPropertyInfoArray[i].nonStructType.OutType) {
        *PropertyLength = (USHORT)sizeof(IN6_ADDR);
      } else if (TDH_INTYPE_UNICODESTRING ==
                     pInfo->EventPropertyInfoArray[i].nonStructType.InType ||
                 TDH_INTYPE_ANSISTRING ==
                     pInfo->EventPropertyInfoArray[i].nonStructType.InType ||
                 (pInfo->EventPropertyInfoArray[i].Flags & PropertyStruct) ==
                     PropertyStruct) {
        *PropertyLength = pInfo->EventPropertyInfoArray[i].length;
      } else {
        LOG("Unexpected length of 0 for intype %d and outtype %d",
            pInfo->EventPropertyInfoArray[i].nonStructType.InType,
            pInfo->EventPropertyInfoArray[i].nonStructType.OutType);

        status = ERROR_EVT_INVALID_EVENT_DATA;
        goto cleanup;
      }
    }
  }

cleanup:

  return status;
}

// Get the size of the array. For MOF-based events, the size is specified in the
// declaration or using the MAX qualifier. For manifest-based events, the
// property can specify the size of the array using the count attribute. The
// count attribue can specify the size directly or specify the name of another
// property in the event data that contains the size.

DWORD GetArraySize(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO pInfo, USHORT i,
                   PUSHORT ArraySize) {
  DWORD status = ERROR_SUCCESS;
  PROPERTY_DATA_DESCRIPTOR DataDescriptor;
  DWORD PropertySize = 0;

  if ((pInfo->EventPropertyInfoArray[i].Flags & PropertyParamCount) ==
      PropertyParamCount) {
    DWORD Count = 0;  // Expects the count to be defined by a UINT16 or UINT32
    DWORD j = pInfo->EventPropertyInfoArray[i].countPropertyIndex;
    ZeroMemory(&DataDescriptor, sizeof(PROPERTY_DATA_DESCRIPTOR));
    DataDescriptor.PropertyName = (ULONGLONG)(
        (PBYTE)(pInfo) + pInfo->EventPropertyInfoArray[j].NameOffset);
    DataDescriptor.ArrayIndex = ULONG_MAX;
    status =
        TdhGetPropertySize(pEvent, 0, NULL, 1, &DataDescriptor, &PropertySize);
    status = TdhGetProperty(pEvent, 0, NULL, 1, &DataDescriptor, PropertySize,
                            (PBYTE)&Count);
    *ArraySize = (USHORT)Count;
  } else {
    *ArraySize = pInfo->EventPropertyInfoArray[i].count;
  }

  return status;
}

// Both MOF-based events and manifest-based events can specify name/value maps.
// The map values can be integer values or bit values. If the property specifies
// a value map, get the map.

DWORD GetMapInfo(PEVENT_RECORD pEvent, LPWSTR pMapName, DWORD DecodingSource,
                 PEVENT_MAP_INFO& pMapInfo) {
  DWORD status = ERROR_SUCCESS;
  DWORD MapSize = 0;

  // Retrieve the required buffer size for the map info.

  status = TdhGetEventMapInformation(pEvent, pMapName, pMapInfo, &MapSize);

  if (ERROR_INSUFFICIENT_BUFFER == status) {
    pMapInfo = (PEVENT_MAP_INFO)malloc(MapSize);
    if (pMapInfo == NULL) {
      LOG("Failed to allocate memory for map info (size=%lu).", MapSize);
      status = ERROR_OUTOFMEMORY;
      goto cleanup;
    }

    // Retrieve the map info.

    status = TdhGetEventMapInformation(pEvent, pMapName, pMapInfo, &MapSize);
  }

  if (ERROR_SUCCESS == status) {
    if (DecodingSourceXMLFile == DecodingSource) {
      RemoveTrailingSpace(pMapInfo);
    }
  } else {
    if (ERROR_NOT_FOUND == status) {
      status = ERROR_SUCCESS;  // This case is okay.
    } else {
      LOG("TdhGetEventMapInformation failed with 0x%x.", status);
    }
  }

cleanup:

  return status;
}

// The mapped string values defined in a manifest will contain a trailing space
// in the EVENT_MAP_ENTRY structure. Replace the trailing space with a null-
// terminating character, so that the bit mapped strings are correctly
// formatted.

void RemoveTrailingSpace(PEVENT_MAP_INFO pMapInfo) {
  DWORD ByteLength = 0;

  for (DWORD i = 0; i < pMapInfo->EntryCount; i++) {
    ByteLength =
        (DWORD)(wcslen((LPWSTR)((PBYTE)pMapInfo +
                                pMapInfo->MapEntryArray[i].OutputOffset)) -
                1) *
        2;
    *((LPWSTR)((PBYTE)pMapInfo +
               (pMapInfo->MapEntryArray[i].OutputOffset + ByteLength))) = L'\0';
  }
}

// Get the metadata for the event.

DWORD GetEventInformation(PEVENT_RECORD pEvent, PTRACE_EVENT_INFO& pInfo) {
  DWORD status = ERROR_SUCCESS;
  DWORD BufferSize = 0;

  // Retrieve the required buffer size for the event metadata.

  status = TdhGetEventInformation(pEvent, 0, NULL, pInfo, &BufferSize);

  if (ERROR_INSUFFICIENT_BUFFER == status) {
    pInfo = (TRACE_EVENT_INFO*)malloc(BufferSize);
    if (pInfo == NULL) {
      LOG("Failed to allocate memory for event info (size=%lu).", BufferSize);
      status = ERROR_OUTOFMEMORY;
      goto cleanup;
    }

    // Retrieve the event metadata.

    status = TdhGetEventInformation(pEvent, 0, NULL, pInfo, &BufferSize);
  }

  if (ERROR_SUCCESS != status) {
    LOG("TdhGetEventInformation failed with 0x%x.", status);
  }

cleanup:

  return status;
}
