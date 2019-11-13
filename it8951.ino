#include <SPI.h>

#define MISO 19
#define MOSI 18
#define SCK 5
#define CS 12

#define RESET 16
#define HRDY 17

#define bcm2835_gpio_write digitalWrite
#define bcm2835_spi_transfer SPI.transfer
#define bcm2835_gpio_lev digitalRead

//Built in I80 Command Code
#define IT8951_TCON_SYS_RUN      0x0001
#define IT8951_TCON_STANDBY      0x0002
#define IT8951_TCON_SLEEP        0x0003
#define IT8951_TCON_REG_RD       0x0010
#define IT8951_TCON_REG_WR       0x0011
#define IT8951_TCON_MEM_BST_RD_T 0x0012
#define IT8951_TCON_MEM_BST_RD_S 0x0013
#define IT8951_TCON_MEM_BST_WR   0x0014
#define IT8951_TCON_MEM_BST_END  0x0015
#define IT8951_TCON_LD_IMG       0x0020
#define IT8951_TCON_LD_IMG_AREA  0x0021
#define IT8951_TCON_LD_IMG_END   0x0022

//I80 User defined command code
#define USDEF_I80_CMD_DPY_AREA     0x0034
#define USDEF_I80_CMD_GET_DEV_INFO 0x0302
#define USDEF_I80_CMD_DPY_BUF_AREA 0x0037
//Panel
#define IT8951_PANEL_WIDTH   1024 //it Get Device information
#define IT8951_PANEL_HEIGHT   758

//Rotate mode
#define IT8951_ROTATE_0     0
#define IT8951_ROTATE_90    1
#define IT8951_ROTATE_180   2
#define IT8951_ROTATE_270   3

//Pixel mode , BPP - Bit per Pixel
#define IT8951_2BPP   0
#define IT8951_3BPP   1
#define IT8951_4BPP   2
#define IT8951_8BPP   3

//Waveform Mode
#define IT8951_MODE_0   0
#define IT8951_MODE_1   1
#define IT8951_MODE_2   2
#define IT8951_MODE_3   3
#define IT8951_MODE_4   4
//Endian Type
#define IT8951_LDIMG_L_ENDIAN   0
#define IT8951_LDIMG_B_ENDIAN   1
//Auto LUT
#define IT8951_DIS_AUTO_LUT   0
#define IT8951_EN_AUTO_LUT    1
//LUT Engine Status
#define IT8951_ALL_LUTE_BUSY 0xFFFF

//-----------------------------------------------------------------------
// IT8951 TCon Registers defines
//-----------------------------------------------------------------------
//Register Base Address
#define DISPLAY_REG_BASE 0x1000               //Register RW access for I80 only
//Base Address of Basic LUT Registers
#define LUT0EWHR  (DISPLAY_REG_BASE + 0x00)   //LUT0 Engine Width Height Reg
#define LUT0XYR   (DISPLAY_REG_BASE + 0x40)   //LUT0 XY Reg
#define LUT0BADDR (DISPLAY_REG_BASE + 0x80)   //LUT0 Base Address Reg
#define LUT0MFN   (DISPLAY_REG_BASE + 0xC0)   //LUT0 Mode and Frame number Reg
#define LUT01AF   (DISPLAY_REG_BASE + 0x114)  //LUT0 and LUT1 Active Flag Reg
//Update Parameter Setting Register
#define UP0SR (DISPLAY_REG_BASE + 0x134)      //Update Parameter0 Setting Reg

#define UP1SR     (DISPLAY_REG_BASE + 0x138)  //Update Parameter1 Setting Reg
#define LUT0ABFRV (DISPLAY_REG_BASE + 0x13C)  //LUT0 Alpha blend and Fill rectangle Value
#define UPBBADDR  (DISPLAY_REG_BASE + 0x17C)  //Update Buffer Base Address
#define LUT0IMXY  (DISPLAY_REG_BASE + 0x180)  //LUT0 Image buffer X/Y offset Reg
#define LUTAFSR   (DISPLAY_REG_BASE + 0x224)  //LUT Status Reg (status of All LUT Engines)

#define BGVR      (DISPLAY_REG_BASE + 0x250)  //Bitmap (1bpp) image color table
//-------System Registers----------------
#define SYS_REG_BASE 0x0000

//Address of System Registers
#define I80CPCR (SYS_REG_BASE + 0x04)
//-------Memory Converter Registers----------------
#define MCSR_BASE_ADDR 0x0200
#define MCSR (MCSR_BASE_ADDR  + 0x0000)
#define LISAR (MCSR_BASE_ADDR + 0x0008)

typedef struct
{
    uint16_t usPanelW;
    uint16_t usPanelH;
    uint16_t usImgBufAddrL;
    uint16_t usImgBufAddrH;
    uint16_t usFWVersion[8];   //16 Bytes String
    uint16_t usLUTVersion[8];   //16 Bytes String
}IT8951DevInfo;

