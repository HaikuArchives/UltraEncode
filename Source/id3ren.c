/* id3 Renamer
 * id3ren.c - Main functions
 * Copyright (C) 1998  Robert Alto (badcrc@tscnet.com)
 * BeOS port by Shayne White (shayne@curvedspace.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "misc.h"
#include "id3tag.h"
#include "id3ren.h"

#define EXIT_PAUSE FALSE


extern ID3_tag *ptrtag;
extern const int genre_count;
extern char *genre_table[];
extern const char logfile[];

char *def_artist = NULL;
char *def_song = NULL;
char *def_album = NULL;
char *def_year = NULL;
char *def_comment = NULL;
int  def_genre = -1;


FLAGS_struct flags = { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
                       FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, 0 };


/*
 * Default template
 * %a - artist
 * %c - comment
 * %g - genre
 * %s - song name
 * %t - album title
 * %y - year
 *
 */

char filename_template[256] = "[%a]-[%s].mp3";
char *program_name = NULL;
char *program_path = NULL;

/* character to replace spaces with */
char replace_spacechar[32] = "_";

char *replace_char = "";
char *remove_char = "";

/* template-applied filename */
char applied_filename[512];


void exit_function (void);
void apply_template (char *);
void show_usage (char *);
void check_num_args (int, int);
int read_config (char *, char *);
void toggle_flag (short *);
void check_arg (int *, int, char *, char *);
int check_args (int, char *[]);
int main (int, char *[]);


void
exit_function (void)
{
  if (EXIT_PAUSE)
    while(getc(stdin)!='\n') ;
}


void
apply_template (char *origfile)
{
  char *ptrNewfile;
  char tmpfile[512];
  int i, tcount = 0;

  applied_filename[0] = '\0';
  ptrNewfile = applied_filename;

  while (filename_template[tcount] != '\0')
  {
    if (filename_template[tcount] == IDENT_CHAR)
    {
      tcount++;

      switch (filename_template[tcount])
      {
        case 'a': strcat(applied_filename, ptrtag->artist); break;
        case 'c': strcat(applied_filename, ptrtag->comment); break;
        case 'g':
          if (ptrtag->genre >= genre_count || ptrtag->genre < 0)
            user_message(FALSE, "Unknown genre in %s: %d\n", origfile, ptrtag->genre);
          else
            strcat(applied_filename, genre_table[ptrtag->genre]);
          break;
        case 's': strcat(applied_filename, ptrtag->songname); break;
        case 't': strcat(applied_filename, ptrtag->album); break;
        case 'y': strcat(applied_filename, ptrtag->year); break;
        default:
          user_message(FALSE, "Unknown identifier in template: %c%c\n",
            IDENT_CHAR, filename_template[tcount]);
          break;
      }

      tcount++;
    }
    else
    {
      ptrNewfile = applied_filename;
      ptrNewfile += strlen(applied_filename);
      *ptrNewfile = filename_template[tcount];
      ptrNewfile++;
      *ptrNewfile = '\0';
      tcount++;
    }
  }

  strcpy(tmpfile, applied_filename);
  ptrNewfile = applied_filename;
  tcount = 0;

  while (tmpfile[tcount] != '\0')
  {
    for (i = 0; i < strlen(replace_char); i += 2)
    {
      if ( replace_char[i] == tmpfile[tcount] &&
           ((i+1) < strlen(replace_char)) )
      {
        *ptrNewfile = replace_char[i+1];
        ptrNewfile++;
        i = 31335;
      }
    }

    if (i != 31337 && strchr(remove_char, tmpfile[tcount]) == NULL)
    {
      if (isalpha(tmpfile[tcount]) && flags.ulcase == 1)
      {
        *ptrNewfile = toupper(tmpfile[tcount]);
        ptrNewfile++;
      }
      else if (isalpha(tmpfile[tcount]) && flags.ulcase == 2)
      {
        *ptrNewfile = tolower(tmpfile[tcount]);
        ptrNewfile++;
      }
      else if (tmpfile[tcount] == ' ')
      {
        *ptrNewfile = '\0';
        strcat(applied_filename, replace_spacechar);
        ptrNewfile = applied_filename;
        ptrNewfile += strlen(applied_filename);
      }
      else
      {
        *ptrNewfile = tmpfile[tcount];
        ptrNewfile++;
      }
    }

    tcount++;
  }

  *ptrNewfile = '\0';

  /* Convert illegal or bad filename characters to good characters */
  for (i=0; i < strlen(applied_filename); i++)
  {
    switch (applied_filename[i])
    {
      case '<': applied_filename[i] = '['; break;
      case '>': applied_filename[i] = ']'; break;
      case '|': applied_filename[i] = '_'; break;
      case '/': applied_filename[i] = '-'; break;
      case '\\': applied_filename[i]= '-'; break;
      case '*': applied_filename[i] = '_'; break;
      case '?': applied_filename[i] = '_'; break;
      case ':': applied_filename[i] = ';'; break;
      case '"': applied_filename[i] = '-'; break;
      default: break;
    }
  }

}


