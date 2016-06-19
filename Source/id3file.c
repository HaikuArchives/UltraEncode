
#include <stdio.h>
/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "DebugBuild.h"
#include "id3file.h"
#include "id3ren.h"


/* todo: add option to change log filename */
const char logfile[] = "id3ren.log";


int add_to_log (char *data)
{
  FILE *fp;

  if (id3_open_file(&fp, (char*)logfile, "at") == FALSE)
    return FALSE;

  fprintf(fp, "%s", data);
  fclose(fp);

  return TRUE;
}


int id3_read_file (char *dest, unsigned long size, FILE *fp, char *fn)
{
  if (fread(dest, size, 1, fp) != 1)
  {
    ERROR("id3_read_file: Error reading %s\n", fn);
    fclose(fp);
    return FALSE;
  }

  if (ferror(fp) != 0)
  {
    ERROR("id3_read_file: Error reading %s\n", fn);
    clearerr(fp);
    fclose(fp);
    return FALSE;
  }

  if (feof(fp))
  {
    ERROR("id3_read_file: Premature end of file in %s\n", fn);
    fclose(fp);
    return FALSE;
  }

  return TRUE;
}


int
id3_write_file(char *src, unsigned long size, FILE *fp, char *fn)
{
  if (fwrite(src, size, 1, fp) != 1)
  {
    ERROR("id3_write_file: Error writing to %s\n", fn);
    fclose(fp);
    return FALSE;
  }

  if (ferror(fp) != 0)
  {
    ERROR("id3_write_file: Error writing to %s\n", fn);
    clearerr(fp);
    fclose(fp);
    return FALSE;
  }

  return TRUE;
}


int id3_open_file (FILE **fp, char *fn, char *mode)
{
  *fp = fopen(fn, mode);

  if (*fp == NULL)
  {
    ERROR("id3_open_file: Error opening file %s\n", fn);
    return FALSE;
  }

  return TRUE;
}


int id3_seek_header (FILE *fp, char *fn)
{

  if (fseek(fp, -128, SEEK_END) < 0)
  {
    fclose(fp);
    ERROR("id3_seek_header: Error reading file %s\n", fn);
    return FALSE;
  }

  return TRUE;
}


int strip_tag (long sizelesstag, char *fn)
{
  int fd;

  fd = open(fn, O_RDWR);

  if (fd == -1)
  {
    ERROR("strip_tag: Error opening %s\n", fn);
    return FALSE;
  }


  ftruncate(fd, sizelesstag);

  close(fd);
  return TRUE;
}

