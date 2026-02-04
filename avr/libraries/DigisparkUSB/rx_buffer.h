#ifndef __rx_buffer_h__
#define __rx_buffer_h__

#ifdef __cplusplus
extern "C"{
#endif 
  uchar rx_buffer[8]; // Buffer 8 bytes
  extern int rx_read_offset; // = -1;
  extern int rx_write_offset; // = 0;
#ifdef __cplusplus
} // extern "C"
#endif

#endif // __rx_buffer_h__