void
show_usage (char *myname)
{
  user_message(TRUE, "id3 Renamer version %s\n", APP_VERSION);
  user_message(TRUE, "(C) Copyright 1998 by Robert Alto (badcrc@tscnet.com)\n");
  user_message(TRUE, "BeOS port by Shayne White (shayne@curvedspace.org)\n");
  user_message(TRUE, "Usage: %s [-help]\n",myname);
  user_message(TRUE, "       [-song \"SONG NAME\"] [-artist \"ARTIST NAME\"] [-album \"ALBUM NAME\"]\n");
  user_message(TRUE, "       [-year ####] [-genre {# | \"GENRE\"}] [-comment \"COMMENT\"]\n");
  user_message(TRUE, "       [-showgen] [-searchgen {# | \"GENRE\"}]\n");
  user_message(TRUE, "       [-quick] [-noalbum] [-nocomment] [-noyear] [-nogenre]\n");
  user_message(TRUE, "       [-tag] [-edit] [-notagprompt | -showtag | -striptag | -tagonly]\n");
  user_message(TRUE, "       [-nocfg] [-log] [-quiet] [-verbose] [-defcase | -lower | -upper]\n");
  user_message(TRUE, "       [-remchar CHARS] [-repchar CHARS] [-space=STRING]\n");
  user_message(TRUE, "       [-template=TEMPLATE] [FILE1 FILE2.. | WILDCARDS]\n\n");
  user_message(TRUE, "When logging is enabled, most normal output is also sent to %s.\n", logfile);
  user_message(TRUE, "To disable all output except for the usage screen on errors, use -quiet.\n\n");
  user_message(TRUE, "The template can contain the following identifiers from the id3 tag:\n");
  user_message(TRUE, " %ca - Artist       %cc - Comment  %cg - Genre  %cs - Song Name\n",
                      IDENT_CHAR, IDENT_CHAR, IDENT_CHAR, IDENT_CHAR);
  user_message(TRUE, " %ct - Album title  %cy - Year\n",
                      IDENT_CHAR, IDENT_CHAR);
}


void
check_num_args (int current, int total)
{
  if (current >= total)
  {
    user_message(TRUE, "%s: Not enough arguments\n", program_name);
    exit(ERRLEV_ARGS);
  }
}


int
read_config (char *path, char *filename)
{
  FILE *fp;
  char buffer[1024];
  char *cfile;
  char *p1, *p2;
  int i;

  if (path == NULL || strlen(path) < 1)
    return FALSE;

  if (filename == NULL || strlen(filename) < 1)
  {
    alloc_string(&cfile, (strlen(path) + 1));
    strcpy(cfile, path);
  }
  else
  {
    alloc_string(&cfile, (strlen(path) + strlen(filename) + 2));
    sprintf(cfile, "%s%c%s", path, PATH_CHAR, filename);
  }

  if (flags.verbose)
    user_message(FALSE, "%s: Checking for config file %s...", program_name, cfile);

  if (access(cfile, F_OK) != 0)
  {
    if (flags.verbose)
      user_message(FALSE, "not found\n");

    free(cfile);
    return FALSE;
  } else if (flags.verbose)
    user_message(FALSE, "found\n");

  fp = fopen(cfile, "rt");

  if (fp == NULL)
  {
    print_error("Couldn't open config file %s", cfile);
    free(cfile);
    return FALSE;
  }

  if (flags.verbose)
    user_message(FALSE, "%s: Reading config file %s\n", program_name, cfile);

  fgets(buffer, sizeof(buffer), fp);

  while (!feof(fp))
  {
    if (buffer[strlen(buffer)-1] == '\n')
      buffer[strlen(buffer)-1] = '\0';

    p1 = buffer;
    while (*p1 == ' ') p1++;

    if (*p1 != '\0' && *p1 != '#')
    {
      p2 = p1;

      while (*p2 != ' ' && *p2 != '\0')
        p2++;

      if (*p2 == ' ')
      {
        *p2 = '\0';
        p2++;
      }

      i = 0;
      check_arg (&i, ((*p2 == '\0') ? 1 : 2), p1, ((*p2 == '\0') ? "" : p2));
    }

    fgets(buffer, sizeof(buffer), fp);
  }

  fclose(fp);
  free(cfile);
  return TRUE;
}

