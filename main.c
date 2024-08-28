#include <16f877A.h>
#use delay(clock=4000000)
#fuses XT,NOWDT,NOPUT,NOLVP,NOCPD,NOPROTECT,NODEBUG,NOBROWNOUT,NOWRT
#define BUZZER PIN_C5
#define RESETPWD PIN_C0
#define OPENSAFE PIN_C1
#use fast_io(b)
#use fast_io(d)
#define use_portb_lcd TRUE
#include <lcd.c>

char pwd[6];
char enteredPassword[6];
int passwordIndex = 0;
int wrongAttempts = 0;
int count = 0;
int start = 0;

enum AUTHSTATE {FIRST_AUTH, NORMAL_AUTH, PASSWORD_AUTH}; // Hangi aþamada olduðunu belirtme ve kontrol deðiþkeni
enum AUTHSTATE currentState = FIRST_AUTH;

#int_EXT
void WrongAttemptsInt() {
   if(wrongAttempts == 4) {
      output_HIGH(BUZZER);
      delay_ms(100);
      output_LOW(BUZZER);
      delay_ms(100);
      sleep();
      wrongAttempts = 0;
   } else {
      output_low(BUZZER);
   }
}

#INT_TIMER0
void GoToSleep() {
   output_toggle(PIN_B0);
}

#int_TIMER1
void TimerCounter() {
   set_timer1(3036);
   count++;
   if(start == 1) {
      if(count >= 60) {
         sleep();
      }
   }
}

void servoRotate(int angle) {
   int16 pulse_width = 0;
   if (angle < 0) angle = 0;
   else if (angle > 360) angle = 360;
   pulse_width = 1000 + (angle * 11);
   setup_ccp1(CCP_PWM);
   setup_timer_2(T2_DIV_BY_16, 255, 1);
   set_pwm1_duty(pulse_width);
   delay_ms(500);
}

void EEPROM_write(int address, int data) { //EEPROM kaydetme fonksiyonu
   write_eeprom(address, data);
}

int EEPROM_read(int address) { // EEPROM okuma fonksiyonu
   return read_eeprom(address);
}

void DeleteButton() { // Girilen deðerleri temizleme
   lcd_gotoxy(1, 2);
   printf(lcd_putc, "                ");
   
   for (int i = 0; i < 6; i++) {
      enteredPassword[i] = 0;
   }
   
   passwordIndex = 0;
}

void NormalAuth() {
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Sifreni Gir     ");
   start = 1;
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);
   set_timer1(3036);
   lcd_gotoxy(1,2);
}

void SaveThePassword() { // Girilen þifreyi EEPROM'a kaydetme
   for(int i = 0; i < 6; i++) {
      pwd[i] = enteredPassword[i]; // Þifreyi enteredPassword'dan al
      EEPROM_write(i, pwd[i]);
   }
   
   currentState = NORMAL_AUTH;
}

void PasswordAuth() { // Kaydedilen þifre ile girilen þifre ayný mý kontrolü
   int passwordMatch = 1;
   
   for (int i = 0; i < 6; i++) {
      pwd[i] = EEPROM_read(i); // EEPROM'dan þifreyi oku
      
      if (enteredPassword[i] != pwd[i]) {
         passwordMatch = 0;  
         break;
      }
   }
   
   lcd_gotoxy(1, 1);
   printf(lcd_putc, "**** SIFRE *****");
   
   if (passwordMatch) {
      lcd_gotoxy(1, 2);
      printf(lcd_putc, "**** DOGRU *****");
      servoRotate(90);
      output_LOW(BUZZER);
   } else {
      lcd_gotoxy(1, 2);
      printf(lcd_putc, "**** YANLIS ****");
      servoRotate(0);
      delay_ms(500);
      wrongAttempts++;
      delay_ms(500);
      NormalAuth();
      lcd_gotoxy(1, 2);
      printf(lcd_putc, " ");
      output_low(BUZZER);
      DeleteButton();
      currentState = NORMAL_AUTH;
   }
}

