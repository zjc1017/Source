#ifndef _ZB_MSG_H__
#define _ZB_MSG_H__
//#include "stdint.h"
#include "z_codec.h"
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum {
	
	ZB_ID_REG_REQ = 0x01,
	ZB_ID_REG_RSP = 0x81,

	ZB_ID_GET_REQ = 0x02,
	ZB_ID_GET_RSP = 0x82,

	ZB_ID_SET_REQ = 0x03,
	ZB_ID_SET_RSP = 0x83,

	ZB_ID_ANN_REG = 0x06,
	ZB_ID_ANN_RSP = 0x86,
  
  ZB_ID_HEARTBEAT_REQ = 0xFE,
  ZB_ID_HEARTBEAT_RSP = 0xFF,
  

	// to be continued
};
///////////////////////////////////////
// header
struct zb_header {
  char     syn[8];
  uint8_t     ver;
  uint16_t    len; 			/* header is not included */
  uint8_t     cmd;
  uint16_t    addr;
};

struct zb_item_id_list {
	uint8_t count;
	uint8_t *id;
};

struct zb_item_pair {
	uint8_t id;
	uint16_t val;
};

struct zb_item_pair_list {
	uint8_t count;
	struct zb_item_pair *pairs;
};

//register
#define len_ieee_addr 8;
  struct zb_reg_req {
	struct zb_header hdr;//14
	uint8_t  ieee_addr[8];//8
	uint16_t dev_type;//2
};

// get
struct zb_get_req {
	struct zb_header hdr;
	struct zb_item_id_list ids;
};

struct zb_get_rsp {
	struct zb_header hdr;
	struct zb_item_pair_list pairs;
};

// set
struct zb_set_rsp {
	struct zb_header hdr;
	uint8_t status;
};

struct zb_set_req {
	struct zb_header hdr;
	struct zb_item_pair_list pairs;
};
//announce
struct item_desc{
    uint8_t id ;
    char name[10];
    char desc[10];
    uint8_t type;
    const char *form;
};
struct zb_ann_reg{
    struct zb_header hdr;
    uint8_t cnt;
    struct item_desc item;
};
//prototype
int zb_encode_header(struct zb_header *val, char *buf, uint32_t buf_len);
int zb_decode_header(struct zb_header *val, char *buf, uint32_t buf_len);

int zb_decode_id_list(struct zb_item_id_list *val, char *buf, uint32_t buf_len);
int zb_encode_id_list(struct zb_item_id_list *val, char *buf, uint32_t buf_len);

uint32_t zb_getlen_header(struct zb_header *val);
uint32_t zb_getlen_ann_reg(struct zb_ann_reg *val);

// get_req
int zb_encode_get_req(struct zb_get_req *msg, char *buf, uint32_t buf_len);
int zb_encode_get_rsp(struct zb_get_rsp *val, char *buf, uint32_t buf_len);

int zb_encode_pair_list(struct zb_item_pair_list *val, char *buf, uint32_t buf_len);
uint32_t zb_getlen_item_pair(void);
uint32_t zb_getlen_pair_list(struct zb_item_pair_list *list);

uint32_t zb_getlen_get_req(struct zb_get_req *val);
uint32_t zb_getlen_get_rsp(struct zb_get_rsp *val);


int zb_decode_get_req(struct zb_get_req *val, char *buf, uint32_t buf_len);
uint32_t zb_getlen_id_list(struct zb_item_id_list *ids);


uint32_t zb_getlen_reg_req(struct zb_reg_req *val);
int zb_decode_pair_list(struct zb_item_pair_list *val, char *buf, uint32_t buf_len);

int zb_test(uint8_t *serial_data,uint8_t value,uint8_t len);

int zb_encode_reg_req(struct zb_reg_req *val, char *buf, uint32_t buf_len);

//int zb_encode_set_req(struct zb_set_req *msg char *buf,uint32_t buf_len);
int zb_decode_set_req(struct zb_set_req *val, char *buf, uint32_t buf_len);
int zb_encode_set_rsp(struct zb_set_rsp *val, char *buf, uint32_t buf_len);

int zb_decode_ann_reg(struct zb_ann_reg *val, char *buf, uint32_t buf_len);
int zb_encode_ann_reg(struct zb_ann_reg *val, char *buf, uint32_t buf_len);

uint32_t zb_getlen_status(void);
uint32_t zb_getlen_set_rsp(struct zb_set_rsp *val);
uint32_t z_encode_status(char val, char *buf, uint32_t buf_len);
int zb_encode_desc_item(struct item_desc *val, char *buf, uint32_t buf_len);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // _ZB_MSG_H__


