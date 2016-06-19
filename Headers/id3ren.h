#ifndef __ID3REN_H__
#define __ID3REN_H__

typedef struct
{
	//	Log normal output to a file
	short logging;
	
	//Don't prompt for tags on files without tags 
	short no_tag_prompt;
	
	/* Don't read config files */
	short no_config;
	
	/* Run silently */
	short quiet;
	
	/* Show lots of stuff */
	short verbose;
	
	/* Only display tags */
	short show_tag;
	
	/* Only strip tags */
	short strip_tag;
	
	/* Always prompt for tag info, even when renaming files with tags */
	short force_tag;
	
	/* Don't do any renaming, only prompt for tags */
	short tag_only;
	
	/* If a tag field already has info, don't prompt for that */
	short edit_tag;
	
	/* Skip asking for these tag items? */
	short no_year;
	short no_album;
	short no_comment;
	short no_genre;
	
	/* 0-default 1-upper 2-lower */
	short ulcase;
} FLAGS_struct;


#endif /* __ID3REN_H__ */

