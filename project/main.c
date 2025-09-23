#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include "cjson/cJSON.h"
#include <gtk/gtk.h>
#include "structs.h"
#include <glib.h>

data_t weather_data;

GtkWidget *window;
GtkWidget *button_close, *button_ok;
GtkWidget *label_current, *label_today, *label_tomorrow, *label_dayaftertomorrow;
GtkWidget *text_city;
GtkWidget *overlay;

static size_t cb(void *data, size_t size, size_t nmemb, void *clientp) {
  size_t realsize = size * nmemb;
  struct memory *mem = (struct memory *)clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if(ptr == NULL)
    return 0;  /* out of memory! */

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}

void abort_on_error_check(CURLcode code, void(*callback)(void *), void *opaque) {
  if (CURLE_OK != code) {
    fprintf(stderr, "CURL Error: %s\n", curl_easy_strerror(code));
    callback(opaque);
    exit(code);
  }
}

#define abort_on_error(code) \
abort_on_error_check(code, curl_easy_cleanup, p_curl)

unsigned long __stack_chk_guard;
void __stack_chk_guard_setup(void)
{
     __stack_chk_guard = 0xBAAAAAAD;//provide some magic numbers
}

void __stack_chk_fail(void)
{
 /* Error message */
}// will be called when guard variable is corrupted

void list_children(cJSON* json)
{
    while (json != 0) {
        printf("%d %s\n", json->type, json->string);
        json = json->next;
    }
}

static void on_close_button_clicked(GtkButton *button, gpointer user_data)
{
  GtkWindow *window = GTK_WINDOW(user_data);
  gtk_window_close(window);
}

static void show_weather()
{
  char s[150];
  char mode[50];

  GError *error = NULL;
  GdkPixbuf *new_pixbuf;
  sprintf(mode,"%s", weather_data.current.fuct);
  if (strcmp(weather_data.current.fuct, "Overcast") == 0)
  {
    new_pixbuf = gdk_pixbuf_new_from_file("./pictures/overcast.jpg", &error);
    sprintf(mode,"Пасмурно");
  }
  else
    if ((strcmp(weather_data.current.fuct, "Partly cloudy") == 0)||(strcmp(weather_data.current.fuct, "Cloudy") == 0))
    {
      new_pixbuf = gdk_pixbuf_new_from_file("./pictures/partly_cloudy.jpg", &error);
      sprintf(mode,"Облачно");
    }
    else
      if ((strcmp(weather_data.current.fuct, "Patchy rain nearby") == 0)||(strcmp(weather_data.current.fuct, "Light rain") == 0)||(strcmp(weather_data.current.fuct, "Shower in vicinity")==0)
          ||(strcmp(weather_data.current.fuct, "Light rain shower")==0)||(strcmp(weather_data.current.fuct, "Rain, light drizzle WNW")==0))
      {
        new_pixbuf = gdk_pixbuf_new_from_file("./pictures/rain.jpg", &error);//patchy_rain_nearby.jpg", &error);
        sprintf(mode,"Местами дождь");
      }
      else
        if (strcmp(weather_data.current.fuct, "Sunny") == 0)
        {
          new_pixbuf = gdk_pixbuf_new_from_file("./pictures/sun.jpg", &error);
          sprintf(mode,"Солнечно");
        }
        else
          if (strcmp(weather_data.current.fuct, "Clear") == 0)
          {
            new_pixbuf = gdk_pixbuf_new_from_file("./pictures/clear.jpg", &error);
            sprintf(mode,"Ясно");
          }
          else
          if (strcmp(weather_data.current.fuct, "Rain with thunderstoENE") == 0)
          {
            new_pixbuf = gdk_pixbuf_new_from_file("./pictures/rain_with_thandersto.jpg", &error);
            sprintf(mode,"Дождь");
          }
          else new_pixbuf = gdk_pixbuf_new_from_file("./pictures/city.jpg", &error);
  if (!new_pixbuf) {
    g_printerr("Ошибка загрузки изображения: %s\n", error->message);
    g_error_free(error);
    return;
  }

  GtkWidget *new_picture = gtk_picture_new_for_pixbuf(new_pixbuf);
  gtk_picture_set_content_fit(GTK_PICTURE(new_picture), GTK_CONTENT_FIT_COVER);
  gtk_overlay_set_child(GTK_OVERLAY(overlay), new_picture);

  sprintf(s, "T: %s\n%s\nВетер: %s\nОсадки: %s mm", weather_data.current.temperature, mode, weather_data.current.wind, weather_data.current.precipitation);

  gtk_label_set_text(GTK_LABEL(label_current), s);


  sprintf(s, "%s\nT: %s / %s", weather_data.days_weather[TODAY].date, weather_data.days_weather[TODAY].temperature_max, weather_data.days_weather[TODAY].temperature_min);
  gtk_label_set_text(GTK_LABEL(label_today), s);

  sprintf(s, "%s\nT: %s / %s", weather_data.days_weather[TOMORROW].date, weather_data.days_weather[TOMORROW].temperature_max, weather_data.days_weather[TOMORROW].temperature_min);
  gtk_label_set_text(GTK_LABEL(label_tomorrow), s);

  sprintf(s, "%s\nT: %s / %s", weather_data.days_weather[DAYAFTERTOMORROW].date, weather_data.days_weather[DAYAFTERTOMORROW].temperature_max, weather_data.days_weather[DAYAFTERTOMORROW].temperature_min);
  gtk_label_set_text(GTK_LABEL(label_dayaftertomorrow), s);
}

