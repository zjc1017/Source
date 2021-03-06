/*********************************/
#ifndef _Z_CODEC_H__
#define _Z_CODEC_H__
#include "platform.h"
//#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define ENCODE_WITH_METHOD(_method, _val, _buf, _buf_len, _tmp)	\
  do {								\
    _tmp = (_method)((_val), (_buf), (_buf_len));		\
    if (_tmp < 0) {						\
      return _tmp;						\
    }								\
    _buf     += _tmp;						\
    _buf_len -= _tmp;						\
  } while (0)

#define DECODE_WITH_METHOD(_method, _val, _buf, _buf_len, _tmp)	\
  do {								\
    _tmp = (_method)((_val), (_buf), (_buf_len));		\
    if (_tmp < 0) {						\
      return _tmp;						\
    }								\
    _buf     += _tmp;						\
    _buf_len -= _tmp;						\
  } while (0)

///////////////////////////////////////////////////////////////
// common encoding functions
// string (c style, ending with '\0')
// prototype
int z_encode_string(const char *val, char *buf, uint32_t buf_len);
int z_decode_string(char **val, char *buf, uint32_t buf_len);
uint32_t z_getlen_string(const char *val);

// byte(integer8)
int z_encode_byte(char val, char *buf, uint32_t buf_len);
int z_decode_byte(char *val, char *buf, uint32_t buf_len);
uint32_t z_getlen_byte(void);
uint32_t z_getlen_binary(void);
uint32_t z_getlen_integer16(void);
uint32_t z_getlen_integer32(void);
// integer16
int z_encode_integer16(uint16_t val, char *buf, uint32_t buf_len);
int z_decode_integer16(uint16_t *val, char *buf, uint32_t buf_len);

// integer32
int z_encode_integer32(uint32_t val, char *buf, uint32_t buf_len);
int z_decode_integer32(uint32_t *val, char *buf, uint32_t buf_len);

int z_encode_binary(char *val,uint32_t val_len,char *buf,uint32_t buf_len);
int z_decode_binary(char *val,uint32_t val_len,char *buf,uint32_t buf_len);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _z_codec_h__

