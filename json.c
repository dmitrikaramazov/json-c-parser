#include "stdint.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

#define null NULL

typedef struct JsonValue JsonValue;

typedef enum {
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_NULL,
    JSON_ERROR,
    JSON_OBJECT,
    JSON_ARRAY
} JSON_type_t;

typedef enum {
    TOKEN_STRING,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_OBJECT_START,
    TOKEN_OBJECT_END,
    TOKEN_ARRAY_START,
    TOKEN_ARRAY_END,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_NUMBER,
    TOKEN_ERROR,
    TOKEN_EOF
} JsonTokenType;

typedef struct {
    JsonTokenType type;
    char* start;
    int32_t length;
    int line;
    int column;
} JsonToken;

typedef struct JsonString {
    char* data;
    int32_t length;
} JsonString;

typedef struct JsonArray {
    JsonValue** items;
    int32_t count;
} JsonArray;

typedef struct JsonObject {
    char** keys;
    JsonValue** values;
    int32_t count;
} JsonObject;

typedef struct JsonValue {
    JSON_type_t type;
    union {
        int boolean;
        double number;
        JsonString string; 
        JsonArray array;
        JsonObject object;
    } value;
} JsonValue;


JsonValue* parse_from_file(const char* path);
void print_json(JsonValue* value, int indent_level);

JsonValue* parseValue(const char** input);
JsonValue* parseObject(const char** input, JsonToken token);
JsonValue* parseArray(const char** input, JsonToken token);
JsonValue* parseString(const char** input, JsonToken token);
JsonValue* parseNumber(const char** input, JsonToken token);
JsonValue* parseLiteral(const char** input, JsonToken token);
JsonToken nextToken(const char** input);

JsonValue* create_object(void);
JsonValue* create_array(void);
JsonValue* create_string(const char* data, int32_t length);
JsonValue* create_number(double number);
JsonValue* create_boolean(int value);
JsonValue* create_null(void);
JsonValue* create_error(void);
char* copy_string(const char* data, int32_t length);
void add_to_object(JsonValue* object, char* key, JsonValue* value);
void add_to_array(JsonValue* array, JsonValue* value);
void free_json_value(JsonValue* value);
void free_json_string(JsonString* str);

void add_to_object(JsonValue* object, char* key, JsonValue* value) {
    object->value.object.keys = realloc(object->value.object.keys, sizeof(char*) * (object->value.object.count + 1));
    object->value.object.values = realloc(object->value.object.values, sizeof(JsonValue*) * (object->value.object.count + 1));
    object->value.object.keys[object->value.object.count] = key;
    object->value.object.values[object->value.object.count] = value;
    object->value.object.count++;
}

void add_to_array(JsonValue* array, JsonValue* value) {
    array->value.array.items = realloc(array->value.array.items, sizeof(JsonValue*) * (array->value.array.count + 1));
    array->value.array.items[array->value.array.count] = value;
    array->value.array.count++;
}



void free_json_value(JsonValue* value) {
    if(value->type == JSON_OBJECT) {
        for(int i = 0; i < value->value.object.count; i++) {
            free(value->value.object.keys[i]);
            free_json_value(value->value.object.values[i]);
        }
        free(value->value.object.keys);
        free(value->value.object.values);
    } else if(value->type == JSON_ARRAY) {
        for(int i = 0; i < value->value.array.count; i++) {
            free_json_value(value->value.array.items[i]);
        }
        free(value->value.array.items);
    } else if(value->type == JSON_STRING) {
        free(value->value.string.data);
    }
    free(value);
}

void free_json_string(JsonString* str) {
    if(!str) return;
    if(str->data) {
        free(str->data);
    }
    free(str);
}

char* copy_string(const char* data, int32_t length) {
    if(length <= 0 || data == null) return null;
    if(length > INT32_MAX - 1) {
        puts("String length exceeds maximum size");
        return null;
    }
    char* str = malloc(length + 1);
    if(str == null) return null;
    memcpy(str, data, length);
    str[length] = '\0';
    return str;
}


