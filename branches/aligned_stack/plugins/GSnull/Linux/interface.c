/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_Config (void)
{
  GtkWidget *Config;
  GtkWidget *vbox1;
  GtkWidget *frame2;
  GtkWidget *hbox1;
  GtkWidget *label4;
  GtkWidget *GtkCombo_Eth;
  GList *GtkCombo_Eth_items = NULL;
  GtkWidget *combo_entry1;
  GtkWidget *label1;
  GtkWidget *frame3;
  GtkWidget *hbox2;
  GtkWidget *label5;
  GtkWidget *GtkCombo_Hdd;
  GList *GtkCombo_Hdd_items = NULL;
  GtkWidget *entry1;
  GtkWidget *label15;
  GtkWidget *hbuttonbox1;
  GtkWidget *button1;
  GtkWidget *button2;

  Config = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (Config, "Config");
  gtk_container_set_border_width (GTK_CONTAINER (Config), 5);
  gtk_window_set_title (GTK_WINDOW (Config), _("DEV9config"));

  vbox1 = gtk_vbox_new (FALSE, 5);
  gtk_widget_set_name (vbox1, "vbox1");
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (Config), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

  frame2 = gtk_frame_new (NULL);
  gtk_widget_set_name (frame2, "frame2");
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox1), frame2, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (TRUE, 5);
  gtk_widget_set_name (hbox1, "hbox1");
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (frame2), hbox1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox1), 5);

  label4 = gtk_label_new (_("Device:"));
  gtk_widget_set_name (label4, "label4");
  gtk_widget_show (label4);
  gtk_box_pack_start (GTK_BOX (hbox1), label4, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_CENTER);

  GtkCombo_Eth = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (GtkCombo_Eth)->popwin),
                     "GladeParentKey", GtkCombo_Eth);
  gtk_widget_set_name (GtkCombo_Eth, "GtkCombo_Eth");
  gtk_widget_show (GtkCombo_Eth);
  gtk_box_pack_start (GTK_BOX (hbox1), GtkCombo_Eth, FALSE, FALSE, 0);
  GtkCombo_Eth_items = g_list_append (GtkCombo_Eth_items, (gpointer) "");
  gtk_combo_set_popdown_strings (GTK_COMBO (GtkCombo_Eth), GtkCombo_Eth_items);
  g_list_free (GtkCombo_Eth_items);

  combo_entry1 = GTK_COMBO (GtkCombo_Eth)->entry;
  gtk_widget_set_name (combo_entry1, "combo_entry1");
  gtk_widget_show (combo_entry1);

  label1 = gtk_label_new (_("Ethernet"));
  gtk_widget_set_name (label1, "label1");
  gtk_widget_show (label1);
  gtk_frame_set_label_widget (GTK_FRAME (frame2), label1);

  frame3 = gtk_frame_new (NULL);
  gtk_widget_set_name (frame3, "frame3");
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (vbox1), frame3, TRUE, TRUE, 0);

  hbox2 = gtk_hbox_new (TRUE, 5);
  gtk_widget_set_name (hbox2, "hbox2");
  gtk_widget_show (hbox2);
  gtk_container_add (GTK_CONTAINER (frame3), hbox2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 5);

  label5 = gtk_label_new (_("Device:"));
  gtk_widget_set_name (label5, "label5");
  gtk_widget_show (label5);
  gtk_box_pack_start (GTK_BOX (hbox2), label5, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_CENTER);

  GtkCombo_Hdd = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (GtkCombo_Hdd)->popwin),
                     "GladeParentKey", GtkCombo_Hdd);
  gtk_widget_set_name (GtkCombo_Hdd, "GtkCombo_Hdd");
  gtk_widget_show (GtkCombo_Hdd);
  gtk_box_pack_start (GTK_BOX (hbox2), GtkCombo_Hdd, FALSE, FALSE, 0);
  GtkCombo_Hdd_items = g_list_append (GtkCombo_Hdd_items, (gpointer) "");
  gtk_combo_set_popdown_strings (GTK_COMBO (GtkCombo_Hdd), GtkCombo_Hdd_items);
  g_list_free (GtkCombo_Hdd_items);

  entry1 = GTK_COMBO (GtkCombo_Hdd)->entry;
  gtk_widget_set_name (entry1, "entry1");
  gtk_widget_show (entry1);

  label15 = gtk_label_new (_("Hdd"));
  gtk_widget_set_name (label15, "label15");
  gtk_widget_show (label15);
  gtk_frame_set_label_widget (GTK_FRAME (frame3), label15);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox1, "hbuttonbox1");
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, TRUE, TRUE, 0);
  gtk_box_set_spacing (GTK_BOX (hbuttonbox1), 30);

  button1 = gtk_button_new_with_mnemonic (_("Ok"));
  gtk_widget_set_name (button1, "button1");
  gtk_widget_show (button1);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button1);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  button2 = gtk_button_new_with_mnemonic (_("Cancel"));
  gtk_widget_set_name (button2, "button2");
  gtk_widget_show (button2);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button2);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) button1, "clicked",
                    G_CALLBACK (OnConf_Ok),
                    NULL);
  g_signal_connect ((gpointer) button2, "clicked",
                    G_CALLBACK (OnConf_Cancel),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (Config, Config, "Config");
  GLADE_HOOKUP_OBJECT (Config, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (Config, frame2, "frame2");
  GLADE_HOOKUP_OBJECT (Config, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (Config, label4, "label4");
  GLADE_HOOKUP_OBJECT (Config, GtkCombo_Eth, "GtkCombo_Eth");
  GLADE_HOOKUP_OBJECT (Config, combo_entry1, "combo_entry1");
  GLADE_HOOKUP_OBJECT (Config, label1, "label1");
  GLADE_HOOKUP_OBJECT (Config, frame3, "frame3");
  GLADE_HOOKUP_OBJECT (Config, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (Config, label5, "label5");
  GLADE_HOOKUP_OBJECT (Config, GtkCombo_Hdd, "GtkCombo_Hdd");
  GLADE_HOOKUP_OBJECT (Config, entry1, "entry1");
  GLADE_HOOKUP_OBJECT (Config, label15, "label15");
  GLADE_HOOKUP_OBJECT (Config, hbuttonbox1, "hbuttonbox1");
  GLADE_HOOKUP_OBJECT (Config, button1, "button1");
  GLADE_HOOKUP_OBJECT (Config, button2, "button2");

  return Config;
}

GtkWidget*
create_About (void)
{
  GtkWidget *About;
  GtkWidget *vbox2;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *hbuttonbox2;
  GtkWidget *button3;

  About = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (About, "About");
  gtk_container_set_border_width (GTK_CONTAINER (About), 5);
  gtk_window_set_title (GTK_WINDOW (About), _("DEV9about"));

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_widget_set_name (vbox2, "vbox2");
  gtk_widget_show (vbox2);
  gtk_container_add (GTK_CONTAINER (About), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);

  label2 = gtk_label_new (_("FireWire Driver"));
  gtk_widget_set_name (label2, "label2");
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (vbox2), label2, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_CENTER);

  label3 = gtk_label_new (_("Author: linuzappz <linuzappz@hotmail.com>"));
  gtk_widget_set_name (label3, "label3");
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (vbox2), label3, FALSE, FALSE, 0);

  hbuttonbox2 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox2, "hbuttonbox2");
  gtk_widget_show (hbuttonbox2);
  gtk_box_pack_start (GTK_BOX (vbox2), hbuttonbox2, TRUE, TRUE, 0);
  gtk_box_set_spacing (GTK_BOX (hbuttonbox2), 30);

  button3 = gtk_button_new_with_mnemonic (_("Ok"));
  gtk_widget_set_name (button3, "button3");
  gtk_widget_show (button3);
  gtk_container_add (GTK_CONTAINER (hbuttonbox2), button3);
  GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) button3, "clicked",
                    G_CALLBACK (OnAbout_Ok),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (About, About, "About");
  GLADE_HOOKUP_OBJECT (About, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (About, label2, "label2");
  GLADE_HOOKUP_OBJECT (About, label3, "label3");
  GLADE_HOOKUP_OBJECT (About, hbuttonbox2, "hbuttonbox2");
  GLADE_HOOKUP_OBJECT (About, button3, "button3");

  return About;
}

