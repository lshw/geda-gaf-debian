/* gEDA - GPL Electronic Design Automation
 * gschem - gEDA Schematic Capture
 * Copyright (C) 1998-2010 Ales Hvezda
 * Copyright (C) 1998-2013 gEDA Contributors (see ChangeLog for details)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include "config.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif


#include "gschem.h"

#include <glib/gstdio.h>

struct PopupEntry {
  const gchar const *name, *action, *stock_id;
};

static struct PopupEntry popup_items[] = {
  { N_("Add Net"), "add-net", "insert-net" },
  { N_("Add Attribute"), "add-attribute", "insert-attribute" },
  { N_("Add Component"), "add-component", "insert-symbol" },
  { N_("Add Bus"), "add-bus", "insert-bus" },
  { N_("Add Text"), "add-text", "insert-text" },
  { "SEPARATOR", NULL, NULL },
  { N_("Zoom In"), "view-zoom-in", "gtk-zoom-in" },
  { N_("Zoom Out"), "view-zoom-out", "gtk-zoom-out" },
  { N_("Zoom Box"), "view-zoom-box", NULL },
  { N_("Zoom Extents"), "view-zoom-extents", "gtk-zoom-fit" },
  { "SEPARATOR", NULL, NULL },
  { N_("Select"), "edit-select", "select" },
  { N_("Edit..."), "edit-edit", NULL },
  { N_("Edit Pin Type..."), "edit-pin-type", NULL },
  { N_("Copy"), "edit-copy", "clone" },
  { N_("Move"), "edit-move", NULL },
  { N_("Delete"), "edit-delete", "gtk-delete" },
  { "SEPARATOR", NULL, NULL },
  { N_("Down Schematic"), "hierarchy-down-schematic", "gtk-go-down" },
  { N_("Down Symbol"), "hierarchy-down-symbol", "gtk-go-bottom" },
  { N_("Up"), "hierarchy-up", "gtk-go-up" },

  { NULL, NULL, NULL }, /* Guard */
};


