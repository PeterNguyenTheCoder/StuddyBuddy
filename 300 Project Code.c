/*
 * File:   study_buddy.c
 * Author: peter 
 *
 * Created on April 5th, 2024, 9:57 AM
 */
#include <avr/io.h>
#include <avr/interrupt.h>

// FUNCTION PROTOTYPES for speaker and motor
void init_speaker_motor();
void intro_song();
/*
- plays the song for when studybuddy is first turned on 
*/
void study_song();
/*
- plays the song for beginning a study session 
*/
void break_song();
/*
- plays the song for beginning a break session
*/
void end_song();
/*
- plays the song for when studdybuddy has finished every cycle  
*/
void play_note(char note, double length); 
/* 
- receives character and length of desired note, plays note for that length of time
- converts character into specific frequency per note
- can only receive C, D, E, F, G, A, B, H
- these begin from middle C and end at C an octave above, H = C
*/
void speaker_output(unsigned int freq, double length); 
/*
- receives an integer and double input of a specific frequency with its length
- outputs a square wave of duty cycle 50% to speaker
- uses clock and timer to do so 
*/
void play_pause(double length);
/*
- generates a pause of length * 1 second
*/
void motor_buzz();
/*
- produces a short 2 burst vibration 
*/

// FUNCTION DEFINITIONS for speaker and motor
void init_speaker_motor(){

    // Initializes counter
    TCA0.SINGLE.CTRLA = 0b00000001; // sets the timer counter frequency to CLK_PER / 16
    TCA0.SINGLE.PER = 0xffff; // sets the top value of the counter to the max possible value for a 16 bit register
    
    // Initializes outputs for speaker and motor
    PORTD.DIRSET = 0b00100010;

}
void intro_song(){

    play_note('G', 0.7);
    play_note('C', 0.35);
    play_note('E', 0.35);
    play_note('A', 1.2);
    
}
void study_song(){
    play_note('F', 0.35);
    play_note('D', 0.35);
    play_note('A', 0.35);
    play_note('H', 0.5);
}
void break_song(){
    play_note('D', 0.35);
    play_note('F', 0.35);
    play_note('E', 0.35);
    play_note('C', 0.5);
}
void end_song(){
    play_note('A', 0.35);
    play_note('G', 0.35);
    play_note('B', 0.35);
    play_note('H', 0.7);
}
void play_note(char note, double length){
    // this if/else statement plays whatever note is inputed, for length * 1 second 
    if(note == 'C'){
        speaker_output(262, length);
    } else if(note == 'D'){
        speaker_output(293, length);
    } else if(note == 'E'){
        speaker_output(330, length);
    } else if(note == 'F'){
        speaker_output(349, length);
    } else if(note == 'G'){
        speaker_output(392, length);
    } else if(note == 'A'){
        speaker_output(440, length);
    } else if(note == 'B'){
        speaker_output(494, length);
    } else if(note == 'H'){
        speaker_output(523, length);
    }
}
void speaker_output(unsigned int freq, double length){
    // Local variables
    unsigned int time = 0; // incrementing value to ensure note length of 1 second * length
    double top_count = 1/(freq*0.0000002); // freq converted into amount of timer counts
        
    while(time <= (freq*length)){
        PORTD.OUT &= 0b11111101;
        
        while( TCA0.SINGLE.CNT <= (top_count)/2) ;        
        TCA0.SINGLE.CNT = 0;

        PORTD.OUT |= 0b00000010;
        
        while( TCA0.SINGLE.CNT <= (top_count)/2) ;        
        TCA0.SINGLE.CNT = 0;
        time++;
    }  
}
void play_pause(double length){
    // Local variables
    unsigned int count = 0; // incrementing value to ensure note length of 1 second * length
    
    // Initializing clock
    CCP = 0xd8;  
    CLKCTRL.OSCHFCTRLA = 0b00010100;  // sets OSCHF frequency to 8MHz
    while( CLKCTRL.MCLKSTATUS & 0b00000001 ){   // stalls the CLK_MAIN while its source is switching
        ;
    }
    
    // Initializes counter
    TCA0.SINGLE.CTRLA = 0b00001001; // sets the timer counter frequency to CLK_PER / 16
    TCA0.SINGLE.PER = 0xffff; // sets the top value of the counter to the max possible value for a 16 bit register
    
    while(count <= 100){
        while( TCA0.SINGLE.CNT <= (5000*length)){
            PORTD.OUT &= 0b11111101;
        }
        TCA0.SINGLE.CNT = 0;
        count++;
    }
}

