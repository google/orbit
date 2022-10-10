# Capture file format

Version: 1

This document describes capture file format for Orbit.

## File structure

The file consists of multiple sections. The Header and Capture Section are mandatory sections.
Other additional sections are optional and may not be present in the file.

| Section                                   |           |
|-------------------------------------------|-----------|
| Header                                    | mandatory | 
| Capture Section                           | mandatory |
| Additional (read-only) Section 1          | optional  |
| ...                                       | optional  |
| Additional (read-only) Section N          | optional  |
| Section List                              | optional  |
| Additional (read-write) User Data Section | optional  |

Note that the file has the following order. The Header has to go first. The Capture Section is second.
Then come N additional read-only sections. Afterwards comes the Section List and last the User Data
Section. Note that the section list only contains the additional sections (including the user data
section). The [USER_DATA](#user_data) is a read-write section.

### Header

| Field                          | Size | Comment                                                   |
|--------------------------------|-----:|-----------------------------------------------------------|
| Signature                      | 4    | 'ORBT'                                                    |
| Version                        | 4    | Format version                                            | 
| Capture Section Offset         | 8    | Offset from the start of the file                         |
| Additional Section List Offset | 8    | May be 0 if there are no additional sections in this file |

### Capture Section
Capture section is a sequence of `orbit_grpc_protos::ClientCaptureEvent` messages. The first message is
always `orbit_grpc_protos::CaptureStarted` and the last one is `orbit_grpc_protos::CapureFinished`.

### Additional Section List
The following is a format of Additional Section List

| Field                          | Size | Comment                                                   |
|--------------------------------|-----:|-----------------------------------------------------------|
| Number of sections             | 8    | The size of the list in bytes                             |
| Section Header 1               | 24   | Section header                                            |  
| ...                            |      |                                                           |
| Section Header N               | 24   | Section header                                            |

Note that the section list size is 8 bytes but it is still limited by 65'535 entries.

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
| USER_DATA    | 1     | This section contains user-defined data like visible frame-tracks, track order, colors, bookmarks, etc. |

#### USER_DATA

User Data section content is `orbit_client_protos::UserDefinedCaptureInfo` proto message.
This section is read/write section that could be modified after the capture has been taken.
For optimization reason this section is always placed at the end of file. Nothing should go
after this section including the section list itself.

#### How the protobuf messages are written
All protobuf messages in sections are prepended by the Varint32 message size, even if
the section contains only one protobuf message.
