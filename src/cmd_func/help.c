#include "../types.h"

#include <stdio.h>

rc cmd_func_help(fat *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 1) return ERR_ARGC;
    fprintf(stdout, "Commands:"
         "\n  cat      <file>          Print the contents of a file."
         "\n  cd       [<dir>]         Change CWD to a directory. CWD set to home if directory is not specified."
         "\n  cp       <src> <dest>    Copy a source to a destination."
         "\n  format   <size>          Format the file system to a size. Has to be from 2kB to 1GB."
         "\n  incp     <src> <dest>    Copy a source from the disk to a destination on the file system."
         "\n  info     <file/dir>      Print cluster info about the file or directory."
         "\n  load     <file>          Load and execute a file from the disk."
         "\n  ls       [<dir>]         List the contents of a directory. List current directory if not specified."
         "\n  mkdir    <dest>          Create a new directory at destination."
         "\n  mv       <src> <dest>    Move a source to a destination."
         "\n  outcp    <src> <dest>    Copy a source from the file system to a destination on the disk."
         "\n  pwd                      Print full path to CWD from root."
         "\n  rm       <file>          Remove file from the file system."
         "\n  rmdir    <dir>           Remove an empty directory from the file system."
         "\n"
         "\n  defrag   <file>          Defragment data placement of a file."
         "\n"
         "\n  clear                    Clear the screen and move cursor to the home position."
         "\n  exit                     Exit the process with exit code 0."
         "\n  help                     Print this message."
         "\n"
         "\n"
    );
    return SUCCESS;
}