void
toggle_flag (short *flag)
{
  if (*flag == TRUE)
  {
    *flag = FALSE;
  }
  else
  {
    *flag = TRUE;
  }
}

void
check_arg (int *count, int argc, char *arg1, char *arg2)
{
  char arg[256];
  char *p;

  strncpy(arg, arg1, (sizeof(arg)-1));
  p = arg; p++;

  if (strcmp(p, "help") == 0)
  {
    show_usage(program_name);
    exit(ERRLEV_SUCCESS);
  }
  else if (strcmp(p, "song") == 0)
  {
    (*count)++;
    check_num_args(*count, argc);
    alloc_string(&def_song, (strlen(arg2)+1));
    strcpy(def_song, arg2);
  }
  else if (strcmp(p, "artist") == 0)
  {
    (*count)++;
    check_num_args(*count, argc);
    alloc_string(&def_artist, (strlen(arg2)+1));
    strcpy(def_artist, arg2);
  }
  else if (strcmp(p, "album") == 0)
  {
    (*count)++;
    check_num_args(*count, argc);
    alloc_string(&def_album, (strlen(arg2)+1));
    strcpy(def_album, arg2);
  }
  else if (strcmp(p, "year") == 0)
  {
    (*count)++;
    check_num_args(*count, argc);
    alloc_string(&def_year, (strlen(arg2)+1));
    strcpy(def_year, arg2);
  }
  else if (strcmp(p, "comment") == 0)
  {
    (*count)++;
    check_num_args(*count, argc);
    alloc_string(&def_comment, (strlen(arg2)+1));
    strcpy(def_comment, arg2);
  }
  else if (strcmp(p, "genre") == 0)
  {
    (*count)++;
    check_num_args(*count, argc);

    if (search_genre(1, &def_genre, arg2) == FALSE)
    {
      user_message(TRUE, "%s: No genre selected\n", program_name);
      exit(ERRLEV_ARGS);
    }
  }
  else if (strcmp(p, "log") == 0)
    toggle_flag((short*)&flags.logging);
  else if (strcmp(p, "notagprompt") == 0)
    toggle_flag((short*)&flags.no_tag_prompt);
  else if (strcmp(p, "noalbum") == 0)
    toggle_flag((short*)&flags.no_album);
  else if (strcmp(p, "nocomment") == 0)
    toggle_flag((short*)&flags.no_comment);
  else if (strcmp(p, "noyear") == 0)
    toggle_flag((short*)&flags.no_year);
  else if (strcmp(p, "nogenre") == 0)
    toggle_flag((short*)&flags.no_genre);
  else if (strcmp(p, "nocfg") == 0)
    toggle_flag((short*)&flags.no_config);
  else if (strcmp(p, "quick") == 0)
  {
    flags.no_album = TRUE;
    flags.no_comment = TRUE;
    flags.no_year = TRUE;
  }
  else if (strcmp(p, "quiet") == 0)
    toggle_flag((short*)&flags.quiet);
  else if (strcmp(p, "verbose") == 0)
    toggle_flag((short*)&flags.verbose);
  else if (strcmp(p, "searchgen") == 0)
  {
    (*count)++;
    check_num_args(*count, argc);
    search_genre(2, &def_genre, arg2);
    exit(ERRLEV_SUCCESS);
  }
  else if (strcmp(p, "showgen") == 0)
  {
    show_genres(FALSE);
    exit(ERRLEV_SUCCESS);
  }
  else if (strcmp(p, "showtag") == 0)
  {
    flags.show_tag = TRUE;
    flags.no_tag_prompt = TRUE;
  }
  else if (strcmp(p, "striptag") == 0)
    toggle_flag((short*)&flags.strip_tag);
  else if (strcmp(p, "tag") == 0)
    toggle_flag((short*)&flags.force_tag);
  else if (strcmp(p, "tagonly") == 0)
    toggle_flag((short*)&flags.tag_only);
  else if (strcmp(p, "edit") == 0)
    toggle_flag((short*)&flags.edit_tag);
  else if (strcmp(p, "defcase") == 0)
    flags.ulcase = 0;
  else if (strcmp(p, "upper") == 0)
    flags.ulcase = 1;
  else if (strcmp(p, "lower") == 0)
    flags.ulcase = 2;
  else if (strncmp(p, "space", 5) == 0)
  {
    p = strstr(p, "="); if (p != NULL) p++;
    if (p == NULL)
      strcpy(replace_spacechar, "");
    else
      strncpy(replace_spacechar, p, (sizeof(replace_spacechar)-1));
  }
  else if (strncmp(p, "remchar", 7) == 0)
  {
    (*count)++;
    check_num_args(*count, argc);
    alloc_string(&remove_char, (strlen(arg2)+1));
    strcpy(remove_char, arg2);
  }
  else if (strncmp(p, "repchar", 7) == 0)
  {
    (*count)++;
    check_num_args(*count, argc);

    if ( (strlen(arg2) % 2) != 0 )
    {
      user_message(TRUE, "%s: Replace characters must be in pairs\n", program_name);
      exit(ERRLEV_ARGS);
    }

    alloc_string(&replace_char, (strlen(arg2)+1));
    strcpy(replace_char, arg2);
  }
  else if (strncmp(p, "template", 8) == 0)
  {
    p = strstr(p, "=");

    if (p != NULL) p++;

    if (p == NULL)
    {
      user_message(TRUE, "%s: Empty template specified (%s)\n", program_name, arg);
      exit(ERRLEV_ARGS);
    }
    else
      strncpy(filename_template, p, (sizeof(filename_template)-1));
  }
  else
  {
    user_message(TRUE, "%s: Unknown option: %s\n", program_name, arg);
    exit(ERRLEV_ARGS);
  }
}


