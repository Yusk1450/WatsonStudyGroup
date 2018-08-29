
/*-----------------------------------------------------------*/
void clearRxBuf()
{
  while (mySerial2.available()) 
  {
    mySerial2.read(); 
  }
}

/*-----------------------------------------------------------*/
void sendCmd(char cmd[], int cmd_len)
{
  for (char i = 0; i < cmd_len; i++)
  {
    mySerial2.write(cmd[i]); 
  }
}

/*-----------------------------------------------------------*/
int readBytes(char *dest, int len, unsigned int timeout)
{
  int read_len = 0;
  unsigned long t = millis();
  while (read_len < len)
  {
    while (mySerial2.available()<1)
    {
      if ((millis() - t) > timeout)
      {
        return read_len;
      }
    }
    *(dest+read_len) = mySerial2.read();
    read_len++;
  }
  return read_len;
}

/*-----------------------------------------------------------*/
void initialize()
{   
  char cmd[] = {0xaa,0x0d|cameraAddr,0x00,0x00,0x00,0x00} ;  
  unsigned char resp[6];

  Serial.print("initializing camera...");
  
  while (1) 
  {
    sendCmd(cmd,6);
    if (readBytes((char *)resp, 6,1000) != 6)
    {
      Serial.print(".");
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0) 
    {
      if (readBytes((char *)resp, 6, 500) != 6) continue; 
      if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break; 
    }
  }  
  cmd[1] = 0x0e | cameraAddr;
  cmd[2] = 0x0d;
  sendCmd(cmd, 6); 
  Serial.println("\nCamera initialization done.");
}

/*-----------------------------------------------------------*/
void preCapture()
{
  char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };  
  unsigned char resp[6]; 
  
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue; 
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break; 
  }
  Serial.println("preCapture() is done.");
}

/*-----------------------------------------------------------*/
void capture()
{
  char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN>>8) & 0xff ,0}; 
  unsigned char resp[6];

  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break; 
  }
  cmd[1] = 0x05 | cameraAddr;
  cmd[2] = 0;
  cmd[3] = 0;
  cmd[4] = 0;
  cmd[5] = 0; 
  while (1)
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1) 
  {
    clearRxBuf();
    sendCmd(cmd, 6);
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      if (readBytes((char *)resp, 6, 1000) != 6)
      {
        continue;
      }
      if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
      {
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16); 
        //Serial.print("picTotalLen:");
        //Serial.println(picTotalLen);
        break;
      }
    }
  }
  Serial.println("Capture() is done.");
}

/*-----------------------------------------------------------*/
void getData()
{
    unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6); 
    if ((picTotalLen % (PIC_PKT_LEN-6)) != 0) pktCnt += 1;
    
    char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };  
    unsigned char pkt[PIC_PKT_LEN];
     
    // post文字列の初期化:null埋め
    for(int i = 0; i < qPostDatLen; i++)
    {
        qPostDat[i] = '\0';
    }
   qPostDatCntr=0;
   char field[10] ="COLO_DAT=";
   for(int i = 0; i < 9; i++)
   {
        qPostDat[qPostDatCntr]=field[i];
        qPostDatCntr++;
   }

    for (unsigned int i = 0; i < pktCnt; i++)
    {
        cmd[4] = i & 0xff;
        cmd[5] = (i >> 8) & 0xff;
        
        int retry_cnt = 0;
        retry:
        delay(10);
        clearRxBuf(); 
        sendCmd(cmd, 6); 
        uint16_t cnt = readBytes((char *)pkt, PIC_PKT_LEN, 200);
        
        unsigned char sum = 0; 
        for (int y = 0; y < cnt - 2; y++)
        {
            sum += pkt[y];
        }
        if (sum != pkt[cnt-2])
        {
            if (++retry_cnt < 100) goto retry;
            else break;
        }
        
        char tempCamDAT[cnt-6];
        int tempCounter = 0;
        for(int i = 0; i < cnt-6; i++)
        {
            tempCamDAT[i]='\0';
        }

        for(int j =4; j < cnt - 2;j++)
        {
            tempCamDAT[tempCounter]=(uint8_t)pkt[j];
            tempCounter++;
        }

        int eLength = Base64encodedLength(tempCounter);
        char encodedString[eLength];
        Base64encode(encodedString, tempCamDAT,tempCounter);
        for(int k = 0; k < eLength; k++)
        {
            qPostDat[qPostDatCntr] = encodedString[k];
            qPostDatCntr++;
        }
    }
    cmd[4] = 0xf0;
    cmd[5] = 0xf0; 
    sendCmd(cmd, 6);

    drawText("CAPTURE\nOK");
    
    Serial.println();
    Serial.println("GetData() is done.");
}

