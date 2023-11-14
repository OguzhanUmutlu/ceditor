#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#ifndef EDITOR_MAIN_H
#define EDITOR_MAIN_H

#define CMD_A 0x01
#define CMD_B 0x02
#define CMD_C 0x03
#define CMD_D 0x04
#define CMD_E 0x05
#define CMD_F 0x06
#define CMD_G 0x07
// reserved to backspace. #define CMD_H 0x08
#define CMD_I 0x09
#define CMD_J 0x0A
#define CMD_K 0x0B
#define CMD_L 0x0C
#define CMD_M 0x0D
#define CMD_N 0x0E
#define CMD_O 0x0F
#define CMD_P 0x10
#define CMD_Q 0x11
#define CMD_R 0x12
#define CMD_S 0x13
#define CMD_T 0x14
#define CMD_U 0x15
// Not possible, since it pastes stuff. #define CMD_V 0x16
#define CMD_W 0x17
#define CMD_X 0x18
#define CMD_Y 0x19
#define CMD_Z 0x1A
#define ARROW_BEGIN 0xE0
#define BACKSPACE '\b'
#define ENTER1 0x0D
#define ENTER2 0x0A

#define C_RAW(a) "\x1b[" #a "m"
#define C_RGB(r, g, b) "\x1b[38;2;" #r ";" #g ";" #b ";m"
#define CB_RGB(r, g, b) "\x1b[48;2;" #r ";" #g ";" #b ";m"

#define C_GRAY C_RGB(128, 128, 128)

#define C_BLACK C_RAW(30)
#define C_RED C_RAW(31)
#define C_GREEN C_RAW(32)
#define C_YELLOW C_RAW(33)
#define C_BLUE C_RAW(34)
#define C_MAGENTA C_RAW(35)
#define C_CYAN C_RAW(36)
#define C_WHITE C_RAW(37)

#define C_BLACK_B C_RAW(90)
#define C_RED_B C_RAW(91)
#define C_GREEN_B C_RAW(92)
#define C_YELLOW_B C_RAW(93)
#define C_BLUE_B C_RAW(94)
#define C_MAGENTA_B C_RAW(95)
#define C_CYAN_B C_RAW(96)
#define C_WHITE_B C_RAW(97)

#define CB_BLACK C_RAW(40)
#define CB_RED C_RAW(41)
#define CB_GREEN C_RAW(42)
#define CB_YELLOW C_RAW(43)
#define CB_BLUE C_RAW(44)
#define CB_MAGENTA C_RAW(45)
#define CB_CYAN C_RAW(46)
#define CB_WHITE C_RAW(47)

#define CB_BLACK_B C_RAW(100)
#define CB_RED_B C_RAW(101)
#define CB_GREEN_B C_RAW(102)
#define CB_YELLOW_B C_RAW(103)
#define CB_BLUE_B C_RAW(104)
#define CB_MAGENTA_B C_RAW(105)
#define CB_CYAN_B C_RAW(106)
#define CB_WHITE_B C_RAW(107)

#define C_RESET C_RAW(0)
#define C_BOLD C_RAW(1)
#define C_DIM C_RAW(2)
#define C_ITALIC C_RAW(3)
#define C_UNDERLINE C_RAW(4)
#define C_INVERSE C_RAW(7)
#define C_HIDDEN C_RAW(8)
#define C_STRIKETHROUGH C_RAW(8)

#ifdef _WIN32
#define CLEAR_SCREEN "cls"

#include <conio.h>

#define GET_NEXT_CHARACTER(vr) int vr = _getch()
#define HANDLE_KEYS(fn) while (1) if (_kbhit()) fn(_getch());
#else
#define CLEAR_SCREEN "clear"
#include <curses.h>
#define GET_NEXT_CHARACTER(vr) int vr = getch(); if(vr == ERR) vr = 0
#define HANDLE_KEYS(fn) int ky; while (1) if ((ky = getch()) != ERR) fn(ky);
#endif

#define uint size_t
#define chr char

#endif //EDITOR_MAIN_H

#pragma clang diagnostic pop