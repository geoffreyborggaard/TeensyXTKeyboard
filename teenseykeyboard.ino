// This is a converter for hooking up a PC XT keyboard using USB.
// It is based loosely on 
// https://github.com/kesrut/tinyPS2toXTArduino/blob/master/ps_keyboard.ino
// Hacked to bits by: Geoffrey Borggaard


#define clk 5 /* must be interrupt pin (0 interrupt) */
#define data 6
#define led 11

#define START 's'
#define STOP 'p'
#define INIT 'i'
#define GROUP1_CNT 85
#define BREAK_GRP1 0xF0
#define MAKE_GRP2 0xE0
#define DEBUG 1

#define KEY_RELEASE_MASK 0x80

byte cycles = 0 ; 
unsigned char value = 0 ; 
unsigned char state = INIT ; 
byte got_data = 0 ; 

#define BUFLEN 100

char buf[BUFLEN];
char ptr = 0;

struct key_struct
{
  unsigned char character ; 
  unsigned char usb_make ;
  unsigned is_char ; 
  unsigned char xt_make ;
  boolean needs_key_up;
  unsigned numlock_make;
} ; 

typedef struct key_struct key_type ; 

key_type keys[] =
{
  {'a', 0x04, 1, 0x1E, false, 0x04},
  {'b', 0x05, 1, 0x30, false, 0x05}, 
  {'c', 0x06, 1, 0x2E, false, 0x06}, 
  {'d', 0x07, 1, 0x20, false, 0x07},
  {'e', 0x08, 1, 0x12, false, 0x08},
  {'f', 0x09, 1, 0x21, false, 0x09}, 
  {'g', 0x0A, 1, 0x22, false, 0x0A},
  {'h', 0x0B, 1, 0x23, false, 0x0B}, 
  {'i', 0x0C, 1, 0x17, false, 0x0C},
  {'j', 0x0D, 1, 0x24, false, 0x0D}, 
  {'k', 0x0E, 1, 0x25, false, 0x0E}, 
  {'l', 0x0F, 1, 0x26, false, 0x0F},
  {'m', 0x10, 1, 0x32, false, 0x10}, 
  {'n', 0x11, 1, 0x31, false, 0x11},
  {'o', 0x12, 1, 0x18, false, 0x12}, 
  {'p', 0x13, 1, 0x19, false, 0x13},
  {'q', 0x14, 1, 0x10, false, 0x14},
  {'r', 0x15, 1, 0x13, false, 0x15},
  {'s', 0x16, 1, 0x1F, false, 0x16},
  {'t', 0x17, 1, 0x14, false, 0x17}, 
  {'u', 0x18, 1, 0x16, false, 0x18}, 
  {'v', 0x19, 1, 0x2F, false, 0x19}, 
  {'w', 0x1A, 1, 0x11, false, 0x1A},
  {'x', 0x1B, 1, 0x2D, false, 0x1B},
  {'y', 0x1C, 1, 0x15, false, 0x1C},
  {'z', 0x1D, 1, 0x2C, false, 0x1D}, 
  {'0', 0x27, 1, 0x0B, false, 0x27},
  {'1', 0x1E, 1, 0x02, false, 0x1E},
  {'2', 0x1F, 1, 0x03, false, 0x1F},
  {'3', 0x20, 1, 0x04, false, 0x20},
  {'4', 0x21, 1, 0x05, false, 0x21},
  {'5', 0x22, 1, 0x06, false, 0x22},
  {'6', 0x23, 1, 0x07, false, 0x23},
  {'7', 0x24, 1, 0x08, false, 0x24},
  {'8', 0x25, 1, 0x09, false, 0x25},
  {'9', 0x26, 1, 0x0A, false, 0x26}, 
  {'`', 0x35, 1, 0x29, false, 0x35},
  {'-', 0x2D, 1, 0x0C, false, 0x2D},
  {'=', 0x2E, 1, 0x0D, false, 0x2E},
  {'\\', 0x31, 1, 0x2B, false, 0x31},
  {'\b', 0x2A, 0, 0x0E, true, 0x2A}, // backsapce
  {' ', 0x2C, 1, 0x39, true, 0x2C}, // space
  {'\t', 0x2B, 0, 0x0F, false, 0x2B}, // tab
  {' ', 0x58, 0, 0x3A, false, 0x58}, // caps
  {'\n', 0x28, 1, 0x1C, true, 0x28}, // enter
  {' ', 0x29, 0, 0x01, false, 0x29}, // esc
  {' ', 0x3A, 0, 0x3B, false, 0x3A}, // F1
  {' ', 0x3B, 0, 0x3C, false, 0x3B}, // F2
  {' ', 0x3C, 0, 0x3D, false, 0x3C}, // F3
  {' ', 0x3D, 0, 0x3E, false, 0x3D}, // F4
  {' ', 0x3E, 0, 0x3F, false, 0x3E}, // F5
  {' ', 0x3F, 0, 0x40, false, 0x3F}, // F6
  {' ', 0x40, 0, 0x41, false, 0x40}, // F7
  {' ', 0x41, 0, 0x42, false, 0x41}, // F8
  {' ', 0x42, 0, 0x43, false, 0x42}, // f9
  {' ', 0x43, 0, 0x44, false, 0x43}, // f10
  {' ', 0x44, 0, 0x57, false, 0x44}, // f11
  {' ', 0x45, 0, 0x58, false, 0x45}, // f12
  {' ', 0x47, 0, 0x46, false, 0x47}, // SCROLL
  {'[', 0x2F, 1, 0x1A, false, 0x2F},
  {' ', 0x53, 0, 0x45, false, 0x53}, // Num Lock
  {'*', 0x55, 1, 0x37, false, 0x55}, // Keypad *
  {'-', 0x56, 1, 0x4A, false, 0x56}, // Keypad -
  {'+', 0x57, 1, 0x4E, false, 0x57}, // Keypad +
  {'.', 0x63, 1, 0x53, false, 0x4C}, // Keypad .
  {'0', 0x62, 1, 0x52, false, 0x49}, // Keypad 0
  {'1', 0x59, 1, 0x4F, false, 0x4D}, // Keypad 1
  {'2', 0x5A, 1, 0x50, false, 0x51}, // Keypad 2
  {'3', 0x5B, 1, 0x51, false, 0x4E}, // Keypad 3
  {'4', 0x5C, 1, 0x4B, false, 0x50}, // Keypad 4
  {'5', 0x5D, 1, 0x4C, false, 0x5D}, // Keypad 5
  {'6', 0x5E, 1, 0x4D, false, 0x4F}, // Keypad 6
  {'7', 0x5F, 1, 0x47, false, 0x4A}, // Keypad 7
  {'8', 0x60, 1, 0x48, false, 0x52}, // Keypad 8
  {'9', 0x61, 1, 0x49, false, 0x4B}, // Keypad 9
  {']', 0x30, 1, 0x1B, false, 0x30},
  {';', 0x33, 1, 0x27, false, 0x33}, 
  {'\'', 0x34, 1, 0x28, false, 0x34},
  {',', 0x36, 1, 0x33, false, 0x36},
  {'.', 0x37, 1, 0x34, false, 0x37},
  {'/', 0x38, 1, 0x35, false, 0x38}
 } ; 

