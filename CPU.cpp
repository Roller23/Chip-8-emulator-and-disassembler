#include "CPU.h"
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
#include <stdarg.h>
#include <ncurses.h>

#define MS 1000
#define SEC (MS * 1000)
#define DEBUG 0
#define CYCLES_PER_SECOND 500

#define WHITE_COLOR 1
#define BLACK_COLOR 2
#define CHANGE_COLOR(COLOR) attron(COLOR_PAIR(COLOR))

static void log(const char *format, ...) {
  #if DEBUG == 1
    va_list list;
    va_start(list, format);
    std::vprintf(format, list);
    va_end(list);
  #else
  #endif
}

static const char keyboard[] = {
  '1', '2', '3', '4',
  'q', 'w', 'e', 'r',
  'a', 's', 'd', 'f',
  'z', 'x', 'c', 'v'
};

const static uint8_t fontset[] = { 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::checkInput(void) {
  log("Key = %d\n", pressed_key);
  if (pressed_key == ERR) {
    this->clearKeys();
    return;
  }
  for (int i = 0; i < sizeof(keyboard); i++) {
    if (keyboard[i] == pressed_key) {
      this->clearKeys();
      this->setKey(i);
      break;
    }
  }
}

void Chip8::drawScreen(void) {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      if (this->screen[(y * SCREEN_WIDTH) + x] == 0) {
        CHANGE_COLOR(BLACK_COLOR);
        mvprintw(y, x * 2, "  ");
      } else {
        CHANGE_COLOR(WHITE_COLOR);
        mvprintw(y, x * 2, "  ");
      }
    }
  }
  refresh();
  this->should_draw = false;
}

