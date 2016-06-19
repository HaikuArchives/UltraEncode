
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "DebugBuild.h"

#include "genre.h"
#include "id3tag.h"
#include "id3file.h"


void resize_tag_field (char *field)
{
  int i;

  i = strlen(field);
  while (field[i-1] == ' ')
    i--;
  field[i] = '\0';

}



bool write_tag (bool append_flag, char *fn, ID3_tag *ptrtag)
{
  FILE *fp;

  if (append_flag == true)
  {
    if (id3_open_file(&fp, fn, "ab") == false)
      return false;
  }
  else
  {
    if (id3_open_file(&fp, fn, "r+b") == false)
      return false;

    if (id3_seek_header(fp, fn) == false)
      return false;
  }

  strcpy(ptrtag->tag, "TAG");
  if (!id3_write_file(ptrtag->tag, (sizeof(ptrtag->tag)-1), fp, fn))
    return false;
  if (!id3_write_file(ptrtag->songname, (sizeof(ptrtag->songname)-1), fp, fn))
    return false;
  if (!id3_write_file(ptrtag->artist, (sizeof(ptrtag->artist)-1), fp, fn))
    return false;
  if (!id3_write_file(ptrtag->album, (sizeof(ptrtag->album)-1), fp, fn))
    return false;
  if (!id3_write_file(ptrtag->year, (sizeof(ptrtag->year)-1), fp, fn))
    return false;
  if (!id3_write_file(ptrtag->comment, (sizeof(ptrtag->comment)-1), fp, fn))
    return false;
/*    fwrite(ptrtag->genre, 1, 1, fp);*/

  if (fputc(ptrtag->genre, fp) == EOF)
  {
    fclose(fp);
    ERROR("write_tag: Error writing to %s \n", fn);
    return false;
  }

  fclose(fp);
  return true;
}


bool read_tag (FILE *fp, char *fn, ID3_tag *ptrtag)
{
	if (!id3_read_file(ptrtag->songname, (sizeof(ptrtag->songname)-1), fp, fn))
		return false;
		
	if (!id3_read_file(ptrtag->artist, (sizeof(ptrtag->artist)-1), fp, fn))
		return false;
		
	if (!id3_read_file(ptrtag->album, (sizeof(ptrtag->album)-1), fp, fn))
		return false;
		
	if (!id3_read_file(ptrtag->year, (sizeof(ptrtag->year)-1), fp, fn))
		return false;
		
	if (!id3_read_file(ptrtag->comment, (sizeof(ptrtag->comment)-1), fp, fn))
		return false;
	
	//fread(&ptrtag->genre, 1, 1, fp);
	ptrtag->genre = fgetc(fp);
	
	if (ptrtag->genre == EOF)
	{
		fclose(fp);
		ERROR("tag_file: Error reading %s\n", fn);
		return false;
	}
	
	return true;
}

bool tag_file (char *fn, ID3_tag *ptrtag)
{
  FILE *fp;
  long sizelesstag;
  short found_tag;

  if (id3_open_file(&fp, fn, "rb") == false)
    return false;

  ptrtag->tag[0] 		= '\0';
  ptrtag->songname[0] 	= '\0';
  ptrtag->artist[0] 	= '\0';
  ptrtag->album[0] 		= '\0';
  ptrtag->year[0] 		= '\0';
  ptrtag->comment[0] 	= '\0';
  ptrtag->genre 		= -1;

  if (id3_seek_header(fp, fn) == false) 
  	return false;
  	
  sizelesstag = ftell(fp);

  if (!id3_read_file(ptrtag->tag, (sizeof(ptrtag->tag)-1), fp, fn))
    return false;

  if (strcmp(ptrtag->tag, "TAG") != 0)
    found_tag = false;
  else
    found_tag = true;
    
  if (found_tag == false)
  {
    fclose(fp);

    PROGRESS("No ID3 tag found in %s\n", fn);

   // if (flags.strip_tag == true)
     // return true;

    /* return false so no renaming is performed on files without a tag */
//    if (flags.no_tag_prompt == true)
  //    return false;

    //printf("\n===> Entering new tag info for %s:\n", fn);
   // if (ask_tag(fn) == false)
   //   return false;

    if(write_tag(true, fn, ptrtag) == false)
      return false;

//    if (flags.tag_only == true)
  //    return true;

    if (id3_open_file(&fp, fn, "rb") == false)
      return false;

    if (id3_seek_header(fp, fn) == false) return false;

    if (!id3_read_file(ptrtag->tag, (sizeof(ptrtag->tag)-1), fp, fn))
      return false;
  }  /*** Found a tag ****/
  //else if (flags.force_tag == true)    /* Always ask for a tag enabled? */
  else
  {
    if (read_tag(fp, fn, ptrtag) == false)
      return false;

    fclose(fp);
    //printf("\n===> Changing old tag info for %s:\n", fn);

    //if (ask_tag(fn) == false)
    //  return false;

    if (write_tag(false, fn, ptrtag) == false)
      return false;

    //if (flags.tag_only == true)
      //return true;

    if (id3_open_file(&fp, fn, "rb") == false)
      return false;

    if (id3_seek_header(fp, fn) == false) return false;

    if (!id3_read_file(ptrtag->tag, (sizeof(ptrtag->tag)-1), fp, fn))
      return false;
  }

  if (read_tag(fp, fn, ptrtag) == false)
    return false;

  resize_tag_field(ptrtag->songname);
  resize_tag_field(ptrtag->artist);
  resize_tag_field(ptrtag->album);
  resize_tag_field(ptrtag->year);
  resize_tag_field(ptrtag->comment);
  fclose(fp);
  return true;
}
