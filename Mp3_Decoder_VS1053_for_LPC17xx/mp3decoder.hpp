#ifndef MP3DECODER_H
#define MP3DECODER_H

//SCI read/write instruction opcode 
#define READ          0x03
#define WRITE         0x02
//SCI Registers VS1053 datasheet pg 37
#define  MODE         0x00 
#define  STATUS       0x01 
#define  BASS         0x02 
#define  CLOCKF       0x03 
#define  DECODE_TIME  0x04
#define  AUDATA       0x05 //sample rate
#define  VOL          0x0B

class MP3DECODER {

private:

public:
  MP3DECODER();
  ~MP3DECODER();
  bool playing;
  bool init();
  bool Unselect_C(void);
  bool Select_C(void);
  bool Select_DC(void);
  bool Unselect_DC(void);
  bool Data_Req_Stat();
  void SCI_en(void);
  void SCI_disable(void);
  void SDI_en(void);
  void SDI_disable(void);
  void SW_RST();
  void VolSet(uint8_t vol);
  void sci_writeReg(uint16_t data, uint8_t addr);
  uint16_t sci_readReg(uint8_t addr);
  void sdi_write(uint8_t data);
};

#endif
