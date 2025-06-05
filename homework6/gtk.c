// Include gtk
#include <gtk/gtk.h>

void listdir(char *path, GtkStringList *list)
{
  GDir *dir;
  GError *error = NULL;
  const gchar* name;

  dir = g_dir_open(path, 0, &error); // Открываем каталог, флаг 0 означает обычный режим

  if (dir == NULL) {
    fprintf(stderr, "Ошибка при открытии каталога: %s\n", error->message);
    g_error_free(error); // Освобождаем память, выделенную для сообщения об ошибке
    return ;
  }

  while ((name = g_dir_read_name(dir)) != NULL)
  {
   // printf("%s\n", name);
    if (g_file_test(name,G_FILE_TEST_IS_DIR))
    {
      char path[1024];
      snprintf(path, sizeof(path), "%s/", name);
      gtk_string_list_append (list, (gchar*)path);
      listdir(path, list);
    } else
    {
      sprintf(path, " - %s", name);
      gtk_string_list_append (list, (gchar*)path);
    }

  }
  g_dir_close(dir);
}


static void setup_listitem (GtkListItemFactory *factory, GtkListItem *listitem)
{
  GtkWidget *label = gtk_label_new(NULL);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_list_item_set_child (listitem, label);
}

static void bind_listitem (GtkListItemFactory *factory, GtkListItem *listitem)
{
  GtkWidget* label = gtk_list_item_get_child(listitem);
  // получаем данные из модели
  GtkStringObject* strobj = gtk_list_item_get_item (listitem);
  // устанавливаем по этим данным текст метки
  gtk_label_set_text (GTK_LABEL (label), gtk_string_object_get_string(strobj));
}

static void app_activate (GApplication *app, gpointer *user_data) {

  GtkWidget *window = gtk_application_window_new (GTK_APPLICATION (app));
  gtk_window_set_title (GTK_WINDOW (window), "Files_View");
  gtk_window_set_default_size (GTK_WINDOW (window), 300, 250);

  GtkStringList* list = gtk_string_list_new(NULL);
  listdir(g_get_current_dir(),list);


    // создаем GtkSingleSelection
    GtkSingleSelection* model = gtk_single_selection_new(G_LIST_MODEL(list));

    // Создание фабрики для элементов списка
    GtkListItemFactory* factory = gtk_signal_list_item_factory_new();
    g_signal_connect (factory, "setup", G_CALLBACK (setup_listitem), NULL);
    g_signal_connect (factory, "bind", G_CALLBACK (bind_listitem), NULL);


    // Создание GtkListView
    GtkWidget* list_view = gtk_list_view_new(GTK_SELECTION_MODEL(model), factory);

    GtkWidget* scrollView = gtk_scrolled_window_new();

    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrollView), list_view);

    gtk_window_set_child (GTK_WINDOW (window), scrollView);

    gtk_window_present (GTK_WINDOW (window));
}


int main (int argc, char **argv)
{
    GtkApplication *app = gtk_application_new ("com.metanit", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
    int status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref(app);

    return status;
}
