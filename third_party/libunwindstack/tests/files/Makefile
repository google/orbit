# Copyright (C) 2021 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

MINGW_CC=x86_64-w64-mingw32-gcc
MINGW_CC_32=i686-w64-mingw32-gcc

libtest: ./TestLibrary.cpp
	${MINGW_CC} ./TestLibrary.cpp -shared -g -gdwarf -o libtest.dll

libtest32: ./TestLibrary.cpp
	${MINGW_CC_32} ./TestLibrary.cpp -shared -g -gdwarf -o libtest32.dll

clean:
	rm -f libtest.dll libtest32.dll