typedef struct IT8951AreaImgInfo
{
    uint16_t usX;
    uint16_t usY;
    uint16_t usWidth;
    uint16_t usHeight;
}IT8951AreaImgInfo;

typedef struct IT8951LdImgInfo
{
    uint16_t usEndianType; //little or Big Endian
    uint16_t usPixelFormat; //bpp
    uint16_t usRotate; //Rotate mode
    uint32_t ulStartFBAddr; //Start address of source Frame buffer
    uint32_t ulImgBufBaseAddr;//Base address of target image buffer
    
}IT8951LdImgInfo;

IT8951DevInfo gstI80DevInfo;
uint8_t* gpFrameBuf;
uint32_t gulImgBufAddr;

#define SYS_REG_BASE 0x0000
#define I80CPCR (SYS_REG_BASE + 0x04)

#define USDEF_I80_CMD_GET_DEV_INFO 0x0302

#define IT8951_TCON_REG_WR       0x0011

uint8_t IT8951_Init()
{
  //bcm2835_spi_begin();
  SPI.begin(SCK, MISO, MOSI, CS);
  //SPI.begin();
  //bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);    //default
  //SPI.setBitOrder(MSBFIRST);
  //bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   //default
  //SPI.setDataMode(SPI_MODE0);
  //bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);    //default
  //SPI.setClockDivider(SPI_CLOCK_DIV8);

  // alternativ:
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));

  //bcm2835_gpio_fsel(CS, BCM2835_GPIO_FSEL_OUTP);
  pinMode(CS, OUTPUT);
  //bcm2835_gpio_fsel(HRDY, BCM2835_GPIO_FSEL_INPT);
  pinMode(HRDY, INPUT);
  //bcm2835_gpio_fsel(RESET, BCM2835_GPIO_FSEL_OUTP);
  pinMode(RESET, OUTPUT);
  
  bcm2835_gpio_write(CS, HIGH);

  printf("****** IT8951 ******\n");

  bcm2835_gpio_write(RESET, LOW);
  //bcm2835_delay(100);
  delay(1000); //NOTE 100ms like in specs is not enough
  bcm2835_gpio_write(RESET, HIGH);

  //Get Device Info
  GetIT8951SystemInfo(&gstI80DevInfo);

  Serial.print("W: ");
  Serial.print(gstI80DevInfo.usPanelW);
  Serial.print(" H: ");
  Serial.println(gstI80DevInfo.usPanelH);
  if (!gstI80DevInfo.usPanelW || !gstI80DevInfo.usPanelH) {
    return 1;
  }

  gulImgBufAddr = gstI80DevInfo.usImgBufAddrL | ((uint32_t)gstI80DevInfo.usImgBufAddrH << 16);

  //Set to Enable I80 Packed mode
  IT8951WriteReg(I80CPCR, 0x0001);

  //SPI.endTransaction();

  return 0;
}

/*
extern IT8951DevInfo gstI80DevInfo;
extern uint8_t* gpFrameBuf;       //Host Source Frame buffer
extern uint32_t gulImgBufAddr;    //IT8951 Image buffer address
*/

//-----------------------------------------------------------
//Host controller function 1---Wait for host data Bus Ready
//-----------------------------------------------------------
void LCDWaitForReady()
{
  uint8_t ulData = bcm2835_gpio_lev(HRDY);
  while(ulData == 0)
  {
    ulData = bcm2835_gpio_lev(HRDY);
  }
}

//-----------------------------------------------------------
//Host controller function 2---Write command code to host data Bus
//-----------------------------------------------------------
void LCDWriteCmdCode(uint16_t usCmdCode)
{
  //Set Preamble for Write Command
  uint16_t wPreamble = 0x6000; 
  
  LCDWaitForReady();  

  bcm2835_gpio_write(CS,LOW);
  
  bcm2835_spi_transfer(wPreamble>>8);
  bcm2835_spi_transfer(wPreamble);
  
  LCDWaitForReady();  
  
  bcm2835_spi_transfer(usCmdCode>>8);
  bcm2835_spi_transfer(usCmdCode);
  
  bcm2835_gpio_write(CS,HIGH); 
}

//-----------------------------------------------------------
//Host controller function 3---Write Data to host data Bus
//-----------------------------------------------------------
void LCDWriteData(uint16_t usData)
{
  //Set Preamble for Write Data
  uint16_t wPreamble  = 0x0000;

  LCDWaitForReady();

  bcm2835_gpio_write(CS,LOW);

  bcm2835_spi_transfer(wPreamble>>8);
  bcm2835_spi_transfer(wPreamble);
  
  LCDWaitForReady();
      
  bcm2835_spi_transfer(usData>>8);
  bcm2835_spi_transfer(usData);
  
  bcm2835_gpio_write(CS,HIGH); 
}