int num_lock = LOW;

#define BUFSIZE 20
unsigned char data_buf[BUFSIZE];
unsigned char state_buf[BUFSIZE];
int write_ptr = 0;

void setup() 
{
#ifdef DEBUG
  Serial.begin(9600) ;
  Serial.println("Starting...");
#endif
  pinMode(clk, INPUT);
  pinMode(data,INPUT);  
  pinMode(led, OUTPUT);
  attachInterrupt(0, clock, FALLING);
}

void dump() {
  int ptr = write_ptr;
  for (int i = 0; i < BUFSIZE; i++) { 
    Serial.write(data_buf[ptr++]);
    ptr = ptr % BUFSIZE;
  }
  Serial.write("\n");
  for (int i = 0; i < BUFSIZE; i++) { 
    Serial.write(state_buf[ptr++]);
    ptr = ptr % BUFSIZE;
  }
  Serial.write("\n");
  for (int i = 0; i < write_ptr; i++) { 
    Serial.write(" ");
  }
  Serial.write("^");  
  
  Serial.write("\nState : ");
  Serial.write(state);
  Serial.write("\nLast Value: ");
  Serial.println(value, HEX);
  Serial.write("\n");
}

unsigned char _read()
{
   if (got_data)
   {
    got_data = 0 ; 
    return value ; 
  } 
  return 0 ; 
}

