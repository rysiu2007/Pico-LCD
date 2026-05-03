#include <stdio.h>
#include "pico/stdlib.h"
uint D7;
uint D6;
uint D5;
uint D4;
uint RS;
uint E;

int16_t HDinstruction;

// uses 5 lower bytes
void putInstruction(int8_t instruction)
{
    gpio_put(E, true);
    gpio_put(D4, instruction & 0x01);
    gpio_put(D5, (instruction & 0x02) >> 1);
    gpio_put(D6, (instruction & 0x04) >> 2);
    gpio_put(D7, (instruction & 0x08) >> 3);
    gpio_put(RS, (instruction & 0x10) >> 4);
    gpio_put(E, false);
    busy_wait_us(100);
}
// 8 lower bytes for data and 9th for RS
void put8bitInstruction(int16_t instruction)
{
    putInstruction(((instruction & 0b100000000) | (instruction & 0x00f0)) >> 4);
    putInstruction(((instruction & 0b100000000) >> 4) | (instruction & 0x000f));
}

//Clear display writes space code 20H (character pattern for character code 20H must be a blank pattern) into
//all DDRAM addresses. It then sets DDRAM address 0 into the address counter, and returns the display to
//its original status if it was shifted. In other words, the display disappears and the cursor or blinking goes to
//the left edge of the display (in the first line if 2 lines are displayed). It also sets I/D to 1 (increment mode)
//in entry mode. S of entry mode does not change.
void ClearDisplay()
{
    put8bitInstruction(0b000000001);
    busy_wait_us(2000);
}
//Return home sets DDRAM address 0 into the address counter, and returns the display to its original status
//if it was shifted. The DDRAM contents do not change.
//The cursor or blinking go to the left edge of the display (in the first line if 2 lines are displayed).
void ReturnHome()
{
    put8bitInstruction(0b000000010);
    busy_wait_us(2000);
}
//I/D:
// Increments  (I/D  =  1)  or  decrements  (I/D  =  0)  the  DDRAM  address  by  1  when  a  character  code  is
//written into or read from DDRAM.
//The cursor or blinking moves to the right when incremented by 1 and to the left when decremented by 1.
//The same applies to writing and reading of CGRAM.
//S:
// Shifts the entire display either to the right (I/D = 0) or to the left (I/D = 1) when S is 1. The display does
//not shift if S is 0.
//If S is 1, it will seem as if the cursor does not move but the display does. The display does not shift when
//reading from DDRAM. Also, writing into or reading out from CGRAM does not shift the display.
void EntryModeSet(bool INC_DDRAM, bool SHIFT_DISPLAY)
{
    HDinstruction = 0b000000100;
    if (INC_DDRAM)
        HDinstruction += 2;
    if (SHIFT_DISPLAY)
        HDinstruction++;
    put8bitInstruction(HDinstruction);
}

//D:
// The display is on when D is 1 and off when D is 0. When off, the display data remains in DDRAM, but
//can be displayed instantly by setting D to 1.
//C:
// The cursor is displayed when C is 1 and not displayed when C is 0. Even if the cursor disappears, the
// function of I/D or other specifications will not change during display data write. The cursor is displayed
// using 5 dots in the 8th line for 5 × 8  dot  character  font  selection  and  in  the  11th  line  for  the  5 × 10 dot character font selection (Figure 13).
// B:
//   The  character  indicated  by  the  cursor  blinks  when  B  is  1  (Figure  13).  The  blinking  is  displayed  as
// switching between all blank dots and displayed characters at a speed of 409.6-ms intervals. The cursor and blinking can be set to display simultaneously.
void DisplayOnOffControl(bool DISPLAY_ON, bool CURSON_ON, bool CURSOR_BLINK)
{
    HDinstruction = 0b000001000;
    if (DISPLAY_ON)
        HDinstruction += 4;
    if (CURSON_ON)
        HDinstruction += 2;
    if (CURSOR_BLINK)
        HDinstruction++;
    put8bitInstruction(HDinstruction);
}
// Cursor or display shift shifts the cursor position or display to the right or left without writing or reading
// display data (Table 7). This function is used to correct or search the display. In a 2-line display, the cursor
// moves to the second line when it passes the 40th digit of the first line. Note that the first and second line
// displays will shift at the same time.
// When the displayed data is shifted repeatedly each line moves only horizontally. The second line display
// does not shift into the first line position.
// The address counter (AC) contents will not change if the only action performed is a display shift
void CursorOrDisplayShift(bool SHIFT_DISPLAY, bool TO_RIGHT)
{
    HDinstruction = 0b000010000;
    if (SHIFT_DISPLAY)
        HDinstruction += 8;
    if (TO_RIGHT)
        HDinstruction += 4;
    put8bitInstruction(HDinstruction);
}