void LCDWriteNData(uint16_t* pwBuf, uint32_t ulSizeWordCnt)
{
  uint32_t i;

  uint16_t wPreamble  = 0x0000;

  LCDWaitForReady();

  bcm2835_gpio_write(CS,LOW);
  
  bcm2835_spi_transfer(wPreamble>>8);
  bcm2835_spi_transfer(wPreamble);
  
  LCDWaitForReady();

  for(i=0;i<ulSizeWordCnt;i++)
  {
    bcm2835_spi_transfer(pwBuf[i]>>8);
    bcm2835_spi_transfer(pwBuf[i]);
  }
  
  bcm2835_gpio_write(CS,HIGH); 
}  

//-----------------------------------------------------------
//Host controller function 4---Read Data from host data Bus
//-----------------------------------------------------------
uint16_t LCDReadData()
{
  uint16_t wRData; 
  
  uint16_t wPreamble = 0x1000;

  LCDWaitForReady();

  bcm2835_gpio_write(CS,LOW);
    
  bcm2835_spi_transfer(wPreamble>>8);
  bcm2835_spi_transfer(wPreamble);

  LCDWaitForReady();
  
  wRData=bcm2835_spi_transfer(0x00);//dummy
  wRData=bcm2835_spi_transfer(0x00);//dummy
  
  LCDWaitForReady();
  
  wRData = bcm2835_spi_transfer(0x00)<<8;
  wRData |= bcm2835_spi_transfer(0x00);
    
  bcm2835_gpio_write(CS,HIGH); 
    
  return wRData;
}

//-----------------------------------------------------------
//  Read Burst N words Data
//-----------------------------------------------------------
void LCDReadNData(uint16_t* pwBuf, uint32_t ulSizeWordCnt)
{
  uint32_t i;
  
  uint16_t wPreamble = 0x1000;

  LCDWaitForReady();
  
  bcm2835_gpio_write(CS,LOW);

  bcm2835_spi_transfer(wPreamble>>8);
  bcm2835_spi_transfer(wPreamble);
  
  LCDWaitForReady();
  
  pwBuf[0]=bcm2835_spi_transfer(0x00);//dummy
  pwBuf[0]=bcm2835_spi_transfer(0x00);//dummy
  
  LCDWaitForReady();
  
  for(i=0;i<ulSizeWordCnt;i++)
  {
    pwBuf[i] = bcm2835_spi_transfer(0x00)<<8;
    pwBuf[i] |= bcm2835_spi_transfer(0x00);
  }
  
  bcm2835_gpio_write(CS,HIGH); 
}

//-----------------------------------------------------------
//Host controller function 5---Write command to host data Bus with aruments
//-----------------------------------------------------------
void LCDSendCmdArg(uint16_t usCmdCode,uint16_t* pArg, uint16_t usNumArg)
{
     uint16_t i;
     //Send Cmd code
     LCDWriteCmdCode(usCmdCode);
     //Send Data
     for(i=0;i<usNumArg;i++)
     {
         LCDWriteData(pArg[i]);
     }
}

//-----------------------------------------------------------
//Host Cmd 1---SYS_RUN
//-----------------------------------------------------------
void IT8951SystemRun()
{
    LCDWriteCmdCode(IT8951_TCON_SYS_RUN);
}

//-----------------------------------------------------------
//Host Cmd 2---STANDBY
//-----------------------------------------------------------
void IT8951StandBy()
{
    LCDWriteCmdCode(IT8951_TCON_STANDBY);
}

//-----------------------------------------------------------
//Host Cmd 3---SLEEP
//-----------------------------------------------------------
void IT8951Sleep()
{
    LCDWriteCmdCode(IT8951_TCON_SLEEP);
}

//-----------------------------------------------------------
//Host Cmd 4---REG_RD
//-----------------------------------------------------------
uint16_t IT8951ReadReg(uint16_t usRegAddr)
{
  uint16_t usData;
  
  //Send Cmd and Register Address
  LCDWriteCmdCode(IT8951_TCON_REG_RD);
  LCDWriteData(usRegAddr);
  //Read data from Host Data bus
  usData = LCDReadData();
  return usData;
}
//-----------------------------------------------------------
//Host Cmd 5---REG_WR
//-----------------------------------------------------------
void IT8951WriteReg(uint16_t usRegAddr,uint16_t usValue)
{
  //Send Cmd , Register Address and Write Value
  LCDWriteCmdCode(IT8951_TCON_REG_WR);
  LCDWriteData(usRegAddr);
  LCDWriteData(usValue);
}