Chip8::Chip8(void) {
  this->delay_timer = 0;
  this->sound_timer = 0;
  this->program_counter = 0x200;
  this->current_opcode = 0;
  this->index_register = 0;
  this->stack_ptr = 0;
  this->timer_counter = 0;
  this->game_finished = false;
  this->should_draw = false;
  this->clearKeys();
  std::memset(this->memory, 0, MEM_SIZE);
  std::memset(this->registers, 0, REGISTER_COUNT);
  std::memset(this->stack, 0, STACK_SIZE * sizeof(uint16_t));
  std::memset(this->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
  std::srand(std::time(0));
  for (int i = 0; i < 80; i++) {
    this->memory[i] = fontset[i];
  }
  initscr();
  cbreak();
  noecho();
  start_color();
  init_pair(WHITE_COLOR, COLOR_BLACK, COLOR_WHITE);
  init_pair(BLACK_COLOR, COLOR_WHITE, COLOR_BLACK);
  timeout(0);
}

void Chip8::executeCycle(void) {
  // Fetch the next opcode
  uint8_t first_byte = this->memory[this->program_counter];
  uint8_t second_byte = this->memory[this->program_counter + 1];
  this->current_opcode = (first_byte << 8) | second_byte;

  // Execute it
  this->executeOpcode();
  if (this->timer_counter == 0) {
    // update timers at 60Hz
    if (this->delay_timer > 0) {
      this->delay_timer--;
    }
    if (this->sound_timer > 0) {
      beep();
      this->sound_timer--;
    }
  }
  this->timer_counter = (this->timer_counter + 1) % 10;
}

void Chip8::setKey(uint8_t index) {
  this->keys[index] = 1;
}

void Chip8::clearKeys(void) {
  std::memset(this->keys, 0, KEYS_COUNT);
}

int8_t Chip8::getKey(void) {
  for (int i = 0; i < KEYS_COUNT; i++) {
    if (this->keys[i]) {
      return i;
    }
  }
  return -1;
}

void Chip8::runEmu(void) {
  int ERRs = 0;
  while (!this->game_finished) {
    int key = getch();
    if (key == ERR) {
      ERRs++;
    } else {
      ERRs = 0;
      pressed_key = key;
    }
    if (ERRs == 10) {
      pressed_key = ERR;
    }
    if (this->should_draw) {
      this->drawScreen();
    }
    this->checkInput();
    this->executeCycle();
    usleep(SEC / CYCLES_PER_SECOND); // 500Hz
  }
}

bool Chip8::loadGame(const char *name) {
  log("Loading %s...\n", name);
  std::FILE *game_file = std::fopen(name, "rb");
  if (game_file == NULL) {
    log("Couldn't open %s!\n", name);
    return false;
  }
  std::fseek(game_file, 0, SEEK_END);
  int size = std::ftell(game_file);
  if (size > MAX_ROM_SIZE) {
    log("%s exceeds maximum ROM size\n", name);
    std::fclose(game_file);
    return false;
  }
  std::fseek(game_file, 0, SEEK_SET);
  uint8_t game_buffer[MAX_ROM_SIZE + 1];
  std::memset(game_buffer, 0, MAX_ROM_SIZE + 1);
  int bytes_read = std::fread(game_buffer, 1, size, game_file);
  if (bytes_read != size) {
    log("Error reading %s\n", name);
    std::fclose(game_file);
    return false;
  }
  std::fclose(game_file);
  for (int i = 0; i < size; i++) {
    this->memory[INTERPRETER_SIZE + i] = game_buffer[i];
  }
  log("Game loaded to memory\n");
  return true;
}

void Chip8::executeOpcode() {
  uint16_t opcode = this->current_opcode;
  if (opcode == 0x0000) {
    // no more instructions to execute
    this->game_finished = true;
    return;
  }
  log("Opcode: 0x%.4X\n", opcode);
  if (opcode == 0x00E0) {
    // clear the display
    std::memset(this->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    this->program_counter += 2;
    log(" Clearing display\n");
    return;
  }
  if (opcode == 0x00EE) {
    // return from subroutine
    this->program_counter = this->stack[--this->stack_ptr] + 2;
    log(" Returning from subroutine\n");
    return;
  }
  if ((opcode & 0xF000) == 0x1000) {
    // sets PC to 0x0NNN
    uint16_t jump_address = opcode & 0x0FFF;
    this->program_counter = jump_address;
    log(" Jump to 0x%.4X\n", jump_address);
    return;
  }
  if ((opcode & 0xF000) == 0x2000) {
    // call to a subroutine at 0x0NNN
    uint16_t routine_address = opcode & 0x0FFF;
    this->stack[this->stack_ptr] = this->program_counter;
		this->stack_ptr++;
    this->program_counter = routine_address;
    log(" Call to a subroutine at 0x%.4X\n", routine_address);
    return;
  }
  if ((opcode & 0xF000) == 0x3000) {
    uint8_t register_index = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
    if (this->registers[register_index] == value) {
      this->program_counter += 4;
    } else {
      this->program_counter += 2;
    }
    log(" Skip instruction if V[%hhu] == %hhu\n", register_index, value);
    return;
  }
  if ((opcode & 0xF000) == 0x4000) {
    uint8_t register_index = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
    if (this->registers[register_index] != value) {
      this->program_counter += 4;
    } else {
      this->program_counter += 2;
    }
    log(" Skip instruction if V[%hhu] != %hhu\n", register_index, value);
    return;
  }
  if ((opcode & 0xF000) == 0x5000) {
    uint8_t register_index1 = (opcode & 0x0F00) >> 8;
    uint8_t register_index2 = (opcode & 0x00F0) >> 4;
    if (this->registers[register_index1] == this->registers[register_index2]) {
      this->program_counter += 4;
    } else {
      this->program_counter += 2;
    }
    log(" Skip instruction if V[%hhu] == V[%hhu]\n", register_index1, register_index2);
    return;
  }
  if ((opcode & 0xF000) == 0x6000) {
    uint8_t register_index = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
    this->registers[register_index] = value;
    this->program_counter += 2;
    log(" Set V[%hhu] to %hhu\n", register_index, value);
    return;
  }
  if ((opcode & 0xF000) == 0x7000) {
    uint8_t register_index = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
    this->registers[register_index] += value;
    this->program_counter += 2;
    log(" Add %hhu to V[%hhu]\n", value, register_index);
    return;
  }
  if ((opcode & 0xF000) == 0x8000) {
    if ((opcode & 0x000F) == 0x0000) {
      uint8_t register_index1 = (opcode & 0x0F00) >> 8;
      uint8_t register_index2 = (opcode & 0x00F0) >> 4;
      this->registers[register_index1] = this->registers[register_index2];
      this->program_counter += 2;
      log(" V[%hhu] = V[%hhu]\n", register_index1, register_index2);
      return;
    }
    if ((opcode & 0x000F) == 0x0001) {
      uint8_t register_index1 = (opcode & 0x0F00) >> 8;
      uint8_t register_index2 = (opcode & 0x00F0) >> 4;
      this->registers[register_index1] |= this->registers[register_index2];
      this->program_counter += 2;
      log(" V[%hhu] |= V[%hhu]\n", register_index1, register_index2);
      return;
    }
    if ((opcode & 0x000F) == 0x0002) {
      uint8_t register_index1 = (opcode & 0x0F00) >> 8;
      uint8_t register_index2 = (opcode & 0x00F0) >> 4;
      this->registers[register_index1] &= this->registers[register_index2];
      this->program_counter += 2;
      log(" V[%hhu] &= V[%hhu]\n", register_index1, register_index2);
      return;
    }
    if ((opcode & 0x000F) == 0x0003) {
      uint8_t register_index1 = (opcode & 0x0F00) >> 8;
      uint8_t register_index2 = (opcode & 0x00F0) >> 4;
      this->registers[register_index1] ^= this->registers[register_index2];
      this->program_counter += 2;
      log(" V[%hhu] ^= V[%hhu]\n", register_index1, register_index2);
      return;
    }
    if ((opcode & 0x000F) == 0x0004) {
      uint8_t register_index1 = (opcode & 0x0F00) >> 8;
      uint8_t register_index2 = (opcode & 0x00F0) >> 4;
      uint8_t value1 = this->registers[register_index1];
      uint8_t value2 = this->registers[register_index2];
      this->registers[0xF] = ((0x00FF - value1) < value2); // set carry
      this->registers[register_index1] += value2;
      this->program_counter += 2;
      log(" V[%hhu] += V[%hhu]\n", register_index1, register_index2);
      return;
    }
    if ((opcode & 0x000F) == 0x0005) {
      uint8_t register_index1 = (opcode & 0x0F00) >> 8;
      uint8_t register_index2 = (opcode & 0x00F0) >> 4;
      uint8_t value1 = this->registers[register_index1];
      uint8_t value2 = this->registers[register_index2];
      this->registers[0xF] = (value1 > value2); // set borrow
      this->registers[register_index1] -= value2;
      this->program_counter += 2;
      log(" V[%hhu] -= V[%hhu]\n", register_index1, register_index2);
      return;
    }
    if ((opcode & 0x000F) == 0x0006) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      this->registers[0xF] = this->registers[register_index] & 0x01;
      this->registers[register_index] >>= 1;
      this->program_counter += 2;
      log(" V[0xF] = V[%hhu] & 0x01 and shift by 1\n", register_index);
      return;
    }
    if ((opcode & 0x000F) == 0x0007) {
      uint8_t register_index1 = (opcode & 0x0F00) >> 8;
      uint8_t register_index2 = (opcode & 0x00F0) >> 4;
      uint8_t value1 = this->registers[register_index1];
      uint8_t value2 = this->registers[register_index2];
      this->registers[0xF] = (value1 < value2); // set borrow
      this->registers[register_index1] = value2 - value1;
      this->program_counter += 2;
      log(" V[%hhu] = %hhu - %hhu\n", register_index1, value1, value2);
      return;
    }
    if ((opcode & 0x000F) == 0x000E) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      this->registers[0xF] = this->registers[register_index] >> 7;
      this->registers[register_index] <<= 1;
      this->program_counter += 2;
      log(" V[0xF] = V[%hhu] >> 7 and shift by 1\n", register_index);
      return;
    }
  }
  if ((opcode & 0xF000) == 0x9000) {
    uint8_t register_index1 = (opcode & 0x0F00) >> 8;
    uint8_t register_index2 = (opcode & 0x00F0) >> 4;
    if (this->registers[register_index1] != this->registers[register_index2]) {
      this->program_counter += 4;
    } else {
      this->program_counter += 2;
    }
    log(" Skip instruction if V[%hhu] != V[%hhu]\n", register_index1, register_index2);
    return;
  }
  if ((opcode & 0xF000) == 0xA000) {
    uint16_t address = opcode & 0x0FFF;
    this->index_register = address;
    this->program_counter += 2;
    log(" Set index register to 0x%.4X\n", address);
    return;
  }
  if ((opcode & 0xF000) == 0xB000) {
    uint16_t jump_address = opcode & 0x0FFF;
    this->program_counter = this->registers[0] + jump_address;
    log(" Set PC to 0x%.4X + 0x%.4X\n", jump_address, this->registers[0]);
    return;
  }
  if ((opcode & 0xF000) == 0xC000) {
    uint8_t register_index = (opcode & 0x0F00) >> 8;
    uint8_t value = opcode & 0x00FF;
    this->registers[register_index] = (rand() % 0xFF) & value;
    this->program_counter += 2;
    log(" rand()\n");
    return;
  }
  if ((opcode & 0xF000) == 0xD000) {
    uint8_t register_index1 = (opcode & 0x0F00) >> 8;
    uint8_t register_index2 = (opcode & 0x00F0) >> 4;
    uint8_t x = registers[register_index1];
    uint8_t y = registers[register_index2];
    uint8_t height = opcode & 0x000F;
    registers[0xF] = 0x0;
    for (int yline = 0; yline < height; yline++) {
      uint16_t pixel = memory[index_register + yline];
      for (int xline = 0; xline < 8; xline++) {
        if ((pixel & (0x80 >> xline)) != 0) {
          int index = (x + xline + ((y + yline) * 64)) % (32 * 64);
          if (screen[index] == 1) {
            registers[0xF] = 0x1;
          }
          screen[index] ^= 1;
        }
      }
    }
    this->should_draw = true;
    this->program_counter += 2;
    log(" DRAW\n");
    return;
  }
  if ((opcode & 0xF000) == 0xE000) {
    if ((opcode & 0x00FF) == 0x009E) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      if (keys[registers[register_index]]) {
        this->program_counter += 4;
      } else {
        this->program_counter += 2;
      }
      log(" Checking if key pressed\n");
      return;
    }
    if ((opcode & 0x00FF) == 0x00A1) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      if (!keys[registers[register_index]]) {
        this->program_counter += 4;
      } else {
        this->program_counter += 2;
      }
      log(" Checking if key not pressed\n");
      return;
    }
  }
  if ((opcode & 0xF000) == 0xF000) {
    if ((opcode & 0x00FF) == 0x0007) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      this->registers[register_index] = this->delay_timer;
      this->program_counter += 2;
      log(" V[%hhu] = delay\n", register_index);
      return;
    }
    if ((opcode & 0x00FF) == 0x000A) {
      log(" Waiting for a keypress\n");
      int8_t key = this->getKey();
      if (key == -1) {
        // no key pressed
        return;
      }
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      this->registers[register_index] = key;
      this->program_counter += 2;
      return;
    }
    if ((opcode & 0x00FF) == 0x0015) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      this->delay_timer = this->registers[register_index];
      this->program_counter += 2;
      log(" Delay timer set to %hhu\n", this->delay_timer);
      return;
    }
    if ((opcode & 0x00FF) == 0x0018) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      this->sound_timer = this->registers[register_index];
      this->program_counter += 2;
      log(" Sound timer set to %hhu\n", this->sound_timer);
      return;
    }
    if ((opcode & 0x00FF) == 0x001E) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      uint8_t value = this->registers[register_index];
      this->registers[0xF] = this->index_register + value > 0xFFF;
      this->index_register += value;
      this->program_counter += 2;
      log(" Index += V[%hhu]\n", register_index);
      return;
    }
    if ((opcode & 0x00FF) == 0x0029) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      uint8_t value = this->registers[register_index];
      this->index_register = value * 0x5;
      this->program_counter += 2;
      return;
    }
    if ((opcode & 0x00FF) == 0x0033) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      memory[index_register] = registers[register_index] / 100;
      memory[index_register + 1] = (registers[register_index] / 10) % 10;
      memory[index_register + 2] = (registers[register_index] % 100) % 10;
      this->program_counter += 2;
      log(" BCD\n");
      return;
    }
    if ((opcode & 0x00FF) == 0x0055) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      for (int i = 0; i <= register_index; i++) {
        memory[index_register + i] = registers[i];
      }
      // index_register += register_index + 1;
      this->program_counter += 2;
      log(" Reg dump\n");
      return;
    }
    if ((opcode & 0x00FF) == 0x0065) {
      uint8_t register_index = (opcode & 0x0F00) >> 8;
      for (int i = 0; i <= register_index; ++i) {
        registers[i] = memory[index_register + i];
      }
      // index_register += register_index + 1;
      this->program_counter += 2;
      log(" Reg load\n");
      return;
    }
  }
  log(" Unknown opcode\n");
  this->program_counter += 2;
}