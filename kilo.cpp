#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <cctype>
#include <winuser.h>
#include <thread>
#include <atomic>
#include <string>

std::atomic<bool> running(true);

DWORD origConsoleModeInput;
DWORD origConsoleModeOutput;
bool shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
bool capsLockState = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
bool leftCtrlPressed = (GetKeyState(VK_LCONTROL) & 0x8000) != 0;
bool rightCtrlPressed = (GetKeyState(VK_RCONTROL) & 0x8000) != 0;


void checkKeyState() {
   while (running) {
        bool currentCapsLockState = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
        if (currentCapsLockState != capsLockState) {
            capsLockState = currentCapsLockState;
            // Handle caps lock state change here
        }

        bool currentShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        if (currentShiftPressed != shiftPressed) {
            shiftPressed = currentShiftPressed;
            // Handle shift key state change here
        }

        bool currentLeftCtrlPressed = (GetKeyState(VK_LCONTROL) & 0x8000) != 0;
        if (currentLeftCtrlPressed != leftCtrlPressed) {
            leftCtrlPressed = currentLeftCtrlPressed;
            // Handle left ctrl key state change here
        }

        bool currentRightCtrlPressed = (GetKeyState(VK_RCONTROL) & 0x8000) != 0;
        if (currentRightCtrlPressed != rightCtrlPressed) {
            rightCtrlPressed = currentRightCtrlPressed;
            // Handle right ctrl key state change here
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for 100 milliseconds to reduce CPU usage
    }
}


void disableRawMode() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hStdin, origConsoleModeInput);
}


void enableRawMode() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    
    GetConsoleMode(hStdin, &origConsoleModeInput);
    atexit(disableRawMode);

    DWORD rawInputMode = origConsoleModeInput;
    rawInputMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(hStdin, rawInputMode);
    
    DWORD fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    SetConsoleMode(hStdin, fdwMode);
}

void setReadTimeouts(HANDLE hStdin) {
    COMMTIMEOUTS timeouts;
    GetCommTimeouts(hStdin, &timeouts);

    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 10;

    SetCommTimeouts(hStdin, &timeouts);
}


void saveOutputToFile(const char* filename, CHAR_INFO* buffer, CONSOLE_SCREEN_BUFFER_INFO csbi) {
    std::ofstream outfile(filename);
    for (int y = 0; y < csbi.dwSize.Y; y++) {
        for (int x = 0; x < csbi.dwSize.X; x++) {
            outfile << buffer[y * csbi.dwSize.X + x].Char.AsciiChar;
        }
        outfile << '\n';
    }

    outfile.close();
}



   


int main() {
    std::thread keyChecker(checkKeyState);

    system("cls");

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    INPUT_RECORD irInBuf[128];
    DWORD cNumRead;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int counter;
    char key_press_c;
    
    enableRawMode();
    setReadTimeouts(hStdin);
    
    while (1) {
        if (!ReadConsoleInputA(hStdin, irInBuf, 128, &cNumRead)) {
            printf("Error reading console input\n");
            return 1;
        }

        for (counter = 0; counter < cNumRead; counter++) {
            if (irInBuf[counter].EventType == KEY_EVENT) {
                KEY_EVENT_RECORD ker = irInBuf[counter].Event.KeyEvent;
                
                if (ker.bKeyDown) {
                    if (leftCtrlPressed || rightCtrlPressed) {
						if (ker.wVirtualKeyCode == 'Q') return 0;
						if (ker.wVirtualKeyCode == 'S'){
                            GetConsoleScreenBufferInfo(hStdout, &csbi);

                            int bufferSize = csbi.dwSize.X * csbi.dwSize.Y;
                            CHAR_INFO* buffer = (CHAR_INFO*)malloc(bufferSize * sizeof(CHAR_INFO));
                            SMALL_RECT readRegion = {0, 0, csbi.dwSize.X - 1, csbi.dwSize.Y - 1};

                            COORD bufferCoord = {0, 0};
                            COORD bufferSizeCoord = {csbi.dwSize.X, csbi.dwSize.Y};
                            ReadConsoleOutput(hStdout, buffer, bufferSizeCoord, bufferCoord, &readRegion);

                            disableRawMode();
                            system("cls");

                            std::string filename;
                            std::cout << "Enter the name of the file: ";
                            std::cin >> filename;

                            filename = filename.append(".txt");
							saveOutputToFile(filename.c_str(), buffer, csbi);
                            

                            WriteConsoleOutput(hStdout, buffer, bufferSizeCoord, bufferCoord, &readRegion);
                            free(buffer);

                            enableRawMode();
						}
						if (ker.wVirtualKeyCode == 'C') continue;
						if (ker.wVirtualKeyCode == 'V') continue;
                    }
                    else{
						GetConsoleScreenBufferInfo(hStdout, &csbi);
						switch (ker.wVirtualKeyCode) {
							case VK_UP:
								if (csbi.dwCursorPosition.Y > 0) {
                                    csbi.dwCursorPosition.Y--;
                                    SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);
                                }
								break;
							case VK_DOWN:
								csbi.dwCursorPosition.Y++;
                                SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);
								break;
							case VK_LEFT:
								if (csbi.dwCursorPosition.X > 0) {
                                    csbi.dwCursorPosition.X--;
                                    SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);
                                }
								break;
							case VK_RIGHT:
								csbi.dwCursorPosition.X++;
                                SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);
								break;
							case VK_BACK:
								if (csbi.dwCursorPosition.X > 0) {
									csbi.dwCursorPosition.X--;
									SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);
                                    printf(" ");
									SetConsoleCursorPosition(hStdout, csbi.dwCursorPosition);
								}
								break;
							case VK_RETURN:
								printf("\r\n");
								break;
							default:
								key_press_c = ker.wVirtualKeyCode;
								if(shiftPressed || capsLockState){
                                    printf("%c", key_press_c);
								}
								else{
                                    printf("%c", (char)tolower(key_press_c));
								}
								break;
						}
					}
                }
            
            }
        }
    }

    
    return 0;
}
