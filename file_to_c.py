#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
@File    : file_to_c.py
@Author  : Jason Wang (https://github.com/flywhc)
@Version : 1.1
@License : MIT. Note: the generated files may have to be GPL license if you use it in an Arduino GPL project.
@Desc    : convert file contents to C language string and generate corresponding header file
@Usage   : file_to_c.py <directory> [-r]
    <directory>: The directory to traverse.
    -r: Recursively traverse subdirectories.
    -c: Compress HTML, JS and CSS files.
    output: <directory>.c and <directory>.h
"""
import os
import re
import argparse
import binascii
import mimetypes
import rjsmin
import minify_html
import lightningcss
import gzip

MAX_LINE_LENGTH = 120

def split_c_string(s, max_length=MAX_LINE_LENGTH, is_text_file=True):
    """
    split long string into multiple lines, to fit C language string length limit
    :param s: the string to be split
    :param max_length: maximum length of each line
    :param is_text_file: is a a text file (True) or binary file (False).
    :return: the split string
    """
    if is_text_file:
        # process text file content to ensure no line break will break escape sequence
        lines = []
        start = 0
        while start < len(s):
            end = start + max_length
            if end >= len(s):
                lines.append(s[start:])
                break
            
            # ensure no line break will break escape sequence
            while end > start and s[end-1] == '\\':
                end -= 1
            
            lines.append(s[start:end])
            start = end

        # add quotes and join with commas
        return '\n'.join(f'"{line}"' for line in lines)
    else:
        # process non-text file content
        lines = []
        current_line = ''
        for part in s.split(','):
            if len(current_line) + len(part) + 1 <= max_length:  # +1 for comma
                current_line += part + ','
            else:
                lines.append(current_line[:-1])  # Remove trailing comma
                current_line = part + ','
        if current_line:  # Add the last line
            lines.append(current_line[:-1])  # Remove trailing comma

        return ',\n'.join(lines)
def file_contents_to_c_string(file_path, compress):
    """
    convert file contents to C language string. For text files, it directly converts, for other files, it converts to binary array
    :param file_path: the path of the file
    :return: the C language string
    """
    print(f"Processing file: {file_path}")
    _, ext = os.path.splitext(file_path)
    with open(file_path, 'rb') as file:
        ext = ext.lower()
        if ext in ['.htm', '.html', '.js', '.css']:
            content = file.read().decode('utf-8')
            if compress:
                # minify html/js/css
                if ext in ['.htm', '.html']:
                    content = minify_html.minify(content, minify_js=True, minify_css=True, remove_processing_instructions=True)
                elif ext == '.js':
                    content = rjsmin.jsmin(content)
                elif ext == '.css':
                    content = lightningcss.process_stylesheet(content)
                # compress html/js/css using gzip
                byte_content = content.encode('utf-8')
                compressed_data = gzip.compress(byte_content)
                return binary_to_c_string(compressed_data) + (True,)
            # return original content without compression
            # replace special characters for C string
            content = content.replace('"', '\\"').replace('\n', '\\n').replace('\r', '\\r')
            return split_c_string(f'{content}'), len(content) + (False,)
        else:
            return binary_to_c_string(file.read()) + (False,)
            

def binary_to_c_string(binary_data):
    """Convert binary data to C language string
    :param binary_data: the binary data to be converted
    :return: the C language string
    """
    byte_array = '0x' + binascii.hexlify(binary_data, ',').decode('ascii').replace(',', ', 0x')
    byte_array = split_c_string(byte_array, is_text_file=False)
    return f"{{\n{byte_array}\n}}", len(binary_data)

def replace_non_letter_digit(s):
    """
    replace all non-letter and digit characters to '_'
    
    :param s: input string
    
    :return: replaced string
    """
    return re.sub(r'[^a-zA-Z0-9_]', '_', s)

def generate_c_and_dict(directory, recursive, compress):
    """
    Generate C source file, header file and path-string dictionary considering file types
    :param directory: the directory to be processed
    :param recursive: whether to process subdirectories
    :return: None
    """
    
    # use the directory name as the C file name
    header_name = re.sub(r'\W|^(?=\d)', '_', directory).upper()
    c_content = f'#include "{header_name.lower()}.h" \n\n'
    file_index = "const ProgmemFileInformation progmemFiles[] = {\n"
    file_count = 0;
    content_type_list = []
    path_list = []

    # recursive function to process directory and read files
    def process_dir(dir_path, root_length):
        nonlocal c_content, file_index, file_count, compress, content_type_list, path_list
        for item_name in os.listdir(dir_path):
            item_path = os.path.join(dir_path, item_name)
            if os.path.isdir(item_path) and recursive:
                process_dir(os.path.join(dir_path, item_name), root_length)
            elif not item_name.startswith('.') and os.path.isfile(item_path):
                c_string, file_length, is_compressed = file_contents_to_c_string(item_path, compress)
                item_path = item_path[root_length:]
                c_var_name = 'v_' + re.sub(r'\W|^(?=\d)', '_', item_path).lower()
                c_content += f"const char {c_var_name}[] PROGMEM = \n{c_string};\n\n"
                
                # generate file_path
                path_key = item_path
                if os.sep == '\\':  # escape \\ for Windows
                    path_key = item_path.replace('\\', '/')
                path_list.append(path_key)
                    
                # generate content_type
                c_content_type = mimetypes.guess_type(item_path)[0]
                if c_content_type is None:
                    c_content_type = "application/octet-stream"
                if c_content_type not in content_type_list:  # add content type to list if not exists
                    content_type_list.append(c_content_type)
                content_type_key = replace_non_letter_digit(c_content_type)
                
                file_index += f"    {{ .file_path = {replace_non_letter_digit(path_key)}, .file_content = {c_var_name}, \
.file_length = {file_length}, .content_type = {content_type_key}, .is_compressed = {1 if is_compressed else 0} }},\n"
                file_count += 1

    process_dir(directory, len(directory) + 1)

    c_content += "// content types\n"
    # output list of content types
    for content_type in content_type_list:
        c_content += f"const char {replace_non_letter_digit(content_type)}[] PROGMEM = \"{content_type}\";\n"

    c_content += "\n// list of file paths\n"
    # output list of file paths
    for file_path in path_list:
        c_content += f"const char {replace_non_letter_digit(file_path)}[] PROGMEM = \"/{file_path}\";\n"
    
    c_content += "\n// file index\n"
    # output file index. Remove the last comma for the last member of struct array
    c_content += f"""{file_index}    {{ .file_path = 0, .file_content = 0, .file_length = 0, .content_type = 0, .is_compressed = 0 }},
}};
""";
    h_content = f"""#pragma once
#ifndef _{header_name}_H_
#define _{header_name}_H_
#include "ProgmemFileInformation.h"

extern const ProgmemFileInformation progmemFiles[];

#endif // _{header_name}_H_
"""

    with open(f"{directory}.c", "w", encoding='utf-8') as c_file:
        c_file.write(c_content)

    with open(f"{directory}.h", "w", encoding='utf-8') as h_file:
        h_file.write(h_content)
        
    return file_count

def main():
    parser = argparse.ArgumentParser(description="Convert files to C PROGMEM strings.")
    parser.add_argument("directory", help="The directory contains the files to be converted.")
#    parser.add_argument("-r", "--recursive", action="store_true", help="Recursively traverse subdirectories.")
    parser.add_argument("-o", "--original", action="store_true", help="Do not compress nor minify HTML, JS and CSS files by using gzip.")
    args = parser.parse_args()

    if args.directory.startswith('.'):
        print("Error: The directory cannot start with '.'.")
        exit(1)
    file_count = generate_c_and_dict(args.directory, True, not args.original)
    print(f"Conversion completed. {file_count} files converted.")

if __name__ == "__main__":
    main()