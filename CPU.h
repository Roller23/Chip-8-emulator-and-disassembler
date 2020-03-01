#ifndef __CPU_
#define __CPU_

#include <stdint.h>

#define MEM_SIZE 4096
#define REGISTER_COUNT 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define STACK_SIZE 16
#define KEYS_COUNT 16
#define INTERPRETER_SIZE 0x200
#define MAX_ROM_SIZE (0xFFF - INTERPRETER_SIZE)

class Chip8 {
  private:
    uint16_t current_opcode;
    uint8_t memory[MEM_SIZE];
    uint8_t registers[REGISTER_COUNT];
    uint16_t program_counter;
    uint16_t index_register;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keys[KEYS_COUNT];
    uint16_t stack[STACK_SIZE];
    uint16_t stack_ptr;
    uint8_t timer_counter;
    volatile bool game_finished;
    void executeOpcode();
    void executeCycle(void);
    int8_t getKey(void);
    void drawScreen(void);
    volatile bool should_draw;
    void checkInput(void);
  public:
    Chip8(void);
    bool loadGame(const char *name);
    void runEmu(void);
    void setKey(uint8_t index);
    void clearKeys(void);
    uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
    int pressed_key;
};

#endif // __CPU_