/* Copyright 2022 Adam Green (https://github.com/adamgreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Nano 33 BLE Memory layout placed in g_memoryMapXml. */
#ifndef MRI_NANO_33_BLE_H_
#define MRI_NANO_33_BLE_H_

static const char g_memoryMapXml[] = "<?xml version=\"1.0\"?>"
                                     "<!DOCTYPE memory-map PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\" \"http://sourceware.org/gdb/gdb-memory-map.dtd\">"
                                     "<memory-map>"
                                     "<memory type=\"flash\" start=\"0x00000000\" length=\"0x100000\"> <property name=\"blocksize\">0x1000</property></memory>"
                                     "<memory type=\"ram\" start=\"0x20000000\" length=\"0x40000\"> </memory>"
                                     "<memory type=\"ram\" start=\"0x00800000\" length=\"0x40000\"> </memory>"
                                     "</memory-map>";

#endif /* MRI_NANO_33_BLE_H_ */
