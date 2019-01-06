
#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "vec_math.h"
#include "json.h"

extern int json_value_get_int(const struct json_value_s* value, int* dest);
extern int json_value_get_float(const struct json_value_s* value, float* dest);
extern int json_value_get_vec3(const struct json_value_s* value, Vec3* dest);
extern int json_value_get_quat(const struct json_value_s* value, Quat* dest);
extern int json_value_get_string(const struct json_value_s* value, char* buffer, size_t maxLength);

#endif