//-----------------------------------------------------------
//Host Cmd 6---MEM_BST_RD_T
//-----------------------------------------------------------
void IT8951MemBurstReadTrigger(uint32_t ulMemAddr , uint32_t ulReadSize)
{
    uint16_t usArg[4];
    //Setting Arguments for Memory Burst Read
    usArg[0] = (uint16_t)(ulMemAddr & 0x0000FFFF); //addr[15:0]
    usArg[1] = (uint16_t)( (ulMemAddr >> 16) & 0x0000FFFF ); //addr[25:16]
    usArg[2] = (uint16_t)(ulReadSize & 0x0000FFFF); //Cnt[15:0]
    usArg[3] = (uint16_t)( (ulReadSize >> 16) & 0x0000FFFF ); //Cnt[25:16]
    //Send Cmd and Arg
    LCDSendCmdArg(IT8951_TCON_MEM_BST_RD_T , usArg , 4);
}
//-----------------------------------------------------------
//Host Cmd 7---MEM_BST_RD_S
//-----------------------------------------------------------
void IT8951MemBurstReadStart()
{
    LCDWriteCmdCode(IT8951_TCON_MEM_BST_RD_S);
}
//-----------------------------------------------------------
//Host Cmd 8---MEM_BST_WR
//-----------------------------------------------------------
void IT8951MemBurstWrite(uint32_t ulMemAddr , uint32_t ulWriteSize)
{
    uint16_t usArg[4];
    //Setting Arguments for Memory Burst Write
    usArg[0] = (uint16_t)(ulMemAddr & 0x0000FFFF); //addr[15:0]
    usArg[1] = (uint16_t)( (ulMemAddr >> 16) & 0x0000FFFF ); //addr[25:16]
    usArg[2] = (uint16_t)(ulWriteSize & 0x0000FFFF); //Cnt[15:0]
    usArg[3] = (uint16_t)( (ulWriteSize >> 16) & 0x0000FFFF ); //Cnt[25:16]
    //Send Cmd and Arg
    LCDSendCmdArg(IT8951_TCON_MEM_BST_WR , usArg , 4);
}
//-----------------------------------------------------------
//Host Cmd 9---MEM_BST_END
//-----------------------------------------------------------
void IT8951MemBurstEnd(void)
{
    LCDWriteCmdCode(IT8951_TCON_MEM_BST_END);
}

//-----------------------------------------------------------
//Example of Memory Burst Write
//-----------------------------------------------------------
// ****************************************************************************************
// Function name: IT8951MemBurstWriteProc( )
//
// Description:
//   IT8951 Burst Write procedure
//      
// Arguments:
//      uint32_t ulMemAddr: IT8951 Memory Target Address
//      uint32_t ulWriteSize: Write Size (Unit: Word)
//      uint8_t* pDestBuf - Buffer of Sent data
// Return Values:
//   NULL.
// Note:
//
// ****************************************************************************************
void IT8951MemBurstWriteProc(uint32_t ulMemAddr , uint32_t ulWriteSize, uint16_t* pSrcBuf )
{
    
    uint32_t i;
 
    //Send Burst Write Start Cmd and Args
    IT8951MemBurstWrite(ulMemAddr , ulWriteSize);
 
    //Burst Write Data
    for(i=0;i<ulWriteSize;i++)
    {
        LCDWriteData(pSrcBuf[i]);
    }
 
    //Send Burst End Cmd
    IT8951MemBurstEnd();
}

// ****************************************************************************************
// Function name: IT8951MemBurstReadProc( )
//
// Description:
//   IT8951 Burst Read procedure
//      
// Arguments:
//      uint32_t ulMemAddr: IT8951 Read Memory Address
//      uint32_t ulReadSize: Read Size (Unit: Word)
//      uint8_t* pDestBuf - Buffer for storing Read data
// Return Values:
//   NULL.
// Note:
//
// ****************************************************************************************
void IT8951MemBurstReadProc(uint32_t ulMemAddr , uint32_t ulReadSize, uint16_t* pDestBuf )
{
    //Send Burst Read Start Cmd and Args
    IT8951MemBurstReadTrigger(ulMemAddr , ulReadSize);
          
    //Burst Read Fire
    IT8951MemBurstReadStart();
    
    //Burst Read Request for SPI interface only
    LCDReadNData(pDestBuf, ulReadSize);

    //Send Burst End Cmd
    IT8951MemBurstEnd(); //the same with IT8951MemBurstEnd()
}

