#include <cstdint>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>

#define MEM_SIZE 4096
#define OPCODE_SIZE 2

class Disassemlber {
  private:
    uint16_t current_opcode;
    uint8_t memory[MEM_SIZE];
    uint16_t program_counter;
    std::FILE *file;
    std::string filename;
    void incrementPC(void) {
      this->program_counter += OPCODE_SIZE;
    }
    void getNextOpcode(void) {
      this->current_opcode = (memory[program_counter] << 8) | memory[program_counter + 1];
      this->incrementPC();
    }
  public:
    Disassemlber(std::string filename) {
      this->program_counter = 0;
      std::memset(this->memory, 0, MEM_SIZE);
      this->filename = filename;
      this->file = std::fopen(filename.c_str(), "rb");
      if (this->file == NULL) {
        std::cout << "Couldn't open " << filename << "\n";
        std::exit(0);
      }
      std::fseek(this->file, 0, SEEK_END);
      int file_size = std::ftell(this->file);
      std::fseek(this->file, 0, SEEK_SET);
      if (file_size > MEM_SIZE) {
        std::fclose(this->file);
        std::cout << "File is too big\n";
        std::exit(0);
      }
      int bytes_read = std::fread(this->memory, 1, file_size, this->file);
      std::fclose(this->file);
      if (bytes_read != file_size) {
        std::cout << "Error reading file\n";
        std::exit(0);
      }
    }
    void disassemble(void) {
      this->current_opcode = 0xFFFF;
      int index = this->filename.rfind(".");
      char *str_ptr = strdup(this->filename.c_str());
      str_ptr[index] = 0;
      std::string output_filename = str_ptr;
      output_filename.append(".chip8");
      std::FILE *output_file = std::fopen(output_filename.c_str(), "w");
      if (output_file == NULL) {
        std::cout << "Couldn't create output file!\n";
        std::exit(0);
      }
      while (true) {
        this->getNextOpcode();
        if (this->current_opcode == 0x0000) {
          break;
        }
        uint16_t opcode = this->current_opcode;
        std::printf("opcode: 0x%.4X\n", opcode);
        if (opcode == 0x00E0) {
          std::fprintf(output_file, "CLS\n");
          continue;
        }
        if (opcode == 0x00EE) {
          std::fprintf(output_file, "RTS\n");
          continue;
        }
        if ((opcode & 0xF000) == 0x1000) {
          uint16_t jump_address = opcode & 0x0FFF;
          std::fprintf(output_file, "JMP  0x%.4X\n", jump_address);
          continue;
        }
        if ((opcode & 0xF000) == 0x2000) {
          uint16_t routine_address = opcode & 0x0FFF;
          std::fprintf(output_file, "JSR  0x%.4X\n", routine_address);
          continue;
        }
        if ((opcode & 0xF000) == 0x3000) {
          uint8_t register_index = (opcode & 0x0F00) >> 8;
          uint8_t value = opcode & 0x00FF;
          std::fprintf(output_file, "SKEQ  V%hu, %hu\n", register_index, value);
          continue;
        }
        if ((opcode & 0xF000) == 0x4000) {
          uint8_t register_index = (opcode & 0x0F00) >> 8;
          uint8_t value = opcode & 0x00FF;
          std::fprintf(output_file, "SKNE  V%hu, %hu\n", register_index, value);
          continue;
        }
        if ((opcode & 0xF000) == 0x5000) {
          uint8_t register_index1 = (opcode & 0x0F00) >> 8;
          uint8_t register_index2 = (opcode & 0x00F0) >> 4;
          std::fprintf(output_file, "SKEQ  V%hu, V%hu\n", register_index1, register_index2);
          continue;
        }
        if ((opcode & 0xF000) == 0x6000) {
          uint8_t register_index = (opcode & 0x0F00) >> 8;
          uint8_t value = opcode & 0x00FF;
          std::fprintf(output_file, "MOV  V%hu, %hu\n", register_index, value);
          continue;
        }
        if ((opcode & 0xF000) == 0x7000) {
          uint8_t register_index = (opcode & 0x0F00) >> 8;
          uint8_t value = opcode & 0x00FF;
          std::fprintf(output_file, "ADD  V%hu, %hu\n", register_index, value);
          continue;
        }
        if ((opcode & 0xF000) == 0x8000) {
          if ((opcode & 0x000F) == 0x0000) {
            uint8_t register_index1 = (opcode & 0x0F00) >> 8;
            uint8_t register_index2 = (opcode & 0x00F0) >> 4;
            std::fprintf(output_file, "MOV  V%hu, V%hu\n", register_index1, register_index2);
            continue;
          }
          if ((opcode & 0x000F) == 0x0001) {
            uint8_t register_index1 = (opcode & 0x0F00) >> 8;
            uint8_t register_index2 = (opcode & 0x00F0) >> 4;
            std::fprintf(output_file, "OR  V%hu, V%hu\n", register_index1, register_index2);
            continue;
          }
          if ((opcode & 0x000F) == 0x0002) {
            uint8_t register_index1 = (opcode & 0x0F00) >> 8;
            uint8_t register_index2 = (opcode & 0x00F0) >> 4;
            std::fprintf(output_file, "AND  V%hu, V%hu\n", register_index1, register_index2);
            continue;
          }
          if ((opcode & 0x000F) == 0x0003) {
            uint8_t register_index1 = (opcode & 0x0F00) >> 8;
            uint8_t register_index2 = (opcode & 0x00F0) >> 4;
            std::fprintf(output_file, "XOR  V%hu, V%hu\n", register_index1, register_index2);
            continue;
          }
          if ((opcode & 0x000F) == 0x0004) {
            uint8_t register_index1 = (opcode & 0x0F00) >> 8;
            uint8_t register_index2 = (opcode & 0x00F0) >> 4;
            std::fprintf(output_file, "ADD  V%hu, V%hu\n", register_index1, register_index2);
            continue;
          }
          if ((opcode & 0x000F) == 0x0005) {
            uint8_t register_index1 = (opcode & 0x0F00) >> 8;
            uint8_t register_index2 = (opcode & 0x00F0) >> 4;
            std::fprintf(output_file, "SUB  V%hu, V%hu\n", register_index1, register_index2);
            continue;
          }
          if ((opcode & 0x000F) == 0x0006) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "SHR  V%hu\n", register_index);
            continue;
          }
          if ((opcode & 0x000F) == 0x0007) {
            uint8_t register_index1 = (opcode & 0x0F00) >> 8;
            uint8_t register_index2 = (opcode & 0x00F0) >> 4;
            std::fprintf(output_file, "RSB  V%hu, V%hu\n", register_index1, register_index2);
            continue;
          }
          if ((opcode & 0x000F) == 0x000E) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "SHL  V%hu\n", register_index);
            continue;
          }
        }
        if ((opcode & 0xF000) == 0x9000) {
          uint8_t register_index1 = (opcode & 0x0F00) >> 8;
          uint8_t register_index2 = (opcode & 0x00F0) >> 4;
          std::fprintf(output_file, "SKNE  V%hu, V%hu\n", register_index1, register_index2);
          continue;
        }
        if ((opcode & 0xF000) == 0xA000) {
          uint16_t address = opcode & 0x0FFF;
          std::fprintf(output_file, "MVI  0x%.4X\n", address);
          continue;
        }
        if ((opcode & 0xF000) == 0xB000) {
          uint16_t address = opcode & 0x0FFF;
          std::fprintf(output_file, "JMI  0x%.4X\n", address);
          continue;
        }
        if ((opcode & 0xF000) == 0xC000) {
          uint8_t register_index = (opcode & 0x0F00) >> 8;
          uint8_t value = opcode & 0x00FF;
          std::fprintf(output_file, "RAND  V%hhu, %hhu\n", register_index, value);
          continue;
        }
        if ((opcode & 0xF000) == 0xD000) {
          uint8_t register_index1 = (opcode & 0x0F00) >> 8;
          uint8_t register_index2 = (opcode & 0x00F0) >> 4;
          uint8_t height = opcode & 0x000F;
          std::fprintf(output_file, "SPRITE  V%hhu, V%hhu, %hhu\n", register_index1, register_index2, height);
          continue;
        }
        if ((opcode & 0xF000) == 0xE000) {
          if ((opcode & 0x00FF) == 0x009E) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "SKPR  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x00A1) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "SKUP  V%hhu\n", register_index);
            continue;
          }
        }
        if ((opcode & 0xF000) == 0xF000) {
          if ((opcode & 0x00FF) == 0x0007) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "GDELAY  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x000A) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "KEY  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x0015) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "SDELAY  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x0018) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "SSOUND  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x001E) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "ADI  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x0029) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "FONT  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x0033) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "BCD  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x0055) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "STR  V%hhu\n", register_index);
            continue;
          }
          if ((opcode & 0x00FF) == 0x0065) {
            uint8_t register_index = (opcode & 0x0F00) >> 8;
            std::fprintf(output_file, "LDR  V%hhu\n", register_index);
            continue;
          }
        }
      }
      std::cout << "Done!\n";
      std::fclose(output_file);
    }
};

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Not enough arguments\n";
    std::exit(0);
  }
  Disassemlber(argv[1]).disassemble();
  return 0;
}