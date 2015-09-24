/*
 * This file is part of StarDict.
 *
 * StarDict is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StarDict is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with StarDict.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdio.h"
#include "stdlib.h"
#include <locale.h>
#include <string.h>
#include <sys/stat.h>

#include <string>

#include <gtk/gtk.h>
#include <glib.h>

struct _worditem
{
	gchar *word;
	gchar *definition;
	gboolean definition_need_free;
};

gint stardict_strcmp(const gchar *s1, const gchar *s2)
{
	gint a;
	a = g_ascii_strcasecmp(s1, s2);
	if (a == 0)
		return strcmp(s1, s2);
	else
		return a;
}

gint comparefunc(gconstpointer a,gconstpointer b)
{
	return stardict_strcmp(((struct _worditem *)a)->word,((struct _worditem *)b)->word);
}

void convert(char *filename)
{			
	struct stat stats;
	if (stat (filename, &stats) == -1)
	{
		printf("file not exist!\n");
		return;
	}
	gchar *basefilename = g_path_get_basename(filename);
	FILE *tabfile;
	tabfile = fopen(filename,"r");

	gchar *buffer = (gchar *)g_malloc (stats.st_size + 1);
	size_t fread_size;
	fread_size = fread (buffer, 1, stats.st_size, tabfile);
	if (fread_size != (size_t)stats.st_size) {
		g_print("fread error!\n");
	}
	fclose (tabfile);
	buffer[stats.st_size] = '\0';	
	
	GArray *array = g_array_sized_new(FALSE,FALSE, sizeof(struct _worditem),20000);
		
	gchar *p, *p1, *p2;
	p = buffer;
	struct _worditem worditem;
	std::string definition;
	glong linenum=1;
	while (1) {
		if (*p == '\0') {
			g_print("over\n");
			break;
		}
		p2 = strchr(p,'\n');
		if (p2) {
			*p2 = '\0';
		}
		else {
			g_print("error! not end up new line found %ld\n", linenum);
			return;
		}
		p1 = strstr(p, "  ");
		if (p1) {
			*p1 = '\0';
			p1+=2;
		} else {
			p1 = strstr(p, ": ");
			if (p1) {
				*p1 = '\0';
				p1+=2;
			} else {
				p1 = strstr(p, " = ");
				if (p1) {
					*p1 = '\0';
					p1+=3;
				} else {
					g_print("%ld: %s\n", linenum, p);
					p= p2+1;
					linenum++;
					continue;
				}
			}
		}
		worditem.word = p;
		worditem.definition = p1;
		g_strstrip(worditem.word);
		g_strstrip(worditem.definition);
		if (!worditem.word[0]) {
			g_print("%ld: %s, bad word! %s\n", linenum, basefilename, worditem.definition);
			p= p2+1;
			linenum++;
			continue;
		}
		if (!worditem.definition[0]) {
			g_print("%s, bad definition!!!\n", basefilename);
			return;
		}
		worditem.definition_need_free = false;
		g_array_append_val(array, worditem);			
		p= p2+1;				
		linenum++;
	}	
	g_array_sort(array,comparefunc);
		
	gchar idxfilename[256];
	gchar dicfilename[256];
	sprintf(idxfilename, "%s.idx", basefilename);
	sprintf(dicfilename, "%s.dict", basefilename);
	FILE *idxfile = fopen(idxfilename,"w");
	FILE *dicfile = fopen(dicfilename,"w");

	glong tmpglong = 0;
	
	glong wordcount = array->len;

	long offset_old;
	const gchar *previous_word = "";
	struct _worditem *pworditem;
	gulong i=0;
	glong thedatasize;
	const gchar *insert_word = "\n";
	gboolean flag;
	pworditem = &g_array_index(array, struct _worditem, i);
	gint definition_len;
	while (i<array->len)
	{
		thedatasize = 0; 
		offset_old = ftell(dicfile);
		flag = true;
		while (flag == true)
		{	
			definition_len = strlen(pworditem->definition);
			fwrite(pworditem->definition, 1 ,definition_len,dicfile);
			thedatasize += definition_len;
			previous_word = pworditem->word;
						
			i++;
			if (i<array->len)
			{
				pworditem = &g_array_index(array, struct _worditem, i);
				if (strcmp(previous_word,pworditem->word)==0)
				{
					//g_print("D! %s\n",previous_word);
					wordcount--;
					fwrite(insert_word,sizeof(gchar),strlen(insert_word),dicfile);
					thedatasize += strlen(insert_word);
				}
				else 
				{
					flag = false;
				}
			}
			else
				flag = false;
		}
		fwrite(previous_word,sizeof(gchar),strlen(previous_word)+1,idxfile);
		tmpglong = g_htonl(offset_old);
		fwrite(&(tmpglong),sizeof(glong),1,idxfile);
		tmpglong = g_htonl(thedatasize);
		fwrite(&(tmpglong),sizeof(glong),1,idxfile);	
	}	
	for (i= 0;i < array->len; i++) {
		pworditem = &g_array_index(array, struct _worditem, i);
		if (pworditem->definition_need_free)
			g_free(pworditem->definition);
	}	

	
	g_print("%s wordcount: %ld\n", basefilename, wordcount);

	g_free(buffer);
	g_array_free(array,TRUE);
	
	fclose(idxfile);
	fclose(dicfile);

	g_free(basefilename);
}

int
main(int argc,char * argv [])
{
	if (argc<2) {
		printf("please type this:\n./duden duden.utf\n");
		return FALSE;
	}

	setlocale(LC_ALL, "");
	for (int i=1; i< argc; i++)
		convert (argv[i]);
	return FALSE;
	
}
