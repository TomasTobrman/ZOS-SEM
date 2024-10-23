#pragma once

/* ---- Standard ---- */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* ---- General ---- */
typedef enum {
    SUCCESS         = 0,

    ERR_ARGC        = 1 <<  0,
    ERR_ARGV        = 1 <<  1,

    ERR_CREATE      = 1 <<  2,
    ERR_OPEN        = 1 <<  3,
    ERR_READ        = 1 <<  4,
    ERR_WRITE       = 1 <<  5,
    ERR_EXISTS      = 1 <<  6,

    ERR_FULL        = 1 <<  7,
    ERR_MEMORY      = 1 <<  8,

    ERR_FILE        = 1 << 12,
    ERR_FAT         = 1 << 13,
    ERR_INTERN      = 1 << 14,

    CMD_EXIT        = 1 << 15
} rc;

/* ---- SIZE ---- */
#define KILOBYTE      1024
#define MEGABYTE      1048576
#define GIGABYTE      1073741824

#define ARGS_SIZE     4
#define BUFFER_SIZE   64

/* ---- FAT ---- */
#define FAT_DIRECTORY      0xFFFFFFFF
#define FAT_UNUSED         0xFFFFFFFE
#define FAT_FILE_END       0xFFFFFFFD
#define FAT_BAD_CLUSTER    0xFFFFFFFC

#define FAT_SIGNATURE_SIZE 9
#define FAT_ITEM_NAME_SIZE 13

typedef struct {
    char     signature[FAT_SIGNATURE_SIZE];
    uint32_t disk_size;
    uint32_t cluster_size;
    uint32_t cluster_count;
    uint32_t fat_start_address;
    uint32_t data_start_address;
} header;

typedef struct {
    char          name[FAT_ITEM_NAME_SIZE];
    uint32_t      size; // size == FAT_DIRECTORY ? directory : file
    uint32_t      start_cluster;
} file_item;

typedef struct {
    FILE       *file;
    const char *filepath;
    header      header;
} file_system;

/* ---- COMMANDS ---- */
typedef rc(cmd_func)(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_cat     (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_cd      (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_cp      (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_format  (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_incp    (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_info    (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_load    (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_ls      (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_mkdir   (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_mv      (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_outcp   (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_pwd     (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_rm      (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_rmdir   (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_defrag  (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_clear   (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_exit    (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_help    (file_system *fs, uint32_t *cluster, uint8_t argc, char **argv);

typedef struct {
    const char *cmd;
    cmd_func *func;
} command;

