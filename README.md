# Secure-Digital-Safe-with-Password-Protection
Digital Safe with PIC16F877A


How it's work:

The LCD screen will display “Save Password,” and the user will be prompted to enter a 5-digit password. The entered password will then be saved to the EEPROM, and the system will move to the second stage. In the second stage, the LCD screen will display “Enter Your Password,” and Timer1 will be started as a 30-second countdown. If the password is not entered within 30 seconds, the Timer1 interrupt will trigger, causing the processor to enter “sleep” mode. After 5 seconds, the processor will wake up from “sleep” mode, the LCD screen will again display “Enter Your Password,” and Timer1 will restart the countdown. If the entered password matches the saved password, the LCD screen will display “PASSWORD CORRECT,” and the servo motor will rotate from +90 degrees to -90 degrees (where the cover is open at +90 degrees and closed at -90 degrees). If the entered password does not match the saved password, the LCD screen will display “PASSWORD INCORRECT,” the buzzer will sound once and then stop, and the external interrupt (INT_EXT) will increase the incorrect attempt count by one. The system will then return to the “Enter Your Password” screen. If the password is entered incorrectly 4 times, the external interrupt will trigger, causing the processor to enter “sleep” mode for 5 seconds. Additionally, there are two buttons connected to the circuit. The first button resets the password saved in the EEPROM, returning the system to the initial stage of “Save Password.” The second button rotates the servo motor to -90 degrees (to close the cover) and then returns to the second stage of “Enter Your Password.”



NOTE: Servo's PWM is not working correctly.
