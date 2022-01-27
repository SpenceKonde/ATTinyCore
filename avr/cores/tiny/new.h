/* New version (c) Spence Konde 2021
 * This file is part of megaTinyCore, DxCore, and ATTinyCore.
 * This is free software (released under the LGPL 2.1). For more
 * information, see the License.md file included with this core.
 */

/*
  C++11 needed some new new and delete operators.
  Since C++14, also needed two more delete variants taking a size argument.
    Said size argument is ignored (cast to void), but on some architectures
    it is an important way to give hints to the memory allocator for greater
    efficiency.
  Since C++17, there's four more each for new / delete, to support allocation
    of objects with alignment greater than __STDCPP_DEFAULT_NEW_ALIGNMENT__.
    This has two major differences from the C++14 sized deallocaton.
    1. It cannot be simply ignored - they presumably wanted alignment for
    a reason; this implementation is not impossible, but nor is it trivial.
    2. If code used on C++14 or earlier is asking for the alignment aware
    versions of these operators, either it doesn't actually need it and
    shouldn't be, or it was sheer happenstance that whatver address it
    was getting worked within it's assumoptions.
  Unless and until official cores or other cores in widespread use add support
    for the alignment aware operators on AVRs, we have no plans to. While we
    do not hesitate to offer users new features to take advantage of hardware
    capabilities not present on older devices, this does not extend to new
    compiler features which require modifications to code to take advantage
    of. We have candidates for overaligned new and delete though, for when/if
    they are required.
    -Spence, 9/25/2021
 */

#ifndef NEW_H
  #define NEW_H

  #include <stdlib.h>
                                                /* C++11 and later                */
  void* operator new   (size_t size             /* return malloc(size);           */);
  void* operator new[] (size_t size             /* return malloc(size);           */);
  void* operator new   (size_t size, void * ptr /* (void size); return ptr;       */) noexcept;
  void* operator new[] (size_t size, void * ptr /* (void size); return ptr;       */) noexcept;
  void operator delete   (void* ptr             /* free(ptr);                     */) noexcept;
  void operator delete[] (void* ptr             /* free(ptr);                     */) noexcept;

  #if (__cpp_sized_deallocation >= 201309L)       /* C++14 and later only         */
    void operator delete   (void* ptr, size_t size /* (void)size; return ptr;     */) noexcept;
    void operator delete[] (void* ptr, size_t size /* (void)size; return ptr;     */) noexcept;
  #endif

  #if (__cpp_aligned_new >= 201606L) /* C++17 and later only                      */
    /* These are unsupported and just stubs that generate a more helpful error    */
    /* If it turns out that we do need them, we have a candidate for these        */
    void* operator new     (size_t size, std::align_val_t al                        );
    void  operator delete  (void* ptr,/* (void)al; return ptr; */std::align_val_t al) noexcept;
    /* Not actually supported, these stubs just generate a more helpful error   */
    void* operator new[]   (size_t size, std::align_val_t al                        );
    void  operator delete[](void* ptr,/* (void)al; return ptr; */std::align_val_t al) noexcept;
    void  operator delete  (void* ptr, size_t size,/* above +  */std::align_val_t al) noexcept;
    void  operator delete[](void* ptr, size_t size,/* (void)al;*/std::align_val_t al) noexcept;
  #endif
#endif