//-----------------------------------------------------------
//Host Cmd 10---LD_IMG
//-----------------------------------------------------------
void IT8951LoadImgStart(IT8951LdImgInfo* pstLdImgInfo)
{
    uint16_t usArg;
    //Setting Argument for Load image start
    usArg = (pstLdImgInfo->usEndianType << 8 )
    |(pstLdImgInfo->usPixelFormat << 4)
    |(pstLdImgInfo->usRotate);
    //Send Cmd
    LCDWriteCmdCode(IT8951_TCON_LD_IMG);
    //Send Arg
    LCDWriteData(usArg);
}
//-----------------------------------------------------------
//Host Cmd 11---LD_IMG_AREA
//-----------------------------------------------------------
void IT8951LoadImgAreaStart(IT8951LdImgInfo* pstLdImgInfo ,IT8951AreaImgInfo* pstAreaImgInfo)
{
    uint16_t usArg[5];
    //Setting Argument for Load image start
    usArg[0] = (pstLdImgInfo->usEndianType << 8 )
    |(pstLdImgInfo->usPixelFormat << 4)
    |(pstLdImgInfo->usRotate);
    usArg[1] = pstAreaImgInfo->usX;
    usArg[2] = pstAreaImgInfo->usY;
    usArg[3] = pstAreaImgInfo->usWidth;
    usArg[4] = pstAreaImgInfo->usHeight;
    //Send Cmd and Args
    LCDSendCmdArg(IT8951_TCON_LD_IMG_AREA , usArg , 5);
}
//-----------------------------------------------------------
//Host Cmd 12---LD_IMG_END
//-----------------------------------------------------------
void IT8951LoadImgEnd(void)
{
    LCDWriteCmdCode(IT8951_TCON_LD_IMG_END);
}

void GetIT8951SystemInfo(void* pBuf)
{
  uint16_t* pusWord = (uint16_t*)pBuf;
  IT8951DevInfo* pstDevInfo;

  //Send I80 CMD
  LCDWriteCmdCode(USDEF_I80_CMD_GET_DEV_INFO);
 
  //Burst Read Request for SPI interface only
  LCDReadNData(pusWord, sizeof(IT8951DevInfo)/2);//Polling HRDY for each words(2-bytes) if possible
  
  //Show Device information of IT8951
  pstDevInfo = (IT8951DevInfo*)pBuf;
  printf("Panel(W,H) = (%d,%d)\r\n",
  pstDevInfo->usPanelW, pstDevInfo->usPanelH );
  printf("Image Buffer Address = %X\r\n",
  pstDevInfo->usImgBufAddrL | (pstDevInfo->usImgBufAddrH << 16));
  //Show Firmware and LUT Version
  printf("FW Version = %s\r\n", (uint8_t*)pstDevInfo->usFWVersion);
  printf("LUT Version = %s\r\n", (uint8_t*)pstDevInfo->usLUTVersion);
}

//-----------------------------------------------------------
//Initial function 2---Set Image buffer base address
//-----------------------------------------------------------
void IT8951SetImgBufBaseAddr(uint32_t ulImgBufAddr)
{
  uint16_t usWordH = (uint16_t)((ulImgBufAddr >> 16) & 0x0000FFFF);
  uint16_t usWordL = (uint16_t)( ulImgBufAddr & 0x0000FFFF);
  //Write LISAR Reg
  IT8951WriteReg(LISAR + 2 ,usWordH);
  IT8951WriteReg(LISAR ,usWordL);
}

//-----------------------------------------------------------
// 3.6. Display Functions
//-----------------------------------------------------------

//-----------------------------------------------------------
//Display function 1---Wait for LUT Engine Finish
//                     Polling Display Engine Ready by LUTNo
//-----------------------------------------------------------
void IT8951WaitForDisplayReady()
{
  //Check IT8951 Register LUTAFSR => NonZero Busy, 0 - Free
  while(IT8951ReadReg(LUTAFSR));
}

//-----------------------------------------------------------
//Display function 2---Load Image Area process
//-----------------------------------------------------------
void IT8951HostAreaPackedPixelWrite(IT8951LdImgInfo* pstLdImgInfo,IT8951AreaImgInfo* pstAreaImgInfo)
{
  uint32_t i,j;
  //Source buffer address of Host
  uint16_t* pusFrameBuf = (uint16_t*)pstLdImgInfo->ulStartFBAddr;

  //Set Image buffer(IT8951) Base address
  IT8951SetImgBufBaseAddr(pstLdImgInfo->ulImgBufBaseAddr);
  //Send Load Image start Cmd
  IT8951LoadImgAreaStart(pstLdImgInfo , pstAreaImgInfo);
  //Host Write Data
  for(j=0;j< pstAreaImgInfo->usHeight;j++)
  {
     for(i=0;i< pstAreaImgInfo->usWidth/2;i++)
      {
          //Write a Word(2-Bytes) for each time
          LCDWriteData(*pusFrameBuf);
          pusFrameBuf++;
      }
  }
  //Send Load Img End Command
  IT8951LoadImgEnd();
}

