#include <syscalls.h>
#include <tipos.h>
// #include <lib_str.h>

//para guardar el comando que voy escribiendo
char command[100];
//indice para moverse en command
int idx;

int main(void) {
  // char buff[10];
  char* ps1 = "kernel@pso: ";
  char* enter = "\n";
  idx = 0;
  char msg[3];
  int fd = open("/console", 0x3);

  write(fd, ps1, 12);
  while (1) {
    read(fd, msg, 1);
    if (msg[0] == '\n') {
      //!todo mandar a ejecutar...

      write(fd, enter, 1);
      write(fd, ps1, 12);

    } else {
      write(fd, msg, 1);
      if (msg[0] != 0x0e) {
        command[idx++] = msg[0];
      }
    }

  }

  // void* p = palloc();
  return 0;
}
