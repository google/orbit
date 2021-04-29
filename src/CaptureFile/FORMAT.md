# Capture file format

Version: 1

This document describes capture file format for Orbit.

## File structure

The file consists of multiple sections. Most of the sections are optional and
may not be present in the file. The header and Capture Section are the only mandatory sections.

| Section                 |           |
|-------------------------|-----------|
| Header                  | mandatory | 
| Capture Section         | mandatory |
| Additional Section 1    | optional  |
| ...                     | optional  |
| Additional Section N    | optional  |
| Additional Section List | optional  |

Note that the Header Section has to go first but other than that all other section may appear
in any order. This includes the CaptureSection, the code reading this file shouldn't rely on
sections appearing in any particular order.

### Header

| Field                          | Size | Comment                                                   |
|--------------------------------|-----:|-----------------------------------------------------------|
| Signature                      | 4    | 'ORBT'                                                    |
| Version                        | 4    | Format version                                            | 
| Capture Section Offset         | 8    | Offset from the start of the file                         |
| Additional Section List Offset | 8    | May be 0 if there are no additional sections in this file |

### Capture Section
Capture section is a sequence of `orbit_grpc_proto::ClientCaptureEvent` messages. The first message is
always `orbit_grpc_proto::CaptureStarted` and the last one is `orbit_grpc_proto::CapureFinished`.

### Additional Section List
The following is a format of Additional Section List

| Field                          | Size | Comment                                                   |
|--------------------------------|-----:|-----------------------------------------------------------|
| Number of sections             | 2    | The size of the list in bytes                             |
| Section Header 1               | 24   | Section header                                            |  
| ...                            |      |                                                           |
| Section Header N               | 24   | Section header                                            |

#### Section Header

| Field  | Size | Comment                                          |
|--------|------|--------------------------------------------------|
| Type   | 8    | Section type                                     |
| Offset | 8    | Offset of the section from the start of the file |
| Size   | 8    | Section size is bytes                            |

### Additional Sections

Here we list all the sections currently supported by the capture file format.

The following table lists all the values for section types.

| Section Type | Value | Comment                     |
|--------------|-------|-----------------------------|
| RESERVED     | 0     | 0 is reserved - do not use. |
| USER_DATA    | 1     | This section contains user-defined data like visible frame-tracks, track order, colors, bookmars, etc. |

#### USER_DATA

User Data section content is `orbit_client_protos::UserDefinedCaptureInfo` proto message.  