JsonValue* create_object(void) {
    JsonValue* object = malloc(sizeof(JsonValue));
    object->type = JSON_OBJECT;
    object->value.object.keys = NULL;
    object->value.object.values = NULL;
    object->value.object.count = 0;
    return object;
}

JsonValue* create_array(void) {
    JsonValue* array = malloc(sizeof(JsonValue));
    array->type = JSON_ARRAY;
    array->value.array.items = NULL;
    array->value.array.count = 0;
    return array;
}

JsonValue* create_string(const char* data, int32_t length) {
    JsonValue* string = malloc(sizeof(JsonValue));
    string->type = JSON_STRING;
    string->value.string.data = copy_string(data, length);
    if(string->value.string.data == null) {
        free(string);
        return null;
    }
    string->value.string.length = length;
    return string;
}

JsonValue* create_number(double num) {
    JsonValue* number = malloc(sizeof(JsonValue));
    number->type = JSON_NUMBER;
    number->value.number = num;
    return number;
}

JsonValue* create_boolean(int value) {
    JsonValue* boolean = malloc(sizeof(JsonValue));
    boolean->type = JSON_BOOLEAN;
    boolean->value.boolean = value;
    return boolean;
}

JsonValue* create_null(void) {
    JsonValue* value = malloc(sizeof(JsonValue));
    value->type = JSON_NULL;
    return value;
}


JsonValue* parseValue(const char** input) {
    puts("ParseValue");
    JsonToken token = nextToken(input);
    switch(token.type) {
        case TOKEN_STRING:
            return parseString(input, token);
        case TOKEN_NUMBER:
            return parseNumber(input, token);
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_NULL:
            return parseLiteral(input, token);
        case TOKEN_OBJECT_START:
            return parseObject(input, token);
        case TOKEN_ARRAY_START:
            return parseArray(input, token);
        default:
            printf("ParseValue::Invalid token type: %d\n", token.type);
            printf("Token start: %.*s\n", token.length, token.start);
            puts("Invalid token in parseValue");
            return null;
    }
}

JsonValue* parseObject(const char** input, JsonToken input_token) {
    /*
    JsonToken token = nextToken(input);
    if(token.type != TOKEN_OBJECT_START) {
        puts("Invalid token in parseObject. Should be start-token");
        return null;
    }
    */
    JsonValue* object = create_object();
    JsonToken token = nextToken(input);
    if(token.type == TOKEN_OBJECT_END) {
        return object;
    }
    while(1) {
        if(token.type != TOKEN_STRING) {
            free_json_value(object);
            return null;
        }
        char* key = copy_string(token.start+1,token.length-2);
        token = nextToken(input);
        if(token.type != TOKEN_COLON) {
            //free_json_string(key);
            free(key);
            free_json_value(object);
            return null;
        }
        JsonValue* value = parseValue(input);
        if(!value) {
            free(key);
            free_json_value(object);
            return null;
        }
        add_to_object(object,key,value);
        token = nextToken(input);
        if(token.type == TOKEN_OBJECT_END) {
            return object;
        }
        if(token.type != TOKEN_COMMA) {
            free_json_value(object);
            return null;
        }
        token = nextToken(input);
    }
}

JsonValue* parseArray(const char** input, JsonToken input_token){
    puts("parseArray::start");
    JsonValue* array = create_array();
    while(1) {
        JsonValue* value = parseValue(input);
        if(!value) {
            free_json_value(array);
            puts("parseArray::Incorrect value in parseArray");
            return null;
        }
        add_to_array(array,value);
        JsonToken token = nextToken(input);
        if(token.type == TOKEN_ARRAY_END) {
            return array;
        }
        if(token.type != TOKEN_COMMA) {
            printf("parseArray::Invalid token in parseArray. Expected comma or end of array\n");
            printf("parseArray::Token type: %d\n", token.type);
            free_json_value(array);
            return null;
        }
    }
}

