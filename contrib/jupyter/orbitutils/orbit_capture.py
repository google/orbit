"""
Copyright (c) 2021 The Orbit Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
"""

import orbitutils.ClientProtos.capture_data_pb2 as proto
import struct

class OrbitCapture:
    """Class that holds a single Orbit capture and all its information.

    Can be construced from the file content of an .orbit capture file
    (e.g. obtained via open(filename, 'rb').read()).
    
    Attributes:
        header:
            The capture header including version of the capture.

            See capture_data.proto for full details of all members.
        capture_info:
            The capture info, which includes various mappings from ids to
            instrumented functins, thread names; callstack events with
            callstack id and a mapping from callstack id to the actual
            callstacks; module information including address ranges;
            basic information about the target process.

            See capture_data.proto for full details of all members.
        timer_infos:
            A list of all timers (timeslices with a start and end time) of
            the capture. This includes scheduling information, instrumented
            function times, GPU scheduling and execution, and more.

            See capture_data.proto (in particular, TimerInfo) for full
            details of all members.
    """
    def __init__(self, file_content):
        pos = 0
        # The capture format is a list of protobuf messages, each preceded
        # by its message size. Message sizes are written as little Endian
        # 32-bit integers.

        # The capture format starts with the capture header.
        msg_len = struct.unpack_from("<I", file_content[pos:pos + 4])[0]
        pos += 4
        msg = proto.CaptureHeader()
        self.header = msg.FromString(file_content[pos:pos + msg_len])
        pos += msg_len

        # Next is the capture info.
        msg_len = struct.unpack_from("<I", file_content[pos:pos + 4])[0]
        pos += 4
        msg = proto.CaptureInfo()
        self.capture_info = msg.FromString(file_content[pos:pos + msg_len])
        pos += msg_len

        # And finally an array of TimerInfo messages.
        self.timer_infos = []
        while pos < len(file_content):
            msg_len = struct.unpack_from("<I", file_content[pos:pos + 4])[0]
            pos += 4
            msg = proto.TimerInfo()
            timer = msg.FromString(file_content[pos:pos + msg_len])
            self.timer_infos.append(timer)
            pos += msg_len

    def compute_min_time_ns(self):
        """ Minimum time of the capture (in nanoseconds)
        """
        start_times = map(lambda timer: timer.start, self.timer_infos)
        return min(start_times)

    def compute_max_time_ns(self):
        """ Maximum time of the capture (in nanoseconds)
        """
        end_times = map(lambda timer: timer.end, self.timer_infos)
        return max(end_times)
