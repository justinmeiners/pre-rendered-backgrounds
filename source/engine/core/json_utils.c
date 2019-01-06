
#include "json_utils.h"
#include <stdlib.h>

int json_value_get_int(const struct json_value_s* value, int* dest)
{
    if (value->type != json_type_number)
        return 0;
    
    struct json_number_s* number = value->payload;
    *dest = atoi(number->number);
    return 1;
}

int json_value_get_float(const struct json_value_s* value, float* dest)
{
    if (value->type != json_type_number)
        return 0;
    
    struct json_number_s* number = value->payload;
    *dest = atof(number->number);
    return 1;
}

int json_value_get_vec3(const struct json_value_s* value, Vec3* dest)
{
    if (value->type != json_type_array)
        return 0;
    
    struct json_array_s* array = value->payload;
    struct json_array_element_s* comp = array->start;
    
    if (!json_value_get_float(comp->value, &dest->x))
        return 0;
    comp = comp->next;
    
    if (!json_value_get_float(comp->value, &dest->y))
        return 0;
    comp = comp->next;
    
    if (!json_value_get_float(comp->value, &dest->z))
        return 0;
    
    return 1; 
}

int json_value_get_quat(const struct json_value_s* value, Quat* dest)
{
    if (value->type != json_type_array)
        return 0;
    
    struct json_array_s* array = value->payload;
    struct json_array_element_s* comp = array->start;
    
    if (!json_value_get_float(comp->value, &dest->w))
        return 0;
    comp = comp->next;
    
    if (!json_value_get_float(comp->value, &dest->x))
        return 0;
    comp = comp->next;
    
    if (!json_value_get_float(comp->value, &dest->y))
        return 0;
    comp = comp->next;
    
    if (!json_value_get_float(comp->value, &dest->z))
        return 0;
    
    return 1;
}

int json_value_get_string(const struct json_value_s* value, char* buffer, size_t maxLength)
{
    if (value->type != json_type_string)
        return 0;

    struct json_string_s* string = value->payload;
    
    if (string->string_size > maxLength)
        return 0;
    
    memcpy(buffer, string->string, string->string_size + 1);

    return 1;
}