static void get_weather(GtkWidget *widget, gpointer data)
{
  char url[100] = "https://wttr.in/";
  char city[15];
  strcpy(city,gtk_editable_get_text(GTK_EDITABLE(data)));
  strcat(url, city);
  strcat(url, "?format=j1");

  CURL *p_curl;

  struct memory mem = { 0 };

  p_curl = curl_easy_init();
  if (NULL == p_curl) {
    (void) fprintf(stderr, "CURL init failed\n");
  }

  abort_on_error(curl_easy_setopt(p_curl, CURLOPT_URL, url));
  abort_on_error(curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L));
  abort_on_error(curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &mem));
  abort_on_error(curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, cb));
  abort_on_error(curl_easy_perform(p_curl));

  int response_code;
  abort_on_error(curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &response_code));
  if (response_code != 200) {
    gtk_entry_set_placeholder_text(GTK_ENTRY(text_city), "No such city found!");
    return;
  }

  cJSON* root = cJSON_Parse(mem.response);

  cJSON* curr_cond_array = cJSON_GetObjectItem(root, "current_condition");
  cJSON* curr_cond_first = cJSON_GetArrayItem(curr_cond_array, 0);
  cJSON* wind_dir_obj = cJSON_GetObjectItem(curr_cond_first, "winddir16Point");
  memcpy(weather_data.current.wind, wind_dir_obj->valuestring, sizeof(weather_data.current.wind));

  cJSON* temp_dir_obj = cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "current_condition"), 0), "temp_C");
  memcpy(weather_data.current.temperature, temp_dir_obj->valuestring, sizeof(weather_data.current.temperature));

  cJSON* precip_dir_obj = cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(root, "current_condition"), 0), "precipMM");
  memcpy(weather_data.current.precipitation, precip_dir_obj->valuestring, sizeof(weather_data.current.precipitation));

  cJSON* weather_desc_obj = cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(curr_cond_first, "weatherDesc"), 0), "value");
  memcpy(weather_data.current.fuct, weather_desc_obj->valuestring, sizeof(weather_data.current.fuct));

