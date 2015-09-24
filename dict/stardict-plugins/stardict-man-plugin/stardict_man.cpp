/*
 * Copyright 2011 kubtek <kubtek@mail.com>
 *
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

#include "stardict_man.h"
#include <glib/gi18n.h>
#include <cstring>
#include <string>

static const StarDictPluginSystemInfo *plugin_info = NULL;
static bool need_prefix;
static IAppDirs* gpAppDirs = NULL;

/* concatenate path1 and path2 inserting a path separator in between if needed. */
static std::string build_path(const std::string& path1, const std::string& path2)
{
	std::string res;
	res.reserve(path1.length() + 1 + path2.length());
	res = path1;
	if(!res.empty() && res[res.length()-1] != G_DIR_SEPARATOR)
		res += G_DIR_SEPARATOR_S;
	if(!path2.empty() && path2[0] == G_DIR_SEPARATOR)
		res.append(path2, 1, std::string::npos);
	else
		res.append(path2);
	return res;
}

static std::string get_cfg_filename()
{
	return build_path(gpAppDirs->get_user_config_dir(), "man.cfg");
}

static char *build_dictdata(char type, const char *definition)
{
	size_t len = strlen(definition);
	guint32 size;
	size = sizeof(char) + len + 1;
	char *data = (char *)g_malloc(sizeof(guint32) + size);
	char *p = data;
	*((guint32 *)p)= size;
	p += sizeof(guint32);
	*p = type;
	p++;
	memcpy(p, definition, len+1);
	return data;
}

static void terminal2pango(const char *t, std::string &pango)
{
	pango.clear();
	std::string prev_str;
	std::string prev_char;
	const char *p1;
	while (*t) {
		if (*t == 8) {
			if (g_str_has_prefix(t+1, prev_char.c_str())) {
				t++;
				pango += "<b>";
				pango += prev_str;
				pango += "</b>";
			} else if (prev_char == "_") {
				t++;
				p1 = g_utf8_next_char(t);
				char *mark = g_markup_escape_text(t, p1 -t);
				pango += "<u>";
				pango += mark;
				pango += "</u>";
				g_free(mark);
			}
			prev_str.clear();
		} else {
			pango += prev_str;
			switch (*t) {
				case '&':
					prev_str = "&amp;";
					break;
				case '<':
					prev_str = "&lt;";
					break;
				case '>':
					prev_str = "&gt;";
					break;
				case '\'':
					prev_str = "&apos;";
					break;
				case '"':
					prev_str = "&quot;";
					break;
				default:
					p1 = g_utf8_next_char(t);
					prev_str.assign(t, p1-t);
					break;
			}
		}
		p1 = g_utf8_next_char(t);
		prev_char.assign(t, p1-t);
		t = p1;
	}
	pango += prev_str;
}

static void lookup(const char *text, char ***pppWord, char ****ppppWordData)
{
	std::string command;
	if (need_prefix || g_str_has_prefix(text, "man ")) {
		if (!g_str_has_prefix(text, "man ") || text[4] == '\0' || (g_ascii_isdigit(text[4]) && (text[5] == '\0' || (text[5] == ' ' && text[6] == '\0')))) {
			*pppWord = NULL;
			return;
		}
		if (g_ascii_isdigit(text[4]) && text[5] ==' ') {
			command.append(text, 6);
			text += 6;
		} else {
			command.append(text, 4);
			text += 4;
		}
	} else {
		if (g_ascii_isdigit(text[0]) && (text[1] == '\0' || (text[1] == ' ' && text[2] == '\0'))) {
			*pppWord = NULL;
			return;
		}
		command = "man ";
		if (g_ascii_isdigit(text[0]) && text[1] ==' ') {
			command.append(text, 2);
			text += 2;
		}
	}
	gchar *quote = g_shell_quote(text); // Must quote, or a security hole!
	command += quote;
	g_free(quote);
	command += " 2>/dev/null";
	FILE *pf = popen(command.c_str(), "r");
	if (!pf) {
		*pppWord = NULL;
		return;
	}
	std::string definition;
	char buffer[2048];
	size_t len;
	while (true) {
		len = fread(buffer, 1, sizeof(buffer), pf);
		if (len <= 0)
			break;
		definition.append(buffer, len);
	}
	pclose(pf);
	if (definition.empty()) {
		*pppWord = NULL;
		return;
	}
	std::string pango;
	terminal2pango(definition.c_str(), pango);
	*pppWord = (gchar **)g_malloc(sizeof(gchar *)*2);
	(*pppWord)[0] = g_strdup(text);
	(*pppWord)[1] = NULL;
	*ppppWordData = (gchar ***)g_malloc(sizeof(gchar **)*(1));
	(*ppppWordData)[0] = (gchar **)g_malloc(sizeof(gchar *)*2);
	(*ppppWordData)[0][0] =  build_dictdata('g', pango.c_str());
	(*ppppWordData)[0][1] = NULL;
}