int
check_args (int argc, char *argv[])
{
  int i;

  for (i = 1; i < argc && argv[i][0] == '-'; i++)
  {
    check_arg (&i, argc, argv[i], (((i+1) < argc) ? argv[i+1] : ""));
  }

  if (flags.no_config == FALSE)
  {
    if (!read_config(getenv("ID3REN"), ""))
      if (!read_config(getenv("HOME"), CONFIG_HOME))
  #ifdef __WIN32__
        read_config(program_path, CONFIG_GLOBAL);
  #else
        read_config("/etc", CONFIG_GLOBAL);
  #endif
  }

  return i;
}


int
main (int argc, char *argv[])
{
  char arg[256];
  char *p;
  int i;
  int count = 0;
  int errcount = 0;
#ifdef WIN32_FIND
  WIN32_FIND_DATA *lpwfd;
  HANDLE hSearch;
#endif

  atexit(exit_function);
  p = strrchr(argv[0], PATH_CHAR);

  if (p == NULL)
  {
    p = argv[0];
    alloc_string(&program_path, 2);
    strcpy(program_path, ".");
  }
  else
  {
    p++;
    alloc_string(&program_path, (strlen(argv[0]) - strlen(p)));
    strncpy(program_path, argv[0], ((strlen(argv[0]) - strlen(p)) - 1));
  }

  alloc_string(&program_name, (strlen(p)+1));
  strcpy(program_name, p);

  if (argc <= 1)
  {
    show_usage(argv[0]);
    return ERRLEV_ARGS;
  }

  if ( (ptrtag = (ID3_tag*)malloc(sizeof(*ptrtag)) ) == NULL)
  {
    print_error("Out of memory for malloc");
    exit(ERRLEV_MALLOC);
  }

  i = check_args (argc, argv);

  if (i >= argc)
  {
    user_message(TRUE, "%s: Not enough arguments specified\n", program_name);
    exit(ERRLEV_ARGS);
  }


  for (; i < argc; i++)
  {

        strncpy(arg, argv[i], (sizeof(arg)-1));

        if (access(arg, F_OK) != 0)
        {
          user_message(TRUE, "%s: File not found: %s\n",
            program_name, arg);
        }
        else if (tag_file(arg))
        {

          count++;

          if (flags.show_tag)
          {
            show_tag(ptrtag, arg);
          }
          else if (!flags.tag_only && !flags.strip_tag)
          {
            apply_template(arg);

            if (access(applied_filename, F_OK) == 0)
            {
              user_message(TRUE, "%s: File already exists: %s\n",
                program_name, applied_filename);
            }
            else if (rename(arg, applied_filename) == 0)
              user_message(FALSE, "%-38s => %-37s\n", arg, applied_filename);
            else
              print_error("Rename failed on %s", arg);
          }
        }
        else
        {
          errcount++;
        }
    {
      user_message(TRUE, "%s: File not found: %s\n", program_name, argv[i]);
    }
  } /* end for loop */


  user_message(FALSE, "Processed: %d  Failed: %d  Total: %d\n", count, errcount, count+errcount);
  return ERRLEV_SUCCESS;
} /* end main */