/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
static void g_menu_execute(GtkAction *action, gpointer user_data)
{
  const gchar *action_name = gtk_action_get_name (action);
  GschemToplevel *w_current = (GschemToplevel *) user_data;
  g_action_eval_by_name (w_current, action_name);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
GtkWidget *
get_main_menu(GschemToplevel *w_current)
{
  char *buf;
  GschemAction *action;
  GtkWidget *menu_item;
  GtkWidget *root_menu;
  GtkWidget *menu_bar;
  GtkWidget *menu;
  int scm_items_len;
  SCM scm_items;
  SCM scm_item;
  SCM scm_item_name;
  SCM scm_item_func;
  SCM scm_item_stock;
  SCM scm_index;
  SCM scm_keys;
  char *menu_name;
  char *action_name;
  char **raw_menu_name = g_malloc (sizeof(char *));
  char *menu_item_name;
  char *raw_menu_item_name;
  char *menu_item_stock;
  char *menu_item_keys;
  int i, j;

  menu_bar = gtk_menu_bar_new ();

  scm_dynwind_begin (0);
  g_dynwind_window (w_current);
  /*! \bug This function may leak memory if there is a non-local exit
   * in Guile code. At some point, unwind handlers need to be added to
   * clean up heap-allocated strings. */

  for (i = 0 ; i < s_menu_return_num(); i++) {
    
    scm_items = s_menu_return_entry(i, raw_menu_name);   
    if (*raw_menu_name == NULL) {
      fprintf(stderr, "Oops.. got a NULL menu name in get_main_menu()\n");
      exit(-1);
    }

    menu = gtk_menu_new();

    menu_item = gtk_tearoff_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
    gtk_widget_show(menu_item);

    scm_items_len = (int) scm_ilength (scm_items);
    for (j = 0 ; j < scm_items_len; j++) {

      scm_index = scm_from_int (j);
      scm_item = scm_list_ref (scm_items, scm_index);
      scm_item_name = SCM_CAR (scm_item);
      scm_item_func = SCM_CADR (scm_item);
      scm_item_stock = scm_is_pair (SCM_CDDR (scm_item)) ?
                         SCM_CADDR (scm_item) : SCM_BOOL_F;
      SCM_ASSERT(scm_is_string(scm_item_name), scm_item_name, SCM_ARGn, "get_main_menu item_name");
      SCM_ASSERT(scm_is_symbol (scm_item_func) ||
                    scm_is_false (scm_item_func),
                 scm_item_func, SCM_ARGn, "get_main_menu item_func");
      SCM_ASSERT (scm_is_string (scm_item_stock) ||
                    scm_is_false (scm_item_stock),
                  scm_item_stock, SCM_ARGn, "get_main_menu stock");

      raw_menu_item_name = scm_to_utf8_string(scm_item_name);
      scm_dynwind_begin(0);
      scm_dynwind_free(raw_menu_item_name);

      menu_item_name = (char *) gettext(raw_menu_item_name);

      if (strcmp(menu_item_name, "SEPARATOR") == 0) {
        menu_item = gtk_menu_item_new();
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
      } else {

        if(scm_is_false (scm_item_func)) {
          menu_item = gtk_menu_item_new_with_mnemonic(menu_item_name);
        } else {

          GtkStockItem stock_info;

          /* Look up key binding in global keymap */
          SCM s_expr =
            scm_list_2 (scm_from_utf8_symbol ("find-key"),
                        scm_list_2 (scm_from_utf8_symbol ("quote"),
                                    scm_item_func));

          scm_keys = g_scm_eval_protected (s_expr, scm_interaction_environment ());

          if (scm_is_false (scm_keys)) {
            menu_item_keys = "";
          } else {
            menu_item_keys = scm_to_utf8_string (scm_keys);
            scm_dynwind_free(menu_item_keys);
          }

          if (scm_is_false (scm_item_stock))
            menu_item_stock = NULL;
          else
            menu_item_stock = scm_to_utf8_string (scm_item_stock);

          action_name = scm_to_utf8_string (scm_symbol_to_string (scm_item_func));
          action = g_object_new (GSCHEM_TYPE_ACTION,
                                 "name", action_name,
                                 "label", menu_item_name,
                                 "tooltip", menu_item_name,
                                 "multikey-accel", menu_item_keys,
                                 NULL);

          /* If stock name corresponds to a GTK stock item, then use
           * it.  Otherwise, look it up in the icon theme. */
          if (menu_item_stock != NULL &&
              gtk_stock_lookup (menu_item_stock, &stock_info)) {
            gtk_action_set_stock_id (GTK_ACTION(action), menu_item_stock);
          } else {
            gtk_action_set_icon_name (GTK_ACTION(action), menu_item_stock);
          }


          free(action_name);
          free(menu_item_stock);

          menu_item = gtk_action_create_menu_item (GTK_ACTION (action));
          g_signal_connect (G_OBJECT(action), "activate",
                            G_CALLBACK(g_menu_execute),
                            w_current);
        }

        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
      }

      gtk_widget_show (menu_item);

      /* add a handle to the menu_bar object to get access to widget objects */
      /* This string should NOT be internationalized */
      buf = g_strdup_printf("%s/%s", *raw_menu_name, raw_menu_item_name);
      g_object_set_data (G_OBJECT (menu_bar), buf, menu_item);
      g_free(buf);

      scm_dynwind_end();
    }
    
    menu_name = (char *) gettext(*raw_menu_name);
    root_menu = gtk_menu_item_new_with_mnemonic (menu_name);
    /* do not free *raw_menu_name */

    /* no longer right justify the help menu since that has gone out of style */

    gtk_widget_show (root_menu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (root_menu), menu);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), root_menu);
  }
  scm_dynwind_end ();

  g_free(raw_menu_name);
  return menu_bar;
}