unsigned long last_dump = 0;
int modifiers = 0;
byte i = 0 ;
void loop() 
{
  unsigned char code = _read();
  if (code != 0) {
    handleKeyEvent(code); 
  }
  
  if (millis() - last_dump > 10000) {
    dump();
    last_dump = millis();
  }
}

void handleKeyEvent(unsigned char code) {  
  unsigned char key_release = code & KEY_RELEASE_MASK;
  Serial.write("keyrelease: ");
  Serial.println(key_release, HEX);
  unsigned char key = code & ~KEY_RELEASE_MASK;
  Serial.write("code : ");
  Serial.println(code, HEX);
  Serial.write("key : ");
  Serial.println(key, HEX);
  int modifier = 0;
  switch (key) {
    case 0x2A: // Left Shift
    case 0x36: // Right Shift
      modifier = MODIFIERKEY_SHIFT;
      break;
    case 0x1D: // Ctrl
      modifier = MODIFIERKEY_CTRL;
      break;
    case 0x38: // Alt
      modifier = MODIFIERKEY_ALT;
      break;
    case 0x45: // Num lock
       num_lock = (key_release == 0) ? num_lock : !num_lock;
       digitalWrite(led, num_lock);
       return;
  }
  
  if (modifier != 0) {
    if (key_release != 0) {
      Serial.write("Removing modifier\n");
      modifiers ^= modifier;
    } else {
      Serial.write("Adding modifier\n");
      modifiers |= modifier;
    }
    Keyboard.set_modifier(modifiers);
    Keyboard.set_key1(0);
    Keyboard.send_now();
  } else {
    int index = get_key(key);
    if (key_release == 0) {
      debug(modifiers, keys[index].character);
      Keyboard.set_modifier(modifiers);
      
      unsigned char scancode = keys[index].usb_make;
      if (num_lock == HIGH) {
        scancode = keys[index].numlock_make;
      }
      Serial.write("pressing key: ");
      Serial.println(scancode, HEX);
      Keyboard.set_key1(scancode);
      Keyboard.send_now();
    } 
    
    if(true || key_release || keys[index].needs_key_up) {
      Serial.write("Key released\n");
      Keyboard.set_modifier(modifiers);
      Keyboard.set_key1(0);
      Keyboard.send_now();
    }
  }
 
#ifdef DEBUG
  if (code != 0) Serial.println(code, HEX) ; 
#endif

}

void debug(int modifiers, unsigned char character) {
  Serial.write("Press: ");
  Serial.write(character);
  Serial.write(" Modifiers: ");
  if ((modifiers & MODIFIERKEY_SHIFT) == MODIFIERKEY_SHIFT) {
    Serial.write("shift ");
  }
  
  if ((modifiers & MODIFIERKEY_CTRL) == MODIFIERKEY_CTRL) {
    Serial.write("ctrl ");
  }
  
  if ((modifiers & MODIFIERKEY_ALT) == MODIFIERKEY_ALT) {
    Serial.write("alt ");
  }
  
  Serial.write("\n");
}

int get_key(int code) {
  for (int i = 0; i < GROUP1_CNT; i++) {  
    if (code == keys[i].xt_make) {
      Serial.write("Pressing key === ");
      Serial.write(keys[i].character);
      Serial.write("\n");
      return i;
    }
  }
  Serial.write("Could not find key for code: ");
  Serial.println(code, HEX);
  return -1;
}

void clock()
{  
   int bit = digitalRead(data);
   data_buf[write_ptr] = (bit == HIGH) ? '1' : '0';
   state_buf[write_ptr] = state;
   write_ptr++;
   write_ptr = write_ptr % BUFSIZE;
   
   if (state == INIT)
   {
     if (bit == HIGH) {
       state = START ; 
       cycles = 0 ;
       got_data = 0 ;
       value = 0 ; 
       return ; 
     } else {
       Serial.write("INIT state, didn't read a HIGH\n");
     }
   }
   if (state == START)
   {
     int read = (bit << cycles);
     value |=  read;
     cycles++ ; 
     if (cycles == 8) {
       got_data = 1; 
       state = STOP ;
     }
     return;  
   }
   if (state == STOP)
   {
     if (bit == LOW)
     {
       state = INIT;
       return; 
     } else {
       Serial.write("STOP state, didn't read a LOW\n");
     }
   }
}