// Motor function definition
void motor_buzz(){
    // local variables
    unsigned int count = 0;
    
    PORTD.OUT |= 0b00100000;   
    while(count <= 50){
        while( TCA0.SINGLE.CNT <= 10000){}
        TCA0.SINGLE.CNT = 0;
        count++;
    }
    count = 0;
        
    PORTD.OUT &= 0b11011111;
    while(count <= 700){
        while( TCA0.SINGLE.CNT <= 1000){}
        TCA0.SINGLE.CNT = 0;
        count++;
    }
    count = 0;
    PORTD.OUT |= 0b00100000;   
    while(count <= 50){
        while( TCA0.SINGLE.CNT <= 10000){}
        TCA0.SINGLE.CNT = 0;
        count++;
    }
    count = 0;
        
    PORTD.OUT &= 0b11011111;
    
}


//Functions to do with LCD
void enable(){
    unsigned long counter=0;
    while(counter < 450){counter++;} // must exceed 230ns
    PORTA.OUT |= 0b00000100;
    counter=0;
    while(counter < 450){counter++;} // must exceed 270ns
    PORTA.OUT &= 0b11111011;
}                    //updates LCD. Should not be called by user
void clearDisplay(){
    PORTA.OUT &= 0b00000011;
    enable();
    PORTA.OUT |= 0b00010000;
    PORTA.OUT &= 0b00010011;
    enable();
}              //Clears display, resets cursor to top left
void initDisplay() {
    PORTA.DIRSET = 0b11111111;  // PA7-4 -> DB7-4
    // Enables PA7-2 for output    PA3   -> RS
    //                             PA2   -> EN
    unsigned long counter=0;
    while(counter < 5000){
        counter++;
    }
    //Enter 4-bit mode
    PORTA.OUT |= 0b00100000;
    PORTA.OUT &= 0b00100011;
    enable();
    //Function set
    PORTA.OUT |= 0b00100000;
    PORTA.OUT &= 0b00100011;
    enable();
    PORTA.OUT |= 0b10000000;
    PORTA.OUT &= 0b10000011;
    enable();
    //Display On/Off Control
    PORTA.OUT &= 0b00000011;
    enable();
    PORTA.OUT |= 0b11100000;
    //PORTA.OUT |= 0b11000000; //comment out above line and uncomment this one to disable cursor
    PORTA.OUT &= 0b11100011;
    enable();
    //Entry mode set
    PORTA.OUT &= 0b00000011;
    enable();
    PORTA.OUT |= 0b01100000;
    PORTA.OUT &= 0b01100011;
    enable();
    clearDisplay();
}             //Initializes display on pins PA7-2. RUN ONLY ONCE
void resetCursor(){
    PORTA.OUT &= 0b00000011;
    enable();
    PORTA.OUT |= 0b00100000;
    PORTA.OUT &= 0b00100011;
    enable();
}               //Resets cursor, does not clear display
void cursorRight(int x){
    for(int i = 0;i<x;i++){
        PORTA.OUT |= 0b00010000;
        PORTA.OUT &= 0b00010011;
        enable();
        PORTA.OUT |= 0b01000000;
        PORTA.OUT &= 0b01000011;
        enable();
    }
}         //Moves cursor right x times
void cursorLeft(int x){
    for(int i = 0; i<x; i++){
        PORTA.OUT |= 0b00010000;
        PORTA.OUT &= 0b00010011;
        enable();
        PORTA.OUT &= 0b00000011;
        enable();
    }
}           //Moves cursor left x times
void cursorRow(){
    cursorRight(40);
}                 //Switches the cursor's current row
void print(int x){
    switch(x){
         case ' ':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b00001000;
            PORTA.OUT &= 0b00001011;
            enable();
            break;
         case '!':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b00011000;
            PORTA.OUT &= 0b00011011;
            enable();
            break;
         case '"':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            break;
         case '#':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            break;
         case '$':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            break;
         case '%':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            break;
         case '&':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            break;
         case '\'':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            break;
         case '(':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b10001000;
            PORTA.OUT &= 0b10001011;
            enable();
            break;
         case ')':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b10011000;
            PORTA.OUT &= 0b10011011;
            enable();
            break;
         case '*':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b10101000;
            PORTA.OUT &= 0b10101011;
            enable();
            break;
         case '+':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b10111000;
            PORTA.OUT &= 0b10111011;
            enable();
            break;
         case ',':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b11001000;
            PORTA.OUT &= 0b11001011;
            enable();
            break;
         case '-':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b11011000;
            PORTA.OUT &= 0b11011011;
            enable();
            break;
         case '.':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b11101000;
            PORTA.OUT &= 0b11101011;
            enable();
            break;
         case '/':
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            PORTA.OUT |= 0b11111000;
            PORTA.OUT &= 0b11111011;
            enable();
            break;
         case 0:
         case '0':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b00001000;
            PORTA.OUT &= 0b00001011;
            enable();
            break;
         case 1:
         case '1':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b00011000;
            PORTA.OUT &= 0b00011011;
            enable();
            break;
         case 2:
         case '2':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            break;
         case 3:
         case '3':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            break;
         case 4:
         case '4':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            break;
         case 5:
         case '5':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            break;
         case 6:
         case '6':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            break;
         case 7:
         case '7':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            break;
         case 8:
         case '8':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b10001000;
            PORTA.OUT &= 0b10001011;
            enable();
            break;
         case 9:
         case '9':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b10011000;
            PORTA.OUT &= 0b10011011;
            enable();
            break;
         case ':':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b10101000;
            PORTA.OUT &= 0b10101011;
            enable();
            break;
         case ';':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b10111000;
            PORTA.OUT &= 0b10111011;
            enable();
            break;
         case '<':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b11001000;
            PORTA.OUT &= 0b11001011;
            enable();
            break;
         case '=':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b11011000;
            PORTA.OUT &= 0b11011011;
            enable();
            break;
         case '>':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b11101000;
            PORTA.OUT &= 0b11101011;
            enable();
            break;
         case '?':
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            PORTA.OUT |= 0b11111000;
            PORTA.OUT &= 0b11111011;
            enable();
            break;
         case '@':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b00001000;
            PORTA.OUT &= 0b00001011;
            enable();
            break;
         case 'A':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b00011000;
            PORTA.OUT &= 0b00011011;
            enable();
            break;
         case 'B':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            break;
         case 'C':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            break;
         case 'D':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            break;
         case 'E':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            break;
         case 'F':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            break;
         case 'G':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            break;
         case 'H':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b10001000;
            PORTA.OUT &= 0b10001011;
            enable();
            break;
         case 'I':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b10011000;
            PORTA.OUT &= 0b10011011;
            enable();
            break;
         case 'J':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b10101000;
            PORTA.OUT &= 0b10101011;
            enable();
            break;
         case 'K':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b10111000;
            PORTA.OUT &= 0b10111011;
            enable();
            break;
         case 'L':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b11001000;
            PORTA.OUT &= 0b11001011;
            enable();
            break;
         case 'M':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b11011000;
            PORTA.OUT &= 0b11011011;
            enable();
            break;
         case 'N':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b11101000;
            PORTA.OUT &= 0b11101011;
            enable();
            break;
         case 'O':
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            PORTA.OUT |= 0b11111000;
            PORTA.OUT &= 0b11111011;
            enable();
            break;
         case 'P':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b00001000;
            PORTA.OUT &= 0b00001011;
            enable();
            break;
         case 'Q':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b00011000;
            PORTA.OUT &= 0b00011011;
            enable();
            break;
         case 'R':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            break;
         case 'S':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            break;
         case 'T':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            break;
         case 'U':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            break;
         case 'V':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            break;
         case 'W':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            break;
         case 'X':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b10001000;
            PORTA.OUT &= 0b10001011;
            enable();
            break;
         case 'Y':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b10011000;
            PORTA.OUT &= 0b10011011;
            enable();
            break;
         case 'Z':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b10101000;
            PORTA.OUT &= 0b10101011;
            enable();
            break;
         case '[':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b10111000;
            PORTA.OUT &= 0b10111011;
            enable();
            break;
         case ']':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b11011000;
            PORTA.OUT &= 0b11011011;
            enable();
            break;
         case '^':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b11101000;
            PORTA.OUT &= 0b11101011;
            enable();
            break;
         case '_':
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            PORTA.OUT |= 0b11111000;
            PORTA.OUT &= 0b11111011;
            enable();
            break;
         case '`':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b00001000;
            PORTA.OUT &= 0b00001011;
            enable();
            break;
         case 'a':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b00011000;
            PORTA.OUT &= 0b00011011;
            enable();
            break;
         case 'b':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            break;
         case 'c':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            break;
         case 'd':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            break;
         case 'e':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            break;
         case 'f':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            break;
         case 'g':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            break;
         case 'h':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b10001000;
            PORTA.OUT &= 0b10001011;
            enable();
            break;
         case 'i':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b10011000;
            PORTA.OUT &= 0b10011011;
            enable();
            break;
         case 'j':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b10101000;
            PORTA.OUT &= 0b10101011;
            enable();
            break;
         case 'k':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b10111000;
            PORTA.OUT &= 0b10111011;
            enable();
            break;
         case 'l':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b11001000;
            PORTA.OUT &= 0b11001011;
            enable();
            break;
         case 'm':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b11011000;
            PORTA.OUT &= 0b11011011;
            enable();
            break;
         case 'n':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b11101000;
            PORTA.OUT &= 0b11101011;
            enable();
            break;
         case 'o':
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            PORTA.OUT |= 0b11111000;
            PORTA.OUT &= 0b11111011;
            enable();
            break;
         case 'p':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b00001000;
            PORTA.OUT &= 0b00001011;
            enable();
            break;
         case 'q':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b00011000;
            PORTA.OUT &= 0b00011011;
            enable();
            break;
         case 'r':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b00101000;
            PORTA.OUT &= 0b00101011;
            enable();
            break;
         case 's':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b00111000;
            PORTA.OUT &= 0b00111011;
            enable();
            break;
         case 't':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b01001000;
            PORTA.OUT &= 0b01001011;
            enable();
            break;
         case 'u':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b01011000;
            PORTA.OUT &= 0b01011011;
            enable();
            break;
         case 'v':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b01101000;
            PORTA.OUT &= 0b01101011;
            enable();
            break;
         case 'w':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            break;
         case 'x':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b10001000;
            PORTA.OUT &= 0b10001011;
            enable();
            break;
         case 'y':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b10011000;
            PORTA.OUT &= 0b10011011;
            enable();
            break;
         case 'z':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b10101000;
            PORTA.OUT &= 0b10101011;
            enable();
            break;
         case '{':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b10111000;
            PORTA.OUT &= 0b10111011;
            enable();
            break;
         case '|':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b11001000;
            PORTA.OUT &= 0b11001011;
            enable();
            break;
         case '}':
            PORTA.OUT |= 0b01111000;
            PORTA.OUT &= 0b01111011;
            enable();
            PORTA.OUT |= 0b11011000;
            PORTA.OUT &= 0b11011011;
            enable();
            break;
    }
}              //prints input x to LCD
void printStr(const char *str){
    for(int i=0; str[i] != '\0'; i++){
        print(str[i]);
    }
}   //prints a string str to LCD
void delay(int i) {
    int delay = 0;
    while(delay < i){ //message displayed for 3 seconds
        if(secondPassed()){
            delay++;
        }
    }
}               // delay in seconds

