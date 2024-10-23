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

    ERR_FULL        = 1 <<  6,
    ERR_MEMORY      = 1 <<  7,

    ERR_FAT         = 1 << 12,
    ERR_INTERN      = 1 << 13,

    CMD_EXIT        = 1 << 15
} rc;

/* ---- SIZE ---- */
#define KILOBYTE      1024
#define MEGABYTE      1048576
#define GIGABYTE      1073741824

#define ARGS_SIZE     4
#define BUFFER_SIZE   64

/* ---- FAT ---- */
#define FAT_UNUSED       0xFFFFFFFE
#define FAT_FILE_END     0xFFFFFFFD
#define FAT_BAD_CLUSTER  0xFFFFFFFC

#define FAT_SIGNATURE_SIZE 9
#define FAT_ITEM_NAME_SIZE 13

typedef struct {
    char     signature[FAT_SIGNATURE_SIZE];
    uint32_t disk_size;
    uint32_t cluster_size;
    uint32_t cluster_count;
    uint32_t fat_start_address;
    uint32_t data_start_address;
} fat_desc;

typedef struct {
    char          item_name[FAT_ITEM_NAME_SIZE];
    uint8_t       isFile;
    uint32_t      size;
    uint32_t      start_cluster;
} fat_item;

typedef struct {
    FILE       *file;
    const char *filepath;
    uint32_t    header_size;
    uint32_t    table_size;
    uint32_t    data_size;
    fat_desc    desc;
} fat;

/* ---- COMMANDS ---- */
typedef rc(cmd_func)(fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_cat     (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_cd      (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_cp      (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_format  (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_incp    (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_info    (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_load    (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_ls      (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_mkdir   (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_mv      (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_outcp   (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_pwd     (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_rm      (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_rmdir   (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_defrag  (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_clear   (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_exit    (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);
rc cmd_func_help    (fat *fs, uint32_t *cluster, uint8_t argc, char **argv);

typedef struct {
    const char *cmd;
    cmd_func *func;
} command;

