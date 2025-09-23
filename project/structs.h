
enum eDay
{
    TODAY = 0,
    TOMORROW,
    DAYAFTERTOMORROW,
};

typedef struct day
{
    char date[20];
    char temperature[20];
    char temperature_min[20];
    char temperature_max[20];
    char fuct[20];
    char wind[20];
    char precipitation[20];
}day_t;

typedef struct data
{
    day_t current;
    day_t days_weather[3];
}data_t;

struct memory {
    char *response;
    size_t size;
};