GtkWidget *
get_main_popup (GschemToplevel *w_current)
{
  GschemAction *action;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GtkStockItem stock_info;
  int i;

  menu = gtk_menu_new ();

  for (i = 0; popup_items[i].name != NULL; i++) {
    struct PopupEntry e = popup_items[i];

    /* No action --> add a separator */
    if (e.action == NULL) {
      menu_item = gtk_menu_item_new();
      gtk_widget_show (menu_item);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
      continue;
    }

    /* Don't bother showing keybindings in the popup menu */
    action = g_object_new (GSCHEM_TYPE_ACTION,
                           "name", e.action,
                           "label", gettext (e.name),
                           "tooltip", gettext (e.name),
                           NULL);
    /* If there's a matching stock item, use it. Otherwise lookup the
       name in the icon theme. */
    if (e.stock_id != NULL && gtk_stock_lookup (e.stock_id, &stock_info)) {
      gtk_action_set_stock_id (GTK_ACTION (action), e.stock_id);
    } else {
      gtk_action_set_icon_name (GTK_ACTION (action), e.stock_id);
    }

    /* Connect things up so that the actions get run */
    g_signal_connect (G_OBJECT (action), "activate",
                      G_CALLBACK (g_menu_execute), w_current);

    menu_item = gtk_action_create_menu_item (GTK_ACTION (action));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Add a handle to the menu object to get access to widget
       objects. Horrible horrible hack, but it's the same approach as
       taken for the main menu bar. :-( */
    g_object_set_data (G_OBJECT (menu), e.name, menu_item);
  }

  return menu;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 *  \note
 *  need to look at this... here and the setup
 */
gint do_popup (GschemToplevel *w_current, GdkEventButton *event)
{
  GtkWidget *menu = (GtkWidget *) w_current->popup_menu;
  g_return_val_if_fail (menu != NULL, FALSE);

  gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
                  event->button, event->time);

  return FALSE;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_menus_sensitivity (GschemToplevel *w_current, const char *buf, int flag)
{
  GtkWidget* item=NULL;
  
  if (!buf) {
    return;
  }

  if (!w_current->menubar) {
    return;
  }
  
  item = (GtkWidget *) g_object_get_data (G_OBJECT (w_current->menubar), buf);

  if (item) {
    gtk_widget_set_sensitive(GTK_WIDGET(item), flag);
    /* free(item); */ /* Why doesn't this need to be freed?  */
  } else {
    s_log_message(_("Tried to set the sensitivity on non-existent menu item '%s'\n"), buf); 
  }
 
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *  This function sets the sensitivity of the items in the right button
 *  popup.
 */
void x_menus_popup_sensitivity (GschemToplevel *w_current, const char *buf, int flag)
{
  GtkWidget *item;

  g_assert (w_current);
  g_assert (buf);
  g_assert (w_current->popup_menu);

  item = GTK_WIDGET (g_object_get_data (G_OBJECT (w_current->popup_menu), buf));

  if (item) {
    gtk_widget_set_sensitive (item, flag);
  } else {
    g_critical (_("Tried to set the sensitivity on non-existent menu item '%s'\n"),
                buf);
  }
}

#define MAX_RECENT_FILES 10
/*! \brief Callback for recent-chooser.
 *
 * Will be called if element of recent-file-list is activated
 */
void
recent_chooser_item_activated (GtkRecentChooser *chooser, GschemToplevel *w_current)
{
  PAGE *page;
  gchar *uri;
  gchar *filename;

  uri = gtk_recent_chooser_get_current_uri (chooser);
  filename = g_filename_from_uri(uri, NULL, NULL);
  gtk_recent_manager_add_item(recent_manager, uri);
  page = x_window_open_page(w_current, (char *)filename);
  x_window_set_current_page(w_current, page);

  g_free(uri);
  g_free(filename);
}

/*! \brief Attach a submenu with filenames to the 'Open Recent'
 *         menu item.
 *
 *  Called from x_window_setup().
 */
void x_menu_attach_recent_files_submenu(GschemToplevel *w_current)
{
  GtkWidget* menuitem_to_append_to = NULL;
  GtkRecentFilter *recent_filter;
  GtkWidget *menuitem_file_recent_items;
  recent_manager = gtk_recent_manager_get_default();

  menuitem_file_recent_items = gtk_recent_chooser_menu_new_for_manager(recent_manager);

  /* Show only schematic- and symbol-files (*.sch and *.sym) in list */
  recent_filter = gtk_recent_filter_new();
  gtk_recent_filter_add_mime_type(recent_filter, "application/x-geda-schematic");
  gtk_recent_filter_add_mime_type(recent_filter, "application/x-geda-symbol");
  gtk_recent_filter_add_pattern(recent_filter, "*.sch");
  gtk_recent_filter_add_pattern(recent_filter, "*.sym");
  gtk_recent_chooser_add_filter(GTK_RECENT_CHOOSER(menuitem_file_recent_items), recent_filter);

  gtk_recent_chooser_set_show_tips(GTK_RECENT_CHOOSER(menuitem_file_recent_items), TRUE);
  gtk_recent_chooser_set_sort_type(GTK_RECENT_CHOOSER(menuitem_file_recent_items),
                                   GTK_RECENT_SORT_MRU);
  gtk_recent_chooser_set_limit(GTK_RECENT_CHOOSER(menuitem_file_recent_items), MAX_RECENT_FILES);
  gtk_recent_chooser_set_local_only(GTK_RECENT_CHOOSER(menuitem_file_recent_items), FALSE);
  gtk_recent_chooser_menu_set_show_numbers(GTK_RECENT_CHOOSER_MENU(menuitem_file_recent_items), TRUE);
  g_signal_connect(GTK_OBJECT(menuitem_file_recent_items), "item-activated",
                   G_CALLBACK(recent_chooser_item_activated), w_current);

  menuitem_to_append_to = (GtkWidget *) g_object_get_data (G_OBJECT (w_current->menubar),
                                                           "_File/Open Recen_t");
  if(menuitem_to_append_to == NULL)
    return;
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem_to_append_to), menuitem_file_recent_items);
}