//-----------------------------------------------------------
//Display functions 3---Application for Display panel Area
//-----------------------------------------------------------
void IT8951DisplayArea(uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, uint16_t usDpyMode)
{
  //Send I80 Display Command (User defined command of IT8951)
  LCDWriteCmdCode(USDEF_I80_CMD_DPY_AREA); //0x0034
  //Write arguments
  LCDWriteData(usX);
  LCDWriteData(usY);
  LCDWriteData(usW);
  LCDWriteData(usH);
  LCDWriteData(usDpyMode);
}

//Display Area with bitmap on EPD
//-----------------------------------------------------------
// Display Function 4---for Display Area for 1-bpp mode format
//   the bitmap(1bpp) mode will be enable when Display
//   and restore to Default setting (disable) after displaying finished
//-----------------------------------------------------------
void IT8951DisplayArea1bpp(uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, uint16_t usDpyMode, uint8_t ucBGGrayVal, uint8_t ucFGGrayVal)
{
    //Set Display mode to 1 bpp mode - Set 0x18001138 Bit[18](0x1800113A Bit[2])to 1
    IT8951WriteReg(UP1SR+2, IT8951ReadReg(UP1SR+2) | (1<<2));
    
    //Set BitMap color table 0 and 1 , => Set Register[0x18001250]:
    //Bit[7:0]: ForeGround Color(G0~G15)  for 1
    //Bit[15:8]:Background Color(G0~G15)  for 0
    IT8951WriteReg(BGVR, (ucBGGrayVal<<8) | ucFGGrayVal);
    
    //Display
    IT8951DisplayArea( usX, usY, usW, usH, usDpyMode);
    IT8951WaitForDisplayReady();
    
    //Restore to normal mode
    IT8951WriteReg(UP1SR+2, IT8951ReadReg(UP1SR+2) & ~(1<<2));
}

//-------------------------------------------------------------------------------------------------------------
//  Command - 0x0037 for Display Base addr by User 
//  uint32_t ulDpyBufAddr - Host programmer need to indicate the Image buffer address of IT8951
//                                         In current case, there is only one image buffer in IT8951 so far.
//                                         So Please set the Image buffer address you got  in initial stage.
//                                         (gulImgBufAddr by Get device information 0x0302 command)
//
//-------------------------------------------------------------------------------------------------------------
void IT8951DisplayAreaBuf(uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH, uint16_t usDpyMode, uint32_t ulDpyBufAddr)
{
    //Send I80 Display Command (User defined command of IT8951)
    LCDWriteCmdCode(USDEF_I80_CMD_DPY_BUF_AREA); //0x0037
    
    //Write arguments
    LCDWriteData(usX);
    LCDWriteData(usY);
    LCDWriteData(usW);
    LCDWriteData(usH);
    LCDWriteData(usDpyMode);
    LCDWriteData((uint16_t)ulDpyBufAddr);       //Display Buffer Base address[15:0]
    LCDWriteData((uint16_t)(ulDpyBufAddr>>16)); //Display Buffer Base address[26:16]
}

//-----------------------------------------------------------
//Test function 1---Software Initial
//-----------------------------------------------------------
/*uint8_t IT8951_Init()
{
  if (!bcm2835_init()) 
  {
    printf("bcm2835_init error \n");
    return 1;
  }
  
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);    //default
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   //default
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);    //default
  
  bcm2835_gpio_fsel(CS, BCM2835_GPIO_FSEL_OUTP);  
  bcm2835_gpio_fsel(HRDY, BCM2835_GPIO_FSEL_INPT);
  bcm2835_gpio_fsel(RESET, BCM2835_GPIO_FSEL_OUTP);
  
  bcm2835_gpio_write(CS, HIGH);

  printf("****** IT8951 ******\n");
  
  bcm2835_gpio_write(RESET, LOW);
  bcm2835_delay(100);
  bcm2835_gpio_write(RESET, HIGH);

  //Get Device Info
  GetIT8951SystemInfo(&gstI80DevInfo);
  
  gpFrameBuf = malloc(gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH);
  if (!gpFrameBuf)
  {
    perror("malloc error!\n");
    return 1;
  }
  
  gulImgBufAddr = gstI80DevInfo.usImgBufAddrL | (gstI80DevInfo.usImgBufAddrH << 16);
  
  //Set to Enable I80 Packed mode
  IT8951WriteReg(I80CPCR, 0x0001);

  return 0;
}*/

void IT8951_Cancel()
{
  free(gpFrameBuf);

  //bcm2835_spi_end();
  //bcm2835_close();
}