//   cJSON* weather_desc_array = cJSON_GetObjectItem(curr_cond_first, "weatherDesc");
//   cJSON* weather_desc_first = cJSON_GetArrayItem(weather_desc_array, 0);
//   cJSON* weather_desc_obj = cJSON_GetObjectItem(weather_desc_first, "value");


  cJSON* weather_array = cJSON_GetObjectItem(root, "weather");
  cJSON* weather_first = cJSON_GetArrayItem(weather_array, 0);
  cJSON* date_obj = cJSON_GetObjectItem(weather_first, "date");
  cJSON* min_temp_c_obj = cJSON_GetObjectItem(weather_first, "mintempC");
  cJSON* max_temp_c_obj = cJSON_GetObjectItem(weather_first, "maxtempC");
  memcpy(weather_data.days_weather[TODAY].temperature_min, min_temp_c_obj->valuestring, sizeof(weather_data.days_weather[TODAY].temperature_min));
  memcpy(weather_data.days_weather[TODAY].temperature_max, max_temp_c_obj->valuestring, sizeof(weather_data.days_weather[TODAY].temperature_max));
  memcpy(weather_data.days_weather[TODAY].date, date_obj->valuestring, sizeof(weather_data.days_weather[TODAY].date));

  weather_first = cJSON_GetArrayItem(weather_array, 1);
  date_obj = cJSON_GetObjectItem(weather_first, "date");
  min_temp_c_obj = cJSON_GetObjectItem(weather_first, "mintempC");
  max_temp_c_obj = cJSON_GetObjectItem(weather_first, "maxtempC");
  memcpy(weather_data.days_weather[TOMORROW].temperature_min, min_temp_c_obj->valuestring, sizeof(weather_data.days_weather[TOMORROW].temperature_min));
  memcpy(weather_data.days_weather[TOMORROW].temperature_max, max_temp_c_obj->valuestring, sizeof(weather_data.days_weather[TOMORROW].temperature_max));
  memcpy(weather_data.days_weather[TOMORROW].date, date_obj->valuestring, sizeof(weather_data.days_weather[TOMORROW].date));

  weather_first = cJSON_GetArrayItem(weather_array, 2);
  date_obj = cJSON_GetObjectItem(weather_first, "date");
  min_temp_c_obj = cJSON_GetObjectItem(weather_first, "mintempC");
  max_temp_c_obj = cJSON_GetObjectItem(weather_first, "maxtempC");
  memcpy(weather_data.days_weather[DAYAFTERTOMORROW].temperature_min, min_temp_c_obj->valuestring, sizeof(weather_data.days_weather[DAYAFTERTOMORROW].temperature_min));
  memcpy(weather_data.days_weather[DAYAFTERTOMORROW].temperature_max, max_temp_c_obj->valuestring, sizeof(weather_data.days_weather[DAYAFTERTOMORROW].temperature_max));
  memcpy(weather_data.days_weather[DAYAFTERTOMORROW].date, date_obj->valuestring, sizeof(weather_data.days_weather[DAYAFTERTOMORROW].date));

  free(mem.response);
  curl_easy_cleanup(p_curl);

  show_weather();

}

