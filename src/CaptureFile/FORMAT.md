# Capture file format
This document describes capture file format.

## File structure

The file consists of multiple sections. Most of the sections are optional and
may not be present in the file. The header and Capture Section are the only mandatory sections.

| Section                 |           |
|-------------------------|-----------|
| Header                  | mandatory | 
| Capture Section         | mandatory |
| Additional Section List | optional  |
| Additional Section 1    | optional  |
| ...                     | optional  |
| Additional Section N    | optional  |

### Header

| Field                          | Size | Comment                                                   |
|--------------------------------|-----:|-----------------------------------------------------------|
| Signature                      | 4    | 'ORBT'                                                    |
| Version                        | 4    | Format version                                            | 
| Capture Section Offset         | 8    | Offset from the start of the file                         |
| Additional Section List Offset | 8    | May be 0 if there are no additional sections in this file |

### Capture Section
Capture section is a sequence of `orbit_grpc_proto::ClientCaptureEvent` messages. The first message is
always `orbit_grpc_proto::CaptureStarted` and the last one is `orbit_grpc_proto::CapureDone`.


