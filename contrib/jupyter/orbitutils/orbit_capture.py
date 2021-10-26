"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

# This is a hack to make sure all generated proto files are found when importing. I wasn't able
# to find a proper solution for this, yet.
import sys
sys.path.append("./orbitutils")

import orbitutils.capture_pb2 as proto
import struct
from google.protobuf.internal.decoder import _DecodeVarint32

class OrbitCapture:
    def __init__(self, file_content):
        pos = 0
        magic = file_content[pos:pos + 4]
        pos += 4
        self.version = struct.unpack_from("<I", file_content[pos:pos + 4])[0]
        pos += 4
        self.capture_section_offset = struct.unpack_from("<Q", file_content[pos:pos + 8])[0]
        pos += 8
        self.section_list_offset = struct.unpack_from("<Q", file_content[pos:pos + 8])[0]
        pos+= 8

        self.read_section_list(file_content, self.section_list_offset)
        self.calculate_capture_section_size(file_content)
        self.read_capture_section(file_content)

    def calculate_capture_section_size(self, file_content):
        if self.section_list_offset == 0:
            self.capture_section_size = len(file_content) - self.capture_section_offset
            return
        section_offsets = map(lambda element: element[1], self.sections)
        capture_section_end_offset = min(self.section_list_offset, min(section_offsets))
        self.capture_section_size = capture_section_end_offset - self.capture_section_offset

    def read_capture_section(self, file_content):
        pos = self.capture_section_offset
        decoder = _DecodeVarint32
        self.events = []
        count_failed = 0
        while pos < self.capture_section_offset + self.capture_section_size:
            try:
                (msg_len, new_pos) = decoder(file_content, pos)
                pos = new_pos
                msg = proto.ClientCaptureEvent()
                event = msg.FromString(file_content[pos:pos + msg_len])
                self.events.append(event)
                pos += msg_len
            except:
                count_failed += 1
                continue
        if count_failed != 0:
            print("count_failed: " + str(count_failed))

    def read_section_list(self, file_content, section_list_offset):
        if (section_list_offset == 0):
            return
        pos = section_list_offset
        number_of_sections = struct.unpack_from("<Q", file_content[pos:pos+8])[0]
        pos += 8
        self.sections = []
        while pos < section_list_offset + 3 * 8 * number_of_sections:
            section_info = struct.unpack_from("<QQQ", file_content[pos:pos + 3*8])[0:3]
            pos += 3 * 8
            self.sections.append(section_info)
        return