//-----------------------------------------------------------
//Test function 2---Example of Display Flow
//-----------------------------------------------------------
void IT8951DisplayExample()
{
  IT8951LdImgInfo stLdImgInfo;
  IT8951AreaImgInfo stAreaImgInfo;
  
  //Prepare image
  //Write pixel 0xF0(White) to Frame Buffer
  memset(gpFrameBuf, 0xF0, gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH);
  
  //Check TCon is free ? Wait TCon Ready (optional)
  IT8951WaitForDisplayReady();
  
  //--------------------------------------------------------------------------------------------
  //      initial display - Display white only
  //--------------------------------------------------------------------------------------------
  //Load Image and Display
  //Setting Load image information
  stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
  stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
  stLdImgInfo.usPixelFormat    = IT8951_8BPP;
  stLdImgInfo.usRotate         = IT8951_ROTATE_0;
  stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
  //Set Load Area
  stAreaImgInfo.usX      = 0;
  stAreaImgInfo.usY      = 0;
  stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
  stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
  
  //Load Image from Host to IT8951 Image Buffer
  IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
  //Display Area ?V (x,y,w,h) with mode 0 for initial White to clear Panel
  IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 0);
  
  //--------------------------------------------------------------------------------------------
  //      Regular display - Display Any Gray colors with Mode 2 or others
  //--------------------------------------------------------------------------------------------
  //Preparing buffer to All black (8 bpp image)
  //or you can create your image pattern here..
  memset(gpFrameBuf, 0x00, gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH);
   
  IT8951WaitForDisplayReady();
  
  //Setting Load image information
  stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
  stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
  stLdImgInfo.usPixelFormat    = IT8951_8BPP; 
  stLdImgInfo.usRotate         = IT8951_ROTATE_0;
  stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
  //Set Load Area
  stAreaImgInfo.usX      = 0;
  stAreaImgInfo.usY      = 0;
  stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
  stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
  
  //Load Image from Host to IT8951 Image Buffer
  IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
  //Display Area ?V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform 
  IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);
}

void IT8951DisplayExample2()
{
  IT8951LdImgInfo stLdImgInfo;
  IT8951AreaImgInfo stAreaImgInfo;
  
  //--------------------------------------------------------------------------------------------
  //      Regular display - Display Any Gray colors with Mode 2 or others
  //--------------------------------------------------------------------------------------------
  //Preparing buffer to All black (8 bpp image)
  //or you can create your image pattern here..
  memset(gpFrameBuf              , 0x00, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 1, 0x11, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 2, 0x22, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 3, 0x33, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 4, 0x44, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 5, 0x55, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 6, 0x66, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 7, 0x77, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 8, 0x88, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 9, 0x99, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 10, 0xaa, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 11, 0xbb, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 12, 0xcc, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 13, 0xdd, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 14, 0xee, 1200 * 51 * 1);
  memset(gpFrameBuf+1200 * 51 * 15, 0xff, (1200*825)-(1200 * 51 * 15));
  IT8951WaitForDisplayReady();
  //Setting Load image information
  stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
  stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
  stLdImgInfo.usPixelFormat    = IT8951_8BPP; 
  stLdImgInfo.usRotate         = IT8951_ROTATE_0;
  stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
  //Set Load Area
  stAreaImgInfo.usX      = 0;
  stAreaImgInfo.usY      = 0;
  stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
  stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
  //Load Image from Host to IT8951 Image Buffer
  IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
  //Display Area ?V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform 
  IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);
}
/*
extern const unsigned char pic[];
void IT8951DisplayExample3()
{
  IT8951LdImgInfo stLdImgInfo;
  IT8951AreaImgInfo stAreaImgInfo;
  uint32_t i;
  
  for (i = 0;i < 1200*825;i++)
  {
    gpFrameBuf[i] = pic[i];
  }

  IT8951WaitForDisplayReady();
  
  //Setting Load image information
  stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
  stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
  stLdImgInfo.usPixelFormat    = IT8951_8BPP; 
  stLdImgInfo.usRotate         = IT8951_ROTATE_0;
  stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
  //Set Load Area
  stAreaImgInfo.usX      = 0;
  stAreaImgInfo.usY      = 0;
  stAreaImgInfo.usWidth  = 1200;
  stAreaImgInfo.usHeight = 825;
  
  //Load Image from Host to IT8951 Image Buffer
  IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
  //Display Area ?V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform 
  IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);
}
*/


void IT8951_BMP_Example(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  IT8951LdImgInfo stLdImgInfo;
  IT8951AreaImgInfo stAreaImgInfo;
  
  EPD_Clear(0xff);
  
  IT8951WaitForDisplayReady();
  
  //Setting Load image information
  stLdImgInfo.ulStartFBAddr    = (uint32_t)gpFrameBuf;
  stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
  stLdImgInfo.usPixelFormat    = IT8951_4BPP; 
  stLdImgInfo.usRotate         = IT8951_ROTATE_0;
  stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
  //Set Load Area
  stAreaImgInfo.usX      = x;
  stAreaImgInfo.usY      = y;
  stAreaImgInfo.usWidth  = w;
  stAreaImgInfo.usHeight = h;
  
  //Load Image from Host to IT8951 Image Buffer
  IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
  //Display Area ?V (x,y,w,h) with mode 2 for fast gray clear mode - depends on current waveform 
  //IT8951DisplayArea(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 2);
}

