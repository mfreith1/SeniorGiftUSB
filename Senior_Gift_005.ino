/*
Program: Senior_Gift_005
Written By: Mark Freithaler
Using: Arduino Max7219 Learning code, which can be found at the following URL
http://playground.arduino.cc/LEDMatrix/Max7219
Revision Date: 05/19/2016
*/

#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3
#define SCROLL_SLOWNESS 145

#define LEDCONTROL_SPI_MOSI 0
#define LEDCONTROL_SPI_CLK 1
#define LEDCONTROL_SPI_CS 7
#define BUTTONPIN1 2
#define BUTTONPIN2 3
#define MAX_LENGTH 25
#define SNAKE_SLOWNESS 300

const static byte alphabetBitmap[41][4]={
	{0x70,0x88,0x70,0x0}, //0
	{0x48,0xf8,0x08,0x0}, //1
	{0x48,0x98,0x68,0x0}, //2
	{0xa8,0xa8,0x50,0x0}, //3
	{0xe0,0x20,0xf8,0x0}, //4
	{0xe8,0xa8,0x90,0x0}, //5
	{0x70,0xa8,0x90,0x0}, //6
	{0x80,0xb8,0xc0,0x0}, //7
	{0x50,0xa8,0x50,0x0}, //8
	{0xe0,0xa0,0xf8,0x0}, //9
	{0x00,0x00,0x00,0x0}, // blank space
	{0x00,0x50,0x00,0x0}, //:
	{0x00,0x20,0x20,0x0}, // -
	{0x00,0x08,0x00,0x0}, // .
	{0xf8,0xf8,0xf8,0x0}, //Ñ //Replaced with a Black Box
	{0x78,0xa0,0x78,0x0}, //A
	{0xf8,0xa8,0x50,0x0}, //B
	{0x70,0x88,0x50,0x0}, //C
	{0xf8,0x88,0x70,0x0}, //D
	{0xf8,0xa8,0xa8,0x0}, //E
	{0xf8,0xa0,0xa0,0x0}, //F
	{0x70,0x88,0xb0,0x0}, //G
	{0xf8,0x20,0xf8,0x0}, //H
	{0x88,0xf8,0x88,0x0}, //I
	{0x90,0x88,0xf0,0x0}, //J
	{0xf8,0x20,0xd8,0x0}, //K
	{0xf8,0x08,0x08,0x0}, //L
	{0xf8,0x60,0xf8,0x0}, //M
	{0xf8,0x80,0x78,0x0}, //N
	{0x70,0x88,0x70,0x0}, //O
	{0xf8,0xa0,0x40,0x0}, //P
	{0x70,0x88,0x78,0x0}, //Q
	{0xf8,0xa0,0x58,0x0}, //R
	{0x48,0xa8,0x90,0x0}, //S
	{0x80,0xf8,0x80,0x0}, //T
	{0xf0,0x08,0xf0,0x0}, //U
	{0xe0,0x18,0xe0,0x0}, //V
	{0xf8,0x30,0xf8,0x0}, //W
	{0xd8,0x20,0xd8,0x0}, //X
	{0xc0,0x38,0xc0,0x0}, //Y
	{0x98,0xa8,0xc8,0x0}  //Z
};

byte spidata[16];
byte screenBuff[5]; // 5 Bytes which each represent a row of the display (only bits 0-5 are being used for 5x6 display)

char loopingString[] = "JOHN, THANK YOU FOR BEING YOU.  PUSH MY BUTTONS.";
char personalMessage[] = "EXAMPLE PERSONAL MESSAGE HERE";

bool playing = true;

volatile int button1_state = HIGH;
volatile int button2_state = HIGH;

void setup () {
  randomSeed(analogRead(0));
  pinMode(LEDCONTROL_SPI_MOSI,OUTPUT);
  pinMode(LEDCONTROL_SPI_CLK,OUTPUT);
  pinMode(LEDCONTROL_SPI_CS,OUTPUT);
  pinMode(9,INPUT); //Setting 9 & 10 to High-Impedance just in case that affects the reading on for the buttons
  pinMode(10,INPUT);  
  pinMode(BUTTONPIN1,INPUT_PULLUP);
  pinMode(BUTTONPIN2,INPUT_PULLUP);  
  digitalWrite(LEDCONTROL_SPI_CS,HIGH);
  spiTransfer(OP_DISPLAYTEST,0);
  setScanLimit(5);
  spiTransfer(OP_DECODEMODE,0);
  clearDisplay();
  dispShutdown(false);
  setIntensity(1);
  
  for(int i=0;i<4;i++) 
    screenBuff[i]=0x00;
  
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN1), button1pressed,FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN2), button2pressed,FALLING);
}  
 