// DL:
//  Sets the interface data length. Data is sent or received in 8-bit lengths (DB7 to DB0) when DL is 1,
// and  in  4-bit  lengths  (DB7  to  DB4)  when  DL  is  0.When  4-bit  length  is  selected,  data  must  be  sent  or
// received twice.
// N:
//  Sets the number of display lines.
// F:
//  Sets the character font.
// Note:    Perform the function at the head of the program before executing any instructions (except for the
// read  busy  flag  and  address  instruction).  From  this  point,  the  function  set  instruction  cannot  be
// executed unless the interface data length is changed
void FunctionSet(bool BIT8_MODE, bool TWO_LINES, bool TALLER_FONT)
{
    HDinstruction = 0b000100000;
    if (BIT8_MODE)
        HDinstruction += 16;
    if (TWO_LINES)
        HDinstruction += 8;
    if (TALLER_FONT)
        HDinstruction += 4;
    put8bitInstruction(HDinstruction);
}
// Set DDRAM address sets the DDRAM address binary AAAAAAA into the address counter.
// Data is then written to or read from the MPU for DDRAM.
// However,  when  N  is  0  (1-line  display),  AAAAAAA  can  be  00H  to  4FH.  When  N  is  1  (2-line  display),
// AAAAAAA can be 00H to 27H for the first line, and 40H to 67H for the second line.
void SetDDRAMAddress(int8_t address){
    HDinstruction = 0b010000000|(address&0b01111111);
    put8bitInstruction(HDinstruction);
}
int written = 0;
void printChar(char character)
{
    HDinstruction = 0b100000000|character;
    put8bitInstruction(HDinstruction);
    written++;
}
union varo{
    int d;
    uint u;
    double f;
    char c;
};

void printN(int64_t num){
    if(num<0){
        num*=-1;
        printChar('-');
    }
    int i = 1;
    while(num%i!=num)i*=10;
    while(i>1){
        printChar(((num%i)/(i/10))+'0');
        i/=10;
    }
    if(num==0)printChar('0');
}

int pow(int base, int8_t exponent){
    int ret = 1;
    for (int8_t i = 0; i < exponent; i++){
        ret*=base;
    }
    return ret;
}

void printF(float num,int8_t precision){
    int floor = (int)(num);
    printN(floor%100);
    printChar(',');
    floor = (int)(num*pow(10,precision));
    printN(floor);

}

void print(char *text,...)
{
    va_list args;
   // va_start(args,n);
   //"jk%f"
    int n = 0, argc=0;
    union varo help;
    while(text[n]!=0)
    {
        if(text[n++]=='%') argc++;
        if(text[n]=='%') argc--;
    }
    va_start(args,text);
    for (int i = 0; i < n; i++)
    {
        if(text[i]=='%'){
            i++;
            switch (text[i])
            {
            case 'd'||'i':
                help.d = va_arg(args,int);
                printN(help.d);
                break;
            case 'u':
                help.u = va_arg(args,uint);
                printN(help.u);
                break;
            case 'f':
                help.f = va_arg(args,double);
                if(text[i+1]=='.'){
                    i++;i++;
                    printF(help.f,text[i]-'0');
                }
            case '%':
                printChar('%');
                break;
            default:
                break;
            }
        }
        else
            printChar(text[i]);
    }
    va_end(args);
    
}


//Settings (counting from left, big endian, 1 - true):
//BIT 1 - BIT 3 FunctionSet options:
//1 - 8 bit mode
//2 - two lines mode
//3 - tall font (exclusive with the second);
//BIT 4 - BIT 6 DisplayOnOffControl options:
//4 - display on
//5 - cursor on
//6 - cursor blink (5 must be on)
//BIT 7 - BIT 8 EntryModeSet options:
//7 - increment DDRAM
//8 - shift display
void initDisplay(uint8_t settings, uint8_t pin_D7,uint8_t pin_D6,uint8_t pin_D5,uint8_t pin_D4,uint8_t pin_RS,uint8_t pin_E){
    D7 = pin_D7; D6 = pin_D6; D5 = pin_D5; D4 = pin_D4; RS = pin_RS; E = pin_E;
    gpio_init(D7);
    gpio_init(D6);
    gpio_init(D5);
    gpio_init(D4);
    gpio_init(RS);
    gpio_init(E);
    gpio_set_dir(D7, GPIO_OUT);
    gpio_set_dir(D6, GPIO_OUT);
    gpio_set_dir(D5, GPIO_OUT);
    gpio_set_dir(D4, GPIO_OUT);
    gpio_set_dir(RS, GPIO_OUT);
    gpio_set_dir(E, GPIO_OUT);
    putInstruction(0b00010);
    FunctionSet((settings&0b10000000)>>7, (settings&0b01000000)>>6, (settings&0b00100000)>>5);
    DisplayOnOffControl((settings&0b00010000)>>4, (settings&0b00001000)>>3, (settings&0b00000100)>>2);
    EntryModeSet((settings&0b00000010)>>1, (settings&0b00000001));
}