//Functions to do with AVR timer
void initClock(){
    //32k oscillator always active
    CLKCTRL.OSC32KCTRLA |= 0b10000000;
    //choose 32k oscillator
    CLKCTRL.MCLKCTRLA |= 0b00000001;
    //select 1.024kHz
    RTC.CLKSEL |= 0b00000001;
    //enable periodic interrupt
    RTC.PITINTCTRL |= 0b00000001;
    //select 1024 cycles and enable
    RTC.PITCTRLA |= 0b01001001;
}                //Initializes clock. RUN ONLY ONCE
int secondPassed(){
    if(RTC.PITINTFLAGS){
        RTC.PITINTFLAGS = 0b00000001;
        return 1;
        }
    else{
        return 0;
    }
}               //Returns 1 if a second has passed since it was last called

//Function for the buttons
void initButton(){
    
    PORTD.DIRCLR = 0b00000100;
    // Enable global interrupts.
    SREG = 0b10000000;
    
    // Set the ADC reference level to VDD.
    VREF.ADC0REF = 0b10000101;
    
    // Enable the ADC interrupt.
    ADC0.INTCTRL = 0b00000001;
    
    // Select PD2 (AIN2) as the ADC input.
    ADC0.MUXPOS = 0x02;

    // Select minimum clock divide.
    ADC0.CTRLC = 0x00; 
    
    // Select single ended mode, 12 bit resolution and free-running modes.
    ADC0.CTRLA = 0b00000011;
    
    // Start conversion.
    ADC0.COMMAND = 0x01; 
}               // initialize the buttons

