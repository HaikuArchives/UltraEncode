/* id3 Renamer
 * file.h - Header for file i/o functions
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

#ifndef __FILE_H__
#define __FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

int add_to_log (char *);
int id3_read_file (char *, unsigned long, FILE *, char *);
int id3_write_file (char *, unsigned long, FILE *, char *);
int id3_open_file (FILE **, char *, char *);
int id3_seek_header (FILE *, char *);
int strip_tag (long, char *);

#ifdef __cplusplus
}
#endif

#endif /* __FILE_H__ */