void FirstAuth() { // Ýlk açýlýþ ekraný (Ýlk þifre kaydetmek için)
   start = 0;
   printf(lcd_putc,"Sifre Kaydet    ");
   lcd_gotoxy(1,2);
}

void DoneButton() { // Deðerleri girdikten sonra tamam butonu
   if (currentState == FIRST_AUTH) {
      lcd_gotoxy(1,2);
      SaveThePassword();
      delay_ms(500);
      NormalAuth();
      currentState = NORMAL_AUTH;
   } else if (currentState == NORMAL_AUTH) {
      currentState = PASSWORD_AUTH;
      PasswordAuth();
   }
}

void TakeInput() { // Kullanýcýdan sayý giriþi
   char keyPressed = 0 ;
   
   output_high(PIN_D0);
   if(input(PIN_D4) == 1) { delay_ms(400); keyPressed = '1'; }
   else if(input(PIN_D5) == 1) { delay_ms(400); keyPressed = '2'; }
   else if(input(PIN_D6) == 1) { delay_ms(400); keyPressed = '3'; }
   output_low(PIN_D0);
   
   output_high(PIN_D1);
   if(input(PIN_D4) == 1) { delay_ms(400); keyPressed = '4'; }
   else if(input(PIN_D5) == 1) { delay_ms(400); keyPressed = '5'; }
   else if(input(PIN_D6) == 1) { delay_ms(400); keyPressed = '6'; }
   output_low(PIN_D1);
   
   output_high(PIN_D2);
   if(input(PIN_D4) == 1) { delay_ms(400); keyPressed = '7'; }
   else if(input(PIN_D5) == 1) { delay_ms(400); keyPressed = '8'; }
   else if(input(PIN_D6) == 1) { delay_ms(400); keyPressed = '9'; }
   output_low(PIN_D2);
   
   output_high(PIN_D3);
   if(input(PIN_D4) == 1) { delay_ms(400); DeleteButton(); }
   else if(input(PIN_D5) == 1) { delay_ms(400); keyPressed = '0'; }
   else if(input(PIN_D6) == 1) { delay_ms(400); DoneButton(); }
   output_low(PIN_D3);
   
   if (keyPressed != 0) {
      if (passwordIndex < 6) { // 6 karakter sýnýrý
         enteredPassword[passwordIndex] = keyPressed;
         passwordIndex++;
      }
      lcd_gotoxy(1, 2);
      for (int i = 0; i < passwordIndex; i++) {
         printf(lcd_putc, "%c", enteredPassword[i]);
      }
      if (currentState == NORMAL_AUTH && passwordIndex >= 6) { // 6 karakter girildiðinde doðrula
         currentState = PASSWORD_AUTH;
         PasswordAuth();
      }
   }
}

void main() {
   setup_psp(PSP_DISABLED);
   setup_timer_0(RTCC_INTERNAL | RTCC_DIV_8);
   setup_adc_ports(NO_ANALOGS);
   setup_adc(ADC_OFF);
   setup_CCP1(CCP_OFF);
   setup_CCP2(CCP_OFF);
   set_tris_b(0x00);
   set_tris_d(0b01110000);
   output_c(0xFF);
   lcd_init();
   enable_interrupts(INT_EXT);
   enable_interrupts(INT_TIMER1);
   enable_interrupts(INT_TIMER0);
   ext_int_edge(0);
   enable_interrupts(GLOBAL);
   FirstAuth();

   while(1) {
      output_b(0x00);
      TakeInput();
   
      if(input(RESETPWD) == 1) {
         setup_timer_1(T1_DISABLED);
         set_timer1(0);
         lcd_gotoxy(1,1);
         DeleteButton();
         currentState = FIRST_AUTH;
         FirstAuth();
      }
      if(input(OPENSAFE) == 1) {
         servoRotate(270);
      }
   }
}