static void activate(GtkApplication *app, gpointer user_data)
{


  // Создаем главное окно
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Прогноз погоды");
  gtk_window_fullscreen(GTK_WINDOW(window));
//  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

  // GtkWidget *picture = gtk_picture_new();
  // gtk_picture_set_filename(GTK_PICTURE(picture), "//home//liubov//otus//project//city.jpg");
  // gtk_picture_set_content_fit(GTK_PICTURE(picture),GTK_CONTENT_FIT_COVER);
  // gtk_window_set_child(GTK_WINDOW(window), picture);
  //



  // GtkCssProvider *provider = gtk_css_provider_new();
  // gtk_css_provider_load_from_data(provider, "window {background-image: url('/home/liubov/otus/project/city.jpg');background-size: cover;}", -1);
  // gtk_style_context_add_provider_for_display(gdk_display_get_default(),GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);


  // Создаём GtkOverlay
  overlay = gtk_overlay_new();
  gtk_window_set_child(GTK_WINDOW(window), overlay);

 //  Загружаем изображение фона
  GError *error = NULL;
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file("./pictures/city.jpg", &error);
  if (!pixbuf) {
    g_printerr("Ошибка загрузки изображения: %s\n", error->message);
    g_error_free(error);
    return;
  }

  // GError *error = NULL;
  // GFile *f = g_file_new_for_path("/home/liubov/otus/project/pictures/city.jpg");
  // GdkTexture *texture = gdk_texture_new_from_file(f, &error);
  // g_object_unref(f);
  // if (!texture) {
  //   g_printerr("Ошибка загрузки изображения: %s\n", error->message);
  //   g_clear_error(&error);
  //   return;
  // }

//  GdkTexture *texture = gdk_texture_new_for_pixbuf(pixbuf);

  // Создаём GtkPicture для отображения изображения
  GtkWidget *picture = gtk_picture_new_for_pixbuf(pixbuf);
  gtk_picture_set_content_fit(GTK_PICTURE(picture), GTK_CONTENT_FIT_COVER);
//  GtkWidget *picture = gtk_picture_new_for_paintable(GDK_PAINTABLE(texture));

  // Заставим картинку масштабироваться по размеру виджета
  // gtk_widget_set_hexpand(picture, TRUE);
  // gtk_widget_set_vexpand(picture, TRUE);
  // Устанавливаем фильтр масштабирования (опционально)
//  g_object_set(picture, "stretch", TRUE, NULL);

  gtk_overlay_set_child(GTK_OVERLAY(overlay), picture);

   // Создаём кнопку "Закрыть"
  button_close = gtk_button_new_with_label("Закрыть");
  gtk_widget_set_valign(button_close, GTK_ALIGN_START);
  gtk_widget_set_halign(button_close, GTK_ALIGN_END);

  g_signal_connect(button_close, "clicked", G_CALLBACK(on_close_button_clicked), window);

  // Добавляем кнопку как оверлей
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), button_close);

  // Создаем текстовый виджет (GtkEntry)
  text_city = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(text_city), "Введите город");
  gtk_widget_set_size_request(text_city, 320, 40);
  gtk_widget_set_valign(text_city, GTK_ALIGN_START);
  gtk_widget_set_halign(text_city, GTK_ALIGN_START);
  gtk_widget_set_margin_start(text_city, 600);
  gtk_widget_set_margin_top(text_city, 50);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), text_city);


  // Создаем кнопку
  button_ok = gtk_button_new_with_label("Оk");

  // Подключаем сигнал "clicked" кнопки к функции create_second_window
   g_signal_connect(button_ok, "clicked", G_CALLBACK(get_weather), text_city);

  // Добавляем кнопку в контейнер
  gtk_widget_set_size_request(button_ok, 40, 40);
  gtk_widget_set_valign(button_ok, GTK_ALIGN_START);
  gtk_widget_set_halign(button_ok, GTK_ALIGN_START);
  gtk_widget_set_margin_start(button_ok, 920);
  gtk_widget_set_margin_top(button_ok, 50);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), button_ok);

  label_current = gtk_label_new("");
  gtk_widget_set_size_request(label_current, 300, 200);
  gtk_widget_set_valign(label_current, GTK_ALIGN_START);
  gtk_widget_set_halign(label_current, GTK_ALIGN_START);
  gtk_widget_set_margin_start(label_current, 600);
  gtk_widget_set_margin_top(label_current, 110);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), label_current);

  label_today = gtk_label_new("");
  gtk_widget_set_size_request(label_today, 100, 100);
  gtk_widget_set_valign(label_today, GTK_ALIGN_START);
  gtk_widget_set_halign(label_today, GTK_ALIGN_START);
  gtk_widget_set_margin_start(label_today, 500);
  gtk_widget_set_margin_top(label_today, 420);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), label_today);

  label_tomorrow = gtk_label_new("");
  gtk_widget_set_size_request(label_tomorrow, 100, 100);
  gtk_widget_set_valign(label_tomorrow, GTK_ALIGN_START);
  gtk_widget_set_halign(label_tomorrow, GTK_ALIGN_START);
  gtk_widget_set_margin_start(label_tomorrow, 700);
  gtk_widget_set_margin_top(label_tomorrow, 420);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), label_tomorrow);

  label_dayaftertomorrow = gtk_label_new("");
  gtk_widget_set_size_request(label_dayaftertomorrow, 100, 100);
  gtk_widget_set_valign(label_dayaftertomorrow, GTK_ALIGN_START);
  gtk_widget_set_halign(label_dayaftertomorrow, GTK_ALIGN_START);
  gtk_widget_set_margin_start(label_dayaftertomorrow, 900);
  gtk_widget_set_margin_top(label_dayaftertomorrow, 420);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), label_dayaftertomorrow);


//  Создаём CSS провайдер и загружаем стили
  GtkCssProvider *css_provider = gtk_css_provider_new();
  const gchar *css_data =
  " label {"
  "  background-color: transparent;"
  "  font-size: 24px;"
  "  font-weight: bold;"
  "}";
  gtk_css_provider_load_from_data(css_provider, css_data, -1);




  // Применяем CSS ко всему приложению
   GtkStyleContext *context = gtk_widget_get_style_context(label_current);
   gtk_style_context_add_provider(context,
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
   GtkStyleContext *context1 = gtk_widget_get_style_context(label_today);
   gtk_style_context_add_provider(context1,
                                  GTK_STYLE_PROVIDER(css_provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_USER);
   GtkStyleContext *context2 = gtk_widget_get_style_context(label_tomorrow);
   gtk_style_context_add_provider(context2,
                                  GTK_STYLE_PROVIDER(css_provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_USER);
   GtkStyleContext *context3 = gtk_widget_get_style_context(label_dayaftertomorrow);
   gtk_style_context_add_provider(context3,
                                  GTK_STYLE_PROVIDER(css_provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_USER);


  // Показываем окно
  gtk_widget_show(GTK_WIDGET(window));

//  g_object_unref(texture);


}

int main(int argc, char *argv[]) {

  GtkApplication *app;
  int status;

  // Создаем приложение с идентификатором
  app = gtk_application_new("org.example.gtk4app", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  // Запускаем приложение
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;


}