int selectButton(){
    if(ADC0.INTFLAGS = 0b0000001){
        
        if(ADC0.RES > 0xe66){
            return 1;
        }
        else{
            return 0;
        }
    }
}               //returns 1 when the select button is pressed, 0 otherwise
int leftButton() {
    if(ADC0.INTFLAGS = 0b0000001){
        if(ADC0.RES > 0x199 & ADC0.RES < 0x333){
            //while(ADC0.RES > 0x320 & ADC0.RES < 0x352){}
            return 1;
        }
        else{
            return 0;
        }
    }
}                 //returns 1 when the left button is pressed, 0 otherwise
int rightButton(){
    if(ADC0.INTFLAGS = 0b0000001){    
        if(ADC0.RES > 0x385 & ADC0.RES < 0x4cc){
            //while(ADC0.RES > 0x659 & ADC0.RES < 0x68b){}
            return 1;
        }
        else{
            return 0;
        }
    }
}                //returns 1 when the right button is pressed, 0 otherwise
int upButton(){
    if(ADC0.INTFLAGS = 0b0000001){
        if(ADC0.RES > 0x51e & ADC0.RES < 0x75c){
            //while(ADC0.RES > 0x979 & ADC0.RES < 0x9ab){}
            return 1;
        }
        else{
            return 0;
        }
    }
}                   //returns 1 when the up button is pressed, 0 otherwise
int downButton(){
    if(ADC0.INTFLAGS = 0b0000001){
        if(ADC0.RES > 0x8f5 & ADC0.RES < 0xa8f){
            //while(ADC0.RES > 0xcb2 & ADC0.RES < 0xce4){}
            return 1;
        }
        else{
            return 0;
        }
    }
}                 //returns 1 when the down button is pressed, 0 otherwise
int user_input(){
    
    int input;
    
    while(ADC0.RES <= 0x030){}
        
    if (selectButton()){
        input = 1;
    } else if (downButton()){
        input = 2;  
    } else if (rightButton()){
        input = 3;
    } else if (upButton()){
        input = 4; 
    } else if(leftButton()){
        input = 5;  
    } else {
        input = 0;
    }
        
    while(ADC0.RES >= 0x030){}
    
    return input;
}