JsonValue* parseString(const char** input, JsonToken token) {
    //JsonToken token = nextToken(input);
    if(token.type != TOKEN_STRING) {
        puts("Invalid token in parseString");
        printf("Token type: %d\n", token.type);
        printf("Token start: %.*s\n", token.length, token.start);
        return null;
    }
    JsonValue* value = create_string(token.start+1, token.length-2);
    return value;
}

JsonValue* parseNumber(const char** input, JsonToken token){
    //JsonToken token = nextToken(input);
    if(token.type != TOKEN_NUMBER) {
        puts("Invalid token in parseNumber");
        return null;
    }
    JsonValue* value = create_number(strtod(token.start, null));
    return value;
}

JsonValue* parseLiteral(const char** input, JsonToken token) {
    if(token.type == TOKEN_TRUE) {
        return create_boolean(1);
    } else if(token.type == TOKEN_FALSE) {
        return create_boolean(0);
    } else if(token.type == TOKEN_NULL) {
        return create_null();
    }
    puts("Invalid token in parseLiteral");
    return null;
}


bool isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

JsonToken nextToken(const char** input) {
    JsonToken token;
    token.start = null;
    token.length = 0;
    token.line = 1;
    token.column = 1;

    while(**input && isWhitespace(**input)) {
        if(**input == '\n') {
            token.line++;
            token.column = 1;
        } else {
            token.column++;
        }
        (*input)++;
    }
    if(!**input) {
        token.type = TOKEN_EOF;
        return token;
    }
    token.start = (char*)*input;
    char c = **input;
    switch(c) {
        case '{':
            token.type = TOKEN_OBJECT_START;
            (*input)++;
            token.length = 1;
            break;
        case '}':
            token.type = TOKEN_OBJECT_END;
            (*input)++;
            token.length = 1;
            break;
        case '[':
            token.type = TOKEN_ARRAY_START;
            (*input)++;
            token.length = 1;
            break;
        case ']':
            token.type = TOKEN_ARRAY_END;
            (*input)++;
            token.length = 1;
            break;
        case ':':
            token.type = TOKEN_COLON;
            (*input)++;
            token.length = 1;
            break;
        case ',':
            token.type = TOKEN_COMMA;
            (*input)++;
            token.length = 1;
            break;
        case '"':
        {
            token.type = TOKEN_STRING;
            (*input)++;
            token.length = 1;
            while(**input && **input != '"') {
                if(**input == '\\') {
                    (*input)++;
                    if(**input) {
                        token.length++;
                        (*input)++;
                    }
                } else {
                    token.length++;
                    (*input)++;
                }
            }
            if(**input == '"') {
                token.length++;
                (*input)++;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
        }
        case 't':
            {
            token.type = TOKEN_TRUE;
            (*input)++;
            token.length = 1;
            if(strncmp(*input, "rue", 3) == 0) {
                token.length += 3;
                *input += 3;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
            }
        case 'T':
            {
            token.type = TOKEN_TRUE;
            (*input)++;
            token.length = 1;
            if(strncmp(*input, "RUE", 3) == 0) {
                token.length += 3;
                *input += 3;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
            }
        case 'f':
            {
            token.type = TOKEN_FALSE;
            (*input)++;
            token.length = 1;
            if(strncmp(*input, "alse",4) == 0) {
                token.length += 4;
                *input += 4;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
            }
        case 'F':
            {
            token.type = TOKEN_FALSE;
            (*input)++;
            token.length = 1;
            if(strncmp(*input, "ALSE",4) == 0){
                token.length +=4;
                *input += 4;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
            }
        case 'n':
            {
            token.type = TOKEN_NULL;
            (*input)++;
            token.length = 1;
            if(strncmp(*input, "ull", 3) == 0) {
                token.length += 3;
                *input += 3;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
            }
        case 'N':
            {
            token.type = TOKEN_NULL;
            (*input)++;
            token.length = 1;
            if(strncmp(*input, "ULL", 3) == 0) {
                token.length += 3;
                *input += 3;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;
            }
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9':
        case '-':
        {
            puts("number token");
            token.type = TOKEN_NUMBER;
            if(c == '-') (*input)++;
            while(**input && (**input >= '0' && **input <= '9')) {
                token.length++;
                (*input)++;
            }
            if(**input == '.') {
                token.length++;
                (*input)++;
                while(**input && (**input >= '0' && **input <= '9')) {
                    token.length++;
                    (*input)++;
                }
            }
            if(**input == 'e' || **input == 'E') {
                token.length++;
                (*input)++;
                if(**input == '+' || **input == '-') {
                    token.length++;
                    (*input)++;
                }
                while(**input && (**input >= '0' && **input <= '9')) {
                    token.length++;
                    (*input)++;
                }
            }
            break;
        }
        default:
            token.type = TOKEN_ERROR;
            token.length = 1;
            (*input)++;
            break;
    }
    printf("nextToken::Token type: %d, Token start: %.*s\n", token.type, token.length, token.start);
    return token;
}


JsonValue* parse_from_file(const char* path){
    FILE* file = fopen(path, "r");
    if(file == null) {
        puts("Failed to open file");
        return null;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(file_size+1);
    if(buffer == null) {
        fclose(file);
        puts("Failed to allocate memory");
        return null;
    }
    int32_t read_size = fread(buffer, 1, file_size, file);
    if(read_size != file_size) {
        free(buffer);
        fclose(file);
        puts("Failed to read file");
        return null;
    }
    buffer[read_size] = '\0';
    fclose(file);
    const char* input = buffer;
    JsonValue* result = parseValue(&input);
    free(buffer);
    return result;
}
void print_json(JsonValue* value, int indent_level) {
    if(value == null) {
        return;
    }
    char indent[128] = {0};
    for(int i = 0; i < indent_level; i++) {
        strcat(indent, "\t");
    }
    switch(value->type){
        case JSON_NULL:
            printf("null");
            break;
        case JSON_BOOLEAN:
            printf("%s", value->value.boolean ? "true" : "false");
            break;
        case JSON_NUMBER:
            printf("%lf", value->value.number);
            break;
        case JSON_STRING:
            printf("\"%s\"", value->value.string.data);
            break;
        case JSON_OBJECT:
            printf("{\n");
            for(int i = 0; i < value->value.object.count; i++) {
                printf("%s\"%s\": ", indent, value->value.object.keys[i]);
                print_json(value->value.object.values[i], indent_level + 1);
                if(i < value->value.object.count - 1) {
                    printf(",\n");
                }
            }
            printf("\n%s}", indent);
            break;
        case JSON_ARRAY:
            printf("[\n");
            printf("%s", indent);
            for(int i = 0; i < value->value.array.count; i++) {
                print_json(value->value.array.items[i], indent_level + 1);
                if(i < value->value.array.count - 1) {
                    printf(",");
                }
            }
            printf("\n%s]", indent);
            break;
        case JSON_ERROR:
            puts("Error parsing JSON");
            break;
        default:
            puts("Unknown JSON type");
            break;
    }
}

JsonValue* find_value(JsonValue* json, const char* key) {
    if(json->type != JSON_OBJECT) {
        return null;
    }
    for(int i = 0; i < json->value.object.count; i++) {
        if(strcmp(json->value.object.keys[i], key) == 0) {
            return json->value.object.values[i];
        }
    }
    return null;
}

int main(void) {
    JsonValue* json = parse_from_file("test.json");
    JsonValue* key = find_value(json, "array");
    //print_json(json, 0);
    print_json(key,0);
    puts("");

    return 0;
}