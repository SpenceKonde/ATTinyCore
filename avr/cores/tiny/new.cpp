/*
  Copyright (c) 2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Updated 2020-21 Spence Konde to support C++ 14 sized
  dealocations, and give useful compile errors when aligned
  new and delete operators are referenced. I do not think adding
  support for them is warranted at this time, though we have
  candidates for the simplest of them, both were written by
  @henrygab (Henry Gabryjelski, which is fitting because that's
  who asked for C++ 17 mode, over on ATTinyCore, but releases
  are more frequent for the new parts, and this is equally
  applicable to all of the cores.)
*/

#include <stdlib.h>

// For C++11, only need the following:
void *operator new  (size_t size) {
  return malloc(size);
}

void *operator new[](size_t size) {
  return malloc(size);
}

void  operator delete  (void * ptr) {
  free(ptr);
}

void  operator delete[](void * ptr) {
  free(ptr);
}

void * operator new  (size_t size, void * ptr) noexcept {
  (void)size;
  return ptr;
}

void * operator new[](size_t size, void * ptr) noexcept {
  (void)size;
  return ptr;
}

// Since C++14, also need two more delete variants...
// Note thnat this CAN break code that compiled amd worked in C++11.
#if (__cpp_sized_deallocation >= 201309L)
  void  operator delete  (void* ptr, size_t size) noexcept {
    (void) size;
    free(ptr);
  }
  void  operator delete[](void* ptr, size_t size) noexcept {
    (void) size;
    free(ptr);
  }
#endif

// Since C++17, there's four more each for new / delete, to support allocation
// of objects with alignment greater than __STDCPP_DEFAULT_NEW_ALIGNMENT__.
// Almost the entirety of AVR is using C++ 11. Any code that works elsewhere, but NOT on these cores and give these error messages either
// is trying to get alignment that it doesn't actually need, or is - by sheet luck - ending up with the addresses which worked. E
#if (__cpp_aligned_new >= 201606L)

  void badAlloc(const char*) __attribute__((error("")));

  void* operator new  (size_t size, std::align_val_t al) {
    badAlloc("Alignment aware new/delete operators, a C++ 17 feaure, are not supported by this core or any other AVR cores at this point in time");
    (void) al;
    return malloc(size);
  }

/*
void* operator new  (size_t size, std::align_val_t al) {

  if (al <= alignof(std::max_align_t)) {
      return malloc(size);
  }

  // how many extra bytes required to ensure can find an aligned pointer?
  size_t toAllocate = size + (al - alignof(std::max_align_t)) - 1;
  toAllocate += sizeof(uintptr_t);  // to store the original pointer

  uintptr_t allocated = malloc(toAllocate);

  // ensure to save space for the back-pointer.
  uintptr_t alignedPtr = allocated + sizeof(uintptr_t);

  size_t mask = al  1; // alignment mask ... the bits that must be zero
  uintptr_t alignedPtr =
      (allocated & mask) == 0 ?
      allocated :
      (allocated & (~mask)) + al;

  // save the original pointer just before the pointer value returned to caller
  static_assert(alignof(uintptr_t) <= alignof(std::max_align_t), "" );
  uintptr_t* storeOriginalPointerAt = (uintptr_t)(alignedPtr - sizeof(uintptr_t));
  *storeOriginalPointerAt = allocated;

  return alignedPtr;
}
*/

  void* operator new[](size_t size, std::align_val_t al) {
    badAlloc("Alignment aware new/delete operators, a C++ 17 feaure, are not supported by this core or any other AVR cores at this point in time");
    (void) al;
    return malloc(size);
  }
  void  operator delete  (void* ptr, std::align_val_t al) noexcept {
    badAlloc("Alignment aware new/delete operators, a C++ 17 feaure, are not supported by this core or any other AVR cores at this point in time");
    (void) al;
    free(ptr);
  }
  /*
  void operator delete (void* ptr, std::align_val_t al) noexcept {

    if (al <= STDCPP_DEFAULT_NEW_ALIGNMENT) {
        free(ptr);
        return;
    }

    // Get the original pointer value from just prior to the provided pointer.
    uintptr_t aligned = ptr;
    uintptr_t tmp = aligned - sizeof(uintptr_t); // go back 2 bytes
    uintptr_t original = *((uintptr_t *)tmp); // extract the original pointer

    // sanity check the original value... for iot, infinite loop recommended
    // as it's better than memory corruption!rhy
    size_t overhead = (al - STDCPP_DEFAULT_NEW_ALIGNMENT) + sizeof(uintptr_t);
    assert(original < aligned);
    assert(original + overhead <= aligned);

    // and finally free the original memory
    free(original);
  }
  */
  void  operator delete[](void* ptr, std::align_val_t al) noexcept {
    badAlloc("Alignment aware new/delete operators, a C++ 17 feaure, are not supported by this core or any other AVR cores at this point in time");
    (void) al;
    free(ptr);
  }
  void  operator delete  (void* ptr, size_t size, std::align_val_t al) noexcept{
    badAlloc("Alignment aware new/delete operators, a C++ 17 feaure, are not supported by this core or any other AVR cores at this point in time");
    (void) al;
    (void) size;
    free(ptr);
  }
  void  operator delete[](void* ptr, size_t size, std::align_val_t al) noexcept {
    badAlloc("Overaligned allocation/deallocation is a C++ 17 feature, and is not supported by this or other AVR cores at this point in time");
    (void) al;
    (void) size;
    free(ptr);
  }

#endif