//Functions to do with code logic
void welcome(){
    intro_song(); // Play the song
    motor_buzz(); // Motor Vibration
    clearDisplay();
    printStr("Welcome to:");
    resetCursor();
    cursorRow();
    printStr("Study Buddy");
    delay(3);
    clearDisplay();
    printStr("Press select");
    resetCursor();
    cursorRow();
    printStr("to start:");
    while(user_input() != 1){}
    //delay(2);
}                  // welcome message
int getStudyInput(){
    int leftNum = 5;
    int rightNum = 5;
    clearDisplay();
    printStr("Study Time: ");
    print(leftNum);
    print(rightNum);
    print('m');
    cursorLeft(3); //go back to leftmost number
    int right = 0; //0 means not on this number
    int left = 1;  //1 means we are on this number
    int input;
    
    while(1){
        
        input = user_input();
        
        if(input == 5 && left == 0){
            //printStr("1");
            //delay(1);
            left = 1;
            right = 0;
            cursorLeft(1);
        }
        else if (input == 3 && right == 0){
            //printStr("2");
            //delay(1);
            right = 1;
            left = 0;
            cursorRight(1);
        }
        
        if(left == 1 && input == 4){
            //printStr("3");
            //delay(1);
            leftNum++;
            if(leftNum > 9){
                leftNum = 0;
            }
            print(leftNum);
            cursorLeft(1);
            //delay(1);
        }
        else if(left == 1 && input == 2){
            //printStr("4");
            //delay(1);
            leftNum--;
            if(leftNum < 0){
                leftNum = 9;
            }
            print(leftNum);
            cursorLeft(1);
            //delay(1);
        }
        
        if(right == 1 && input == 4){
            //printStr("5");
            //delay(1);
            rightNum++;
            if(rightNum > 9){
                rightNum = 0;
            }
            print(rightNum);
            cursorLeft(1);
            //delay(1);
        }
        else if(right == 1 && input == 2){
            //printStr("6");
            //delay(1);
            rightNum--;
            if(rightNum < 0){
                rightNum = 9;
            }
            print(rightNum);
            cursorLeft(1);
            //delay(1);
        } else if(input == 1){
            //printStr("7");
            //delay(1);
            break;
        }
        
    }
    
    //delay(1);
    return leftNum * 10 + rightNum;
}             //returns the study time in minutes
int getBreakInput(){
    int leftNum = 5;
    int rightNum = 5;
    clearDisplay();
    printStr("Break Time: ");
    print(leftNum);
    print(rightNum);
    print('m');
    cursorLeft(3); //go back to leftmost number
    int right = 0;
    int left = 1;
    int input;
    
    while(1){
        
        input = user_input();
        
        if(input == 5 && left == 0){
            left = 1;
            right = 0;
            cursorLeft(1);
        }
        else if (input == 3 && right == 0){
            right = 1;
            left = 0;
            cursorRight(1);
        }
        
        if(left == 1 && input == 4){
            leftNum++;
            if(leftNum > 9){
                leftNum = 0;
            }
            print(leftNum);
            cursorLeft(1);
            //delay(1);
        }
        else if(left == 1 && input == 2){
            leftNum--;
            if(leftNum < 0){
                leftNum = 9;
            }
            print(leftNum);
            cursorLeft(1);
            //delay(1);
        }
        
        if(right == 1 && input == 4){
            rightNum++;
            if(rightNum > 9){
                rightNum = 0;
            }
            print(rightNum);
            cursorLeft(1);
            //delay(1);
        }
        else if(right == 1 && input == 2){
            rightNum--;
            if(rightNum < 0){
                rightNum = 9;
            }
            print(rightNum);
            cursorLeft(1);
            //delay(1);
        } else if(input == 1){
            break;
        }
        
    }
    
    //delay(1);
    return leftNum * 10 + rightNum;
    
    
}             //returns the break time in minutes
int getRotations(){
    int rots = 1;
    clearDisplay();
    printStr("Rotations: ");
    print(rots);
    cursorLeft(1); //go back to leftmost number
    int input;
    
    while(1){
        
        input = user_input();
        
        if(input == 4){
            rots++;
            if(rots > 9){
                rots = 0;
            }
            print(rots);
            cursorLeft(1);
            //delay(1);
        }
        else if(input == 2){
            rots--;
            if(rots < 0){
                rots = 9;
            }
            print(rots);
            cursorLeft(1);
            //delay(1);
        } else if(input == 1){
            break;
        }
        
    }
    
    //delay(1);
    return rots;
}              //returns the number of rotations (cycles)
void displayInput(int studyTime, int breakTime, int rotations){
   //Displays the chose study/break time and rotations
   int dig1 = studyTime / 10;
   int dig2 = studyTime % 10;
   int dig3 = breakTime / 10;
   int dig4 = breakTime % 10;
   clearDisplay();
   printStr("You chose: ");
   print(dig1);
   print(dig2);
   print('/');
   print(dig3);
   print(dig4);
   resetCursor();
   cursorRow();
   print(rotations);
   printStr(" times!! :D");
   delay(3);
}
int indTimer(int x, int rots){
   //x is number of mins timer will run for
   int x_seconds = x * 60;
   // above converts x from minutes to seconds
   resetCursor();
   cursorRow();
   printStr("Rotations Left:");
   print(rots);
  
   while(x_seconds > 0){
         x_seconds--;
       while(!secondPassed()){}
         resetCursor();
         cursorRight(11);
         int seconds = x_seconds % 60;
         int minutes = (x_seconds - seconds) / 60;
         //convert i into minutes & seconds
         int dig1 = minutes / 10;
         int dig2 = minutes % 10;
         int dig3 = seconds / 10;
         int dig4 = seconds % 10;
         //convert minutes and seconds to individual digits to be passed to print function
         print(dig1);
         print(dig2);
         print(':');
         print(dig3);
         print(dig4);
         //print countdown to LCD screen
    }
}   //Should print timer starting at 11th digit on LCD
void allTimer(int studyTime, int breakTime, int rotations){  
   //This function calls the indTimer function to do the chosen study/break times for the chosen rotations
   for(int i = rotations; i > 0; i--){
      PORTA.OUT |= 0b00000001; // Turn off LED_2
      PORTA.OUT &= 0b11111101; // Turn on LED_1 (Study LED)      
      study_song(); // Play the song
      motor_buzz(); // Motor Vibration
      clearDisplay();
      resetCursor();
      printStr("Study Time ");
      indTimer(studyTime, i);
      clearDisplay();
      printStr("Switch!");
      for(int count = 0; count < 2; count++){   // Switch LED states
        PORTA.OUT |= 0b00000010; // Turn off LED_1
        PORTA.OUT &= 0b11111110; // Turn on LED_2   
        delay(1);
        PORTA.OUT |= 0b00000001; // Turn off LED_2
        PORTA.OUT &= 0b11111101; // Turn on LED_1
        delay(1);
      }
      PORTA.OUT |= 0b00000010; // Turn off LED_1      
      PORTA.OUT &= 0b11111110; // Turn on LED_2 (Break LED)         
      while(!secondPassed());
      clearDisplay();
      if(i > 1){
        break_song(); // Play the song
        motor_buzz(); // Motor Vibration
        clearDisplay();
        resetCursor();
        printStr("Break Time ");
        indTimer(breakTime, i);
        clearDisplay();
        printStr("Switch!");
        for(int count = 0; count < 2; count++){   // Switch LED states
            PORTA.OUT |= 0b00000001; // Turn off LED_2
            PORTA.OUT &= 0b11111101; // Turn on LED_1
            delay(1);
            PORTA.OUT |= 0b00000010; // Turn off LED_1
            PORTA.OUT &= 0b11111110; // Turn on LED_2   
            delay(1);
        }
        while(!secondPassed());
      }
   }
}                   // Includes LED
void closing(){
    end_song(); // Play the song
    motor_buzz(); // Motor Vibration
    clearDisplay();
    resetCursor();
    printStr("All Done!");
    delay(3);
}                  //closing message

int main(void) {
    
    init_speaker_motor();
    initButton();
    initDisplay();
    initClock();

    int userStudy;
    int userBreak;
    int userRotations;
    
    PORTA.OUT |= 0b00000010; // Turn on LED_1
    PORTA.OUT |= 0b00000001; // Turn on LED_2 
    
    while(1){
      
      
      welcome();
      while(!secondPassed()){}
      userStudy = getStudyInput();
      while(!secondPassed()){}
      userBreak = getBreakInput();
      while(!secondPassed()){}
      userRotations = getRotations();
      displayInput(userStudy, userBreak, userRotations);
      allTimer(userStudy, userBreak, userRotations);
      PORTA.OUT |= 0b00000010; // Turn on LED_1
      PORTA.OUT |= 0b00000001; // Turn on LED_2
      closing();
      
    }
}