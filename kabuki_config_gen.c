/*
 * kabuki_config_gen
 * Copyright (c) 2024 nosuke <sasugaanija@gmail.com>
 *
 * This software is released under the MIT License.
 * http://opensource.org/licenses/mit-license.php
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <ctype.h>

#include <unistd.h>
extern char *optarg;
extern int optind, opterr, optopt;
#include <getopt.h>


#ifdef __MINGW32__
#include <windows.h>
#endif

#define CODE_LEN 11

#define NAME "kabuki_config_gen"
#define VERSION "1.0"
#define CONFIG_FILE "config.txt"
#define CONFIG_FILE_ALL "config_all.txt" // for DEBUG
//#define DEBUG

// convert MAME style code to KABUKI config data
void
convert_code(uint8_t *dst, uint32_t key1, uint32_t key2, uint16_t addr, uint8_t xor)
{
    uint8_t *code = dst;
    uint8_t tmp[8];
    int shift[] = {6, 5, 4, 2, 1, 0};

    // clear first 8Bytes
    memset(code, 0, 8);

    code[0] = addr >> 8;
    code[1] = addr & 0xff;

    for (int i = 0; i < 4; i++) {
        tmp[i] = key2 >> ((3 - i) * 8);
        tmp[i + 4] = key1 >> ((3 - i) * 8);
    }

    for (int i = 0, j = 0, k = 2; i < 6 * 8; i++) {
        code[k] |= ((tmp[j] >> shift[i % 6]) & 1) << (7 - (i % 8));
        if (i % 6 == 5)
            j++;
        if (i % 8 == 7)
            k++;
    }
    
    code[8] = xor;
}

#define MASK_8BIT     0x7000
#define MASK_CPS1     0xFF00
#define MASK_PANG     0xF000

enum game_id {
      pang = 0,
      spang,
      spangj,
      sbbros,
      dokaben,
      dokaben2,
      cbasebal,
      cworld,
      hatena,
      qtono1,
      qsangoku,
      block,
      mgakuen2,
      pkladies,
      marukin,
      wof,
      dino,
      punisher,
      slammast,
      mbombrd,
      _base_id_num
};

typedef struct title_jpn {
    enum game_id id;
    unsigned char title[128];
} t_title_jpn;


t_title_jpn title_jpn[] = {
#ifdef USE_SJIS
# include "title_jpn_sjis.txt"
#else
# include "title_jpn_utf8.txt"
#endif
};


typedef struct kabuki_title_info {
    char long_name[64];
    char short_name[10];
    enum game_id id;
    enum game_id master;
    uint32_t swap1;
    uint32_t swap2;
    uint16_t addr;
    uint8_t xor;
    uint16_t mask;
    uint8_t code[CODE_LEN];
    int valid;
} t_kabuki_title_info;

t_kabuki_title_info kabuki_games[] = {
// long name,                                  short_name, id,       master,   swap, swap2, addr, xor, mask,      code, valid
  {"Pang / Buster Bros / Pomping World",       "pang",     pang,     pang,     0,    0,     0,    0,   MASK_PANG, {0},  0},
  {"Super Pang (World)",                       "spang",    spang,    spang,    0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Super Pang (Japan)",                       "spangj",   spangj,   spangj,   0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Super Buster Bros",                        "sbbros",   sbbros,   sbbros,   0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Dokaben",                                  "dokaben",  dokaben,  mgakuen2, 0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Dokaben 2",                                "dokaben2", dokaben2, mgakuen2, 0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Capcom Baseball",                          "cbasebal", cbasebal, pang,     0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Capcom World",                             "cworld",   cworld,   cworld,   0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Adventure Quiz 2 Hatena ? no Dai-Bouken",  "hatena",   hatena,   hatena,   0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Quiz Tonosama no Yabou",                   "qtono1",   qtono1,   qtono1,   0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Quiz Sangokushi",                          "qsangoku", qsangoku, qsangoku, 0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Block Block",                              "block",    block,    block,    0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Mahjong Gakuen 2 Gakuen-chou no Fukushuu", "mgakuen2", mgakuen2, mgakuen2, 0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Porker Ladies",                            "pkladies", pkladies, mgakuen2, 0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Super Marukin-Ban",                        "marukin",  marukin,  marukin,  0,    0,     0,    0,   MASK_8BIT, {0},  0},
  {"Warriors of Fate",                         "wof",      wof,      wof,      0,    0,     0,    0,   MASK_CPS1, {0},  0},
  {"Cadillacs and Dinosaurs",                  "dino",     dino,     dino,     0,    0,     0,    0,   MASK_CPS1, {0},  0},
  {"Punisher",                                 "punisher", punisher, punisher, 0,    0,     0,    0,   MASK_CPS1, {0},  0},
  {"Slam Masters",                             "slammast", slammast, slammast, 0,    0,     0,    0,   MASK_CPS1, {0},  0},
  {"Muscle Bomber Duo",                        "mbombrd",  mbombrd,  slammast, 0,    0,     0,    0,   MASK_CPS1, {0},  0}
};




int
search_title_and_get_value(char *line, t_kabuki_title_info *title_info)
{
    if (strstr(line, title_info->long_name) == line) {
        // if line begins from long_name then
        // find " +([0-7]{8}) +([0-7]{8}) +([0-9a-f]{4}) +([0-9a-f]{2}) *$"
        char *p1;

        p1 = line + strlen(title_info->long_name);

        for (int j = 0; strlen(p1 + j) > (1 + 8 + 1 + 8 + 1 + 4 + 1 + 2); j++) {
            char *n1, *n2, *n3, *n4;
            char m[16];
            int unmatch = 0;
            char *p2 = p1 + j;
            if (*p2 != ' ')
                continue;
            p2++;

            while (*p2 == ' ') {
                p2++;
            }

            n1 = p2;
            for (int i = 0; i < 8; i++) {
                if (*(p2 + i) < '0' || *(p2 + i) > '9') {
                    unmatch = 1;
                    break;
                }
            }
            if (unmatch)
                continue;
            p2 += 8;

            if (*p2 != ' ')
                continue;
            p2++;

            while (*p2 == ' ') {
                p2++;
            }

            n2 = p2;
            for (int i = 0; i < 8; i++) {
                if (*(p2 + i) < '0' || *(p2 + i) > '9') {
                    unmatch = 1;
                    break;
                }
            }
            if (unmatch)
                continue;
            p2 += 8;


            if (*p2 != ' ')
                continue;
            p2++;

            while (*p2 == ' ') {
                p2++;
            }

            n3 = p2;
            for (int i = 0; i < 4; i++) {
                if (!isxdigit(*(p2 + i))) {
                    unmatch = 1;
                    break;
                }
            }
            if (unmatch)
                continue;
            p2 += 4;

            if (*p2 != ' ')
                continue;
            p2++;

            while (*p2 == ' ') {
                p2++;
            }

            n4 = p2;
            for (int i = 0; i < 2; i++) {
                if (!isxdigit(*(p2 + i))) {
                    unmatch = 1;
                    break;
                }
            }
            if (unmatch)
                continue;
            p2 += 2 ;

            if (*p2 != '\0' && !isspace(*p2))
                continue;
            p2++;

            while (isspace(*p2)) {
                p2++;
            }


            if (*p2 != '\0')
                continue;


            //printf("%s\n", title_info->long_name);

            m[0] = '0';
            m[1] = 'x';
            memcpy(m + 2, n1, 8);
            m[10] = '\0';
            title_info->swap1 = strtol(m, NULL, 16);
            //printf("1st: %s\n", m);

            memcpy(m + 2, n2, 8);
            m[10] = '\0';
            title_info->swap2 = strtol(m, NULL, 16);
            //printf("2nd: %s\n", m);

            memcpy(m + 2, n3, 4);
            m[6] = '\0';
            title_info->addr = strtol(m, NULL, 16);
            //printf("3rd: %s\n", m);

            memcpy(m + 2, n4, 2);
            m[4] = '\0';
            title_info->xor = strtol(m, NULL, 16);
            //printf("4th: %s\n", m);

            title_info->valid = 1;

            return 0;
        }

    }

    return -1;
}

unsigned char *
get_jp_title(int id)
{
    int title_num = sizeof(title_jpn) / sizeof(title_jpn[0]);

    for (int i = 0; i < title_num; i++) {
        if (title_jpn[i].id == id) {
            return title_jpn[i].title;
        }
    }
    return NULL;
}


int
title_selection(int ncodes)
{
    int in;

    for (int i = 0; i < ncodes; i++) {
        unsigned char *jp;
        if (kabuki_games[i].valid == 1) {
            printf("[%2d]", i);
        } else {
            printf("[**]");
        }
        jp = get_jp_title(kabuki_games[i].id);
        printf(" %s (%s)\n", kabuki_games[i].long_name, 
               jp != NULL ? jp : (unsigned char *)"");
    }
    printf("--------------------------------------------------------------------------------\n");
    printf("Select game title: ");
    fflush(stdout);
    scanf("%d", &in);
    while (fgetc(stdin) != '\n')
        ;
    if (in >= 0 && in < _base_id_num && kabuki_games[in].valid == 1) {
        return in;
    }
    return -1;
}

void
usage(char **argv)
{
    fprintf(stderr, "Usage: %s [OPTION]\n", argv[0]);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -o FILE    change output file name from %s\n", CONFIG_FILE);
    fprintf(stderr, "  -v         show tool version\n");   
    fprintf(stderr, "  -h         show this message\n");   
}


#ifdef __MINGW32__
int cp_org_valid = 0;
UINT cp_org;

BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
{
    switch(dwCtrlType) {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        if (cp_org_valid) {
            SetConsoleOutputCP(cp_org);
        }
        return FALSE;
        break;
    default:
        break;
    }
    return TRUE;
}
#endif

int
main(int argc, char *argv[])
{
    FILE *fp;
    char *configfile = CONFIG_FILE;
    int ret = -1;
    char line[1024];
    int ncodes = sizeof(kabuki_games) / sizeof(kabuki_games[0]);

    int opt;

#ifdef __MINGW32__
    cp_org = GetConsoleOutputCP();
    cp_org_valid = 1;
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
    SetConsoleOutputCP(65001);
#endif


    while ((opt = getopt(argc, argv, "o:vh")) != -1) {
        switch (opt) {
        case 'o':
            configfile = optarg;
            break;
        case 'v':
            ret = 0;
            printf("%s %s\n", NAME, VERSION);
            goto FIN;
        case 'h':
            usage(argv);
            goto FIN;
        default:
            fprintf(stderr, "Invalid options\n");
            usage(argv);
            goto FIN;
        }
    }


    fp = fopen("kabuki.cpp", "rb");
    if (fp == NULL) {
        perror("fopen");
        fprintf(stderr, "kabuki.cpp open failed\n");
        goto FIN;
    }



    while (fgets(line, sizeof(line) - 1, fp) > 0) {
        for (int i = 0; i < ncodes; i++) {
            if (search_title_and_get_value(line, &kabuki_games[i]) == 0)
                break;
        }
    }

    fclose (fp);

    for (int i = 0; i < ncodes; i++) {
        if (kabuki_games[i].valid == 0) {
            for (int j = 0; j < ncodes; j++) {
                // find master title
                if (kabuki_games[j].id == kabuki_games[i].master && kabuki_games[j].valid) {
                    kabuki_games[i].swap1 = kabuki_games[j].swap1;
                    kabuki_games[i].swap2 = kabuki_games[j].swap2;
                    kabuki_games[i].addr = kabuki_games[j].addr;
                    kabuki_games[i].xor = kabuki_games[j].xor;
                    kabuki_games[i].valid = 1;
                    break;
                }
            }
        }
    }

#ifdef DEBUG
    fp = fopen(CONFIG_FILE_ALL, "wb");
    if (fp == NULL) {
        perror("fopen");
        fprintf(stderr, "Output file \"%s\" open failed\n", CONFIG_FILE_ALL);
        goto FIN;
    }

    for (int i = 0; i < ncodes; i++) {
        if (kabuki_games[i].valid) {
            convert_code(kabuki_games[i].code, kabuki_games[i].swap1,
                         kabuki_games[i].swap2,
                         kabuki_games[i].addr, kabuki_games[i].xor);
            kabuki_games[i].code[9] = kabuki_games[i].mask >> 8;
            kabuki_games[i].code[10] = kabuki_games[i].mask & 0xff;        

            fprintf(fp, "%-8s   ", kabuki_games[i].short_name);
            for (int j = 0; j < CODE_LEN; j++) {
                fprintf(fp, "0x%02x", kabuki_games[i].code[j]);
                if (j < CODE_LEN - 1)
                    fprintf(fp, ", ");
                else
                    fprintf(fp, "\n");
            }
        }
    }

    fclose(fp);
#endif    

    ret = title_selection(ncodes);

    if (ret >= 0) {
        unsigned char *title_jpn = get_jp_title(kabuki_games[ret].id);
        printf("Generated config \"%s\" file for \n"
               "\"%s\" (%s)\n", configfile,
               kabuki_games[ret].long_name, title_jpn);
        
        fp = fopen(configfile, "wb");
        if (fp == NULL) {
            perror("fopen");
            fprintf(stderr, "Config file \"%s\" open failed\n", configfile);
            ret = -1;
            goto FIN;
        }

        convert_code(kabuki_games[ret].code, kabuki_games[ret].swap1,
                     kabuki_games[ret].swap2,
                     kabuki_games[ret].addr, kabuki_games[ret].xor);
        kabuki_games[ret].code[9] = kabuki_games[ret].mask >> 8;
        kabuki_games[ret].code[10] = kabuki_games[ret].mask & 0xff;        
            
        for (int i = 0; i < CODE_LEN; i++) {
            fprintf(fp, "0x%02x", kabuki_games[ret].code[i]);
            if (i < CODE_LEN - 1)
                fprintf(fp, ", ");
            else {
                fprintf(fp, " # %s (%s)\n", kabuki_games[ret].long_name,
                        title_jpn);
                fprintf(fp, "\n");
            }
        }


        fclose(fp);


    } else {
        printf("Invalid number\n");
    }
        

 FIN:

    fflush(stderr);

    printf("Press enter to exit.");
    fflush(stdout);
    getchar();

#ifdef __MINGW32__
    SetConsoleOutputCP(cp_org);
#endif


    return ret;
}
