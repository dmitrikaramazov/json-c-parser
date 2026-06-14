# json-c-parser
This was a weekend project to better understand parsing techniques and JSON internals in C.

Usage: JsonValue *val = parse_from_file("test.json") which returns a typed JSON value tree. Also supports find_value(val, "key") for lookup and print_json(val, 0) for pretty-printing.

To build: gcc json.c -o json -Wall

```sh
gcc json.c -o json
./json
```

This implements a recursive descent parser from scratch. A handwritten tokenizer (nextToken) feeds tokens into separate parse functions for each JSON construct (RFC 8259). The parsed result is a tagged union of JsonValue nodes (object, array, string, number, boolean, null) with proper recursive freeing via free_json_value.

Features in this version:

-File input (parse_from_file)

-Pretty-printing with indentation (print_json)

-Key lookup on objects (find_value)

-Escape sequence handling in strings

-Case-insensitive literal parsing (true/false/null, e.g. TRUE / NULL / FALSE)


Future work:

Streaming / in-memory string input (beyond file-only)
Proper error reporting with line/column info
Serialisation (value tree → JSON string)