static void configure()
{
	GtkWidget *window = gtk_dialog_new_with_buttons(_("Man configuration"), GTK_WINDOW(plugin_info->pluginwin), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
#if GTK_MAJOR_VERSION >= 3
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
#else
	GtkWidget *vbox = gtk_vbox_new(false, 5);
#endif
	GtkWidget *check_button = gtk_check_button_new_with_mnemonic(_("_Input string requires the \"man \" prefix. For example: \"man printf\"."));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), need_prefix);
	gtk_box_pack_start(GTK_BOX(vbox), check_button, false, false, 0);
	gtk_widget_show_all(vbox);
	gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(window))), vbox);
	gtk_dialog_run(GTK_DIALOG(window));
	gboolean new_need_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button));
	if (new_need_prefix != need_prefix) {
		need_prefix = new_need_prefix;
		const char *tmp;
		if (need_prefix)
			tmp = "true";
		else
			tmp = "false";
		gchar *data = g_strdup_printf("[man]\nneed_prefix=%s\n", tmp);
		std::string res = get_cfg_filename();
		g_file_set_contents(res.c_str(), data, -1, NULL);
		g_free(data);
	}
	gtk_widget_destroy (window);
}

bool stardict_plugin_init(StarDictPlugInObject *obj, IAppDirs* appDirs)
{
	g_debug(_("Loading Man plug-in..."));
	if (strcmp(obj->version_str, PLUGIN_SYSTEM_VERSION)!=0) {
		g_print("Error: Man plugin version doesn't match!\n");
		return true;
	}
	obj->type = StarDictPlugInType_VIRTUALDICT;
	obj->info_xml = g_strdup_printf("<plugin_info><name>%s</name><version>1.0</version><short_desc>%s</short_desc><long_desc>%s</long_desc><author>Hu Zheng &lt;huzheng001@gmail.com&gt;</author><website>http://www.stardict.org</website></plugin_info>", _("Man"), _("Man virtual dictionary."), _("Show the man pages."));
	obj->configure_func = configure;
	plugin_info = obj->plugin_info;
	gpAppDirs = appDirs;

	return false;
}

void stardict_plugin_exit(void)
{
	gpAppDirs = NULL;
}

bool stardict_virtualdict_plugin_init(StarDictVirtualDictPlugInObject *obj)
{
	obj->lookup_func = lookup;
	obj->dict_name = _("Man");
	std::string res = get_cfg_filename();
	if (!g_file_test(res.c_str(), G_FILE_TEST_EXISTS)) {
		g_file_set_contents(res.c_str(), "[man]\nneed_prefix=true\n", -1, NULL);
	}
	GKeyFile *keyfile = g_key_file_new();
	g_key_file_load_from_file(keyfile, res.c_str(), G_KEY_FILE_NONE, NULL);
	GError *err = NULL;
	need_prefix = g_key_file_get_boolean(keyfile, "man", "need_prefix", &err);
	if (err) {
		g_error_free (err);
		need_prefix = true;
	}
	g_key_file_free(keyfile);
	g_unsetenv("MANPAGER");
	g_print(_("Man plug-in loaded.\n"));
	return false;
}
