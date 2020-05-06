

#define PARSE_PROPERTY_INT32   0
#define PARSE_PROPERTY_INT64   1
#define PARSE_PROPERTY_STR     2
#define PARSE_PROPERTY_DOUBLE  3

// This instructs the parser to fill in param struct
typedef struct {
  int offset;
  int8_t type;
  int8_t size;
  union {
    int64_t lower_int64;
    double lower_double;
  };
  union {
    int64_t upper_int64;
    double upper_double;
  };
} parse_property_t;

parse_property_t parse_properties[] = {
  {(int)offsetof(plot_param_t, width), PARSE_PROPERTY_DOUBLE, (int8_t)sizeof(double), 
    {0}, {.upper_double = PARSE_DOUBLE_MAX}},
  {(int)offsetof(plot_param_t, height), PARSE_PROPERTY_DOUBLE, (int8_t)sizeof(double), 
    {0}, {.upper_double = PARSE_DOUBLE_MAX}},
  // Legends
  {(int)offsetof(plot_param_t, legend_rows), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {1}, {.upper_int64 = PARSE_INT64_MAX}},
  {(int)offsetof(plot_param_t, legend_font_size), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {1}, {.upper_int64 = PARSE_INT64_MAX}},
  {(int)offsetof(plot_param_t, legend_pos), PARSE_PROPERTY_STR, PLOT_LEGEND_POS_MAX_SIZE, 
    {0}, {0}},
  // Ticks
  {(int)offsetof(plot_param_t, xtick_font_size), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {1}, {.upper_int64 = PARSE_INT64_MAX}},
  {(int)offsetof(plot_param_t, xtick_rotation), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {0}, {.upper_int64 = 359L}},
  {(int)offsetof(plot_param_t, ytick_font_size), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {1}, {.upper_int64 = PARSE_INT64_MAX}},
  {(int)offsetof(plot_param_t, ytick_rotation), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {0}, {.upper_int64 = 359L}},
  // Title font sizes
  {(int)offsetof(plot_param_t, xtitle_font_size), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {1}, {.upper_int64 = PARSE_INT64_MAX}},
  {(int)offsetof(plot_param_t, ytitle_font_size), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {1}, {.upper_int64 = PARSE_INT64_MAX}},
  // Bar text
  {(int)offsetof(plot_param_t, bar_text_font_size), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {1}, {.upper_int64 = PARSE_INT64_MAX}},
  {(int)offsetof(plot_param_t, bar_text_rotation), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {0}, {.upper_int64 = 359L}},
  {(int)offsetof(plot_param_t, bar_text_decimals), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {.lower_int64 = PARSE_INT64_MIN}, {.upper_int64 = PARSE_INT64_MAX}},
  {(int)offsetof(plot_param_t, bar_text_rtrim), PARSE_PROPERTY_INT32, (int8_t)sizeof(int32_t), 
    {0}, {.upper_int64 = 1L}},
};