void loop () {
  //Main loop scrolls a simple string like "touch my buttons"
  
  switch (scrollString(loopingString,SCROLL_SLOWNESS, true)){ // Scroll the welcome screen until their is an iterrupt
    case 0x01: // Button 1 was pressed (Display the personal message)
      screenTrans(150);
      scrollString(personalMessage,SCROLL_SLOWNESS, true);
      screenTrans(150);
    break; 
    case 0x02: // Button 2 was pressed (Play a little game)
      screenTrans(150);
      playSnake(); 
      screenTrans(150);
    break;
    default: // Typically triggered by 0x00
      //NO-OP
    break;
  }
  delay(500);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void screenTrans(int slowness){
  for(int i = 0; i < 5; i++){
      screenBuff[i]=0xff;
      refreshDisplay();
      delay(slowness);
  }
  for(int i = 4; i > -1; i--){
      screenBuff[i] = 0x00;
      refreshDisplay();
      delay(slowness);
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void button1pressed(){
  button1_state = LOW;
}
void button2pressed(){
  button2_state = LOW;  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void playSnake(){
  playing = true;
  button1_state = HIGH;
  button2_state = HIGH;
  
  int dir = DIR_LEFT;
  int snakeX[MAX_LENGTH];
  int snakeY[MAX_LENGTH];
  int snakeLength = 1;
  int nomsBlink = 1;
  int slowness = SNAKE_SLOWNESS;
  int nextX = -1;
  int nextY = -1;
  int nomsX = -1;
  int nomsY = -1;
  int blinkTime;
  
  moveNoms(&nomsX, &nomsY, snakeX, snakeY, &snakeLength);
        
  snakeX[snakeLength-1] = 0;
  snakeY[snakeLength-1] = 2;
  for(int i = snakeLength; i < MAX_LENGTH; i++){
    snakeX[i] = -1;
    snakeY[i] = -1;
  }
  while(playing){
    //Check for buttons and adjust direction accordingly
    if(button1_state == LOW){
      button1_state = HIGH;
      dir = (dir+1)%4;
    }else
    if(button2_state == LOW){
      button2_state = HIGH;
      dir = dir-1;
      if(dir < 0){
        dir = 3;
      }
    }
    switch (dir){
      case DIR_UP:
        nextX = snakeX[snakeLength-1];
        nextY = snakeY[snakeLength-1] - 1;
      break;
      case DIR_RIGHT:
        nextX = snakeX[snakeLength-1] - 1;
        nextY = snakeY[snakeLength-1];
      break;
      case DIR_DOWN:
        nextX = snakeX[snakeLength-1];
        nextY = snakeY[snakeLength-1] + 1;
      break;
      case DIR_LEFT:
        nextX = snakeX[snakeLength-1] + 1;
        nextY = snakeY[snakeLength-1];
      break;
      default:
        //NO-OP
      break;
    }
    
    //Blink the noms
    blinkTime = slowness - ((snakeLength-1)*10);
    if(blinkTime < 0){
      blinkTime = 0;
    }
    setLed(nomsY, nomsX, true);
    delay(blinkTime);
    setLed(nomsY, nomsX, false);
    delay(blinkTime);
    
    //Act based on whether or not the snake is moving onto the noms
    if(nextX == nomsX && nextY == nomsY){ //If the snake's head is about to be on top of the noms
      setLed(nextY, nextX, true);
      snakeX[snakeLength] = nextX;
      snakeY[snakeLength] = nextY;      
      snakeLength++;
      moveNoms(&nomsX, &nomsY, snakeX, snakeY, &snakeLength);
    }else{
      setLed(nextY, nextX, true);
      setLed(snakeY[0],snakeX[0],false);
      for(int i=0; i<snakeLength-1; i++){
        snakeX[i] = snakeX[i+1];
        snakeY[i] = snakeY[i+1];
        
      }
      snakeX[snakeLength-1] = nextX;
      snakeY[snakeLength-1] = nextY;     
    }
    for(int i = 0; i < snakeLength-1; i++){ // Check for snake collisions
      for(int j = i+1; j < snakeLength; j++){
        if(snakeX[i] == snakeX[j] && snakeY[i] == snakeY[j]){
          playing = false;
        }
      }
    }
    if(snakeX[snakeLength-1] < 0 || snakeX[snakeLength-1] > 5 || snakeY[snakeLength-1] < 0 || snakeY[snakeLength-1] > 4){ //Check for out of bounds
      playing = false;
    }
  }
  button1_state = HIGH;
  button2_state = HIGH;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void moveNoms(int* x_p, int* y_p, int* snakeX, int* snakeY, int* snakeLength){
  bool repeat = false;
  do{
    *x_p = (int)random(0,5);
    *y_p = (int)random(0,4);
    for(int i = 0; i < *snakeLength; i++){ // Check for snake collisions
      if((snakeX[i] == *x_p) && (snakeY[i] == *y_p)){
        repeat = true;
      }else{
        repeat = false; 
      }
    }
  }while(repeat);
  return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
byte scrollString(char* text,int slowness,bool interruptable){
  button1_state = HIGH;
  button2_state = HIGH;
  
  while(text[0] != 0){
    for(int i = 0; i < 4; i++){
      for(int j = 5; j > -1; j--){
        screenBuff[j] = screenBuff[j] >> 1;
        if(alphabetBitmap[getCharArrayPosition(text[0])][i]&(B10000000 >> j)){
          screenBuff[j]= screenBuff[j]|B01000000;
        }
      }
      refreshDisplay();
      if(interruptable){
        if(button1_state == LOW){
          button1_state = HIGH;
          return 0x01;
        }
        if(button2_state == LOW){
          button2_state = HIGH;
          return 0x02;
        }
      }
      delay(slowness);    
    }
    text++;
  }
  //Clear the screen by scrolling the text off of it
  for(int i = 0; i < 6; i++){
      for(int j = 0; j < 5; j++){
        screenBuff[j]= screenBuff[j] >> 1;
      }
      refreshDisplay();
      delay(slowness);
  }
  return 0x00;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void refreshDisplay(){
  setRow(0, screenBuff[0]);
  setRow(1, screenBuff[1]);
  setRow(2, screenBuff[2]);
  setRow(3, screenBuff[3]);
  setRow(4, screenBuff[4]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int getDeviceCount() {
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void dispShutdown(bool b) {
    if(b)
	spiTransfer(OP_SHUTDOWN,0);
    else
	spiTransfer(OP_SHUTDOWN,1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
void setScanLimit(int limit) {
    if(limit>=0 || limit<8)
    	spiTransfer(OP_SCANLIMIT,limit);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setIntensity(int intensity) {
    if(intensity>=0 || intensity<16)	
	spiTransfer(OP_INTENSITY,intensity);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void clearDisplay() {
    for(int i=0;i<8;i++) {
	screenBuff[i]=0;
	spiTransfer(i+1,screenBuff[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setLed(int row, int column, boolean state) {
    byte val=0x00;
    column++;
    if(row<0 || row>7 || column<0 || column>7)
	return;
    val=B10000000 >> column;
    if(state)
	screenBuff[row]=screenBuff[row]|val;
    else {
	val=~val;
	screenBuff[row]=screenBuff[row]&val;
    }
    spiTransfer(row+1,screenBuff[row]);
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setRow(int row, byte value) {
    if(row<0 || row>7)
	return;
    screenBuff[row]=value;
    spiTransfer(row+1,screenBuff[row]);
}
    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setColumn(int col, byte value) {
    col++;
    byte val;
    if(col<0 || col>7) 
	return;
    for(int row=0;row<8;row++) {
	val=value >> (7-row);
	val=val & 0x01;
	setLed(row,col,val);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void spiTransfer(volatile byte opcode, volatile byte data) {
    int maxbytes=2;
    for(int i=0;i<maxbytes;i++)
	spidata[i]=(byte)0;
    spidata[1]=opcode;    //put our device data into the array
    spidata[0]=data;    //put our device data into the array
    digitalWrite(LEDCONTROL_SPI_CS,LOW);    //enable the line 
    for(int i=maxbytes;i>0;i--)    //Now shift out the data 
 	shiftOut(LEDCONTROL_SPI_MOSI,LEDCONTROL_SPI_CLK,MSBFIRST,spidata[i-1]);
    digitalWrite(LEDCONTROL_SPI_CS,HIGH);    //latch the data onto the display

}    

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int getCharArrayPosition(char input){
     if ((input==' ')||(input=='+')) return 10;
     if (input==':') return 11;
     if (input=='-') return 12;
     if (input=='.') return 13;
     if ((input =='(')) return  14;  //replace by 'ñ'   
     if ((input >='0')&&(input <='9')) return (input-'0');
     if ((input >='A')&&(input <='Z')) return (input-'A' + 15);
     if ((input >='a')&&(input <='z')) return (input-'a' + 15);     
     return 13;
}  

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writeString(int mtx, char * displayString) {
  while ( displayString[0]!=0) {
    char c =displayString[0];
    int pos= getCharArrayPosition(c);
    displayChar(mtx,pos);
    delay(300);
    clearDisplay();
    displayString++;
  }  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayChar(int matrix, int charIndex) {
  for (int i=0; i<6;i++) {
      setRow(i, alphabetBitmap[charIndex][i]);
  } 
}