//-----------------------------------------------------------
// Load 1bpp image flow (must display with IT8951DisplayArea1bpp()
//-----------------------------------------------------------

void IT8951Load1bppImage(uint8_t* p1bppImgBuf, uint16_t usX, uint16_t usY, uint16_t usW, uint16_t usH)
{
    IT8951LdImgInfo stLdImgInfo;
    IT8951AreaImgInfo stAreaImgInfo;
  
    //Setting Load image information
    stLdImgInfo.ulStartFBAddr    = (uint32_t) p1bppImgBuf;
    stLdImgInfo.usEndianType     = IT8951_LDIMG_L_ENDIAN;
    stLdImgInfo.usPixelFormat    = IT8951_8BPP; //we use 8bpp because IT8951 dose not support 1bpp mode for load image?Aso we use Load 8bpp mode ,but the transfer size needs to be reduced to Size/8
    stLdImgInfo.usRotate         = IT8951_ROTATE_0;
    stLdImgInfo.ulImgBufBaseAddr = gulImgBufAddr;
    //Set Load Area
    stAreaImgInfo.usX      = usX/8;
    stAreaImgInfo.usY      = usY;
    stAreaImgInfo.usWidth  = usW/8;//1bpp, Chaning Transfer size setting to 1/8X of 8bpp mode 
    stAreaImgInfo.usHeight = usH;
    printf("IT8951HostAreaPackedPixelWrite [wait]\n\r");
    //Load Image from Host to IT8951 Image Buffer
    IT8951HostAreaPackedPixelWrite(&stLdImgInfo, &stAreaImgInfo);//Display function 2
}

//-----------------------------------------------------------
//Test function 3---Example of Display 1bpp Flow
//-----------------------------------------------------------
void IT8951Display1bppExample()
{
    IT8951AreaImgInfo stAreaImgInfo;
    
    //Prepare image
    //Write pixel 0x00(Black) to Frame Buffer
    //or you can create your image pattern here..
     memset(gpFrameBuf, 0x00, (gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH)/8);//Host Frame Buffer(Source)
     
     //Check TCon is free ? Wait TCon Ready (optional)
     IT8951WaitForDisplayReady();
     
     //Load Image and Display
     //Set Load Area
     stAreaImgInfo.usX      = 0;
     stAreaImgInfo.usY      = 0;
     stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
     stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
     //Load Image from Host to IT8951 Image Buffer
     IT8951Load1bppImage(gpFrameBuf, stAreaImgInfo.usX, stAreaImgInfo.usY, stAreaImgInfo.usWidth, stAreaImgInfo.usHeight);//Display function 4, Arg
     
     //Display Area - (x,y,w,h) with mode 2 for Gray Scale
     //e.g. if we want to set b0(Background color) for Black-0x00 , Set b1(Foreground) for White-0xFF
     IT8951DisplayArea1bpp(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 0, 0x00, 0xFF);
}

void IT8951Display1bppExample2()
{
    IT8951AreaImgInfo stAreaImgInfo;
    
    //Prepare image
    //Write pixel 0x00(Black) to Frame Buffer
    //or you can create your image pattern here..
    memset(gpFrameBuf, 0xff, (gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH)/8);//Host Frame Buffer(Source)
    
    //Check TCon is free ? Wait TCon Ready (optional)
    IT8951WaitForDisplayReady();
    
    //Load Image and Display
    //Set Load Area
    stAreaImgInfo.usX      = 0;
    stAreaImgInfo.usY      = 0;
    stAreaImgInfo.usWidth  = gstI80DevInfo.usPanelW;
    stAreaImgInfo.usHeight = gstI80DevInfo.usPanelH;
    //Load Image from Host to IT8951 Image Buffer
    IT8951Load1bppImage(gpFrameBuf, stAreaImgInfo.usX, stAreaImgInfo.usY, stAreaImgInfo.usWidth, stAreaImgInfo.usHeight);//Display function 4, Arg
    
    //Display Area - (x,y,w,h) with mode 2 for Gray Scale
    //e.g. if we want to set b0(Background color) for Black-0x00 , Set b1(Foreground) for White-0xFF
    IT8951DisplayArea1bpp(0,0, gstI80DevInfo.usPanelW, gstI80DevInfo.usPanelH, 0, 0x00, 0xFF);
}

void EPD_Clear(uint8_t Color)
{
  //memset(gpFrameBuf, Color, gstI80DevInfo.usPanelW * gstI80DevInfo.usPanelH);
}
