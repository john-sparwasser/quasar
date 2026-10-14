#pragma once

#include "libstr.h"

bool file_exists(const char * filename);
bool path_is_absolute(const char* path);
char get_native_path_separator();
bool check_is_regular_file(const char* path);

// Quasar: what identifies a file's current contents without reading it, for the file cache in
// virtualfile.cpp. A platform without a cheap, fine-grained answer returns false and is not cached.
struct file_stamp
{
	bool regular;
	unsigned long long size;
	unsigned long long mtime_ns;
	unsigned long long inode;
	unsigned long long dev;
};
bool get_file_stamp(const char* path, file_stamp* out);

string dir(char const *name);
string create_combined_path(const char* path1, const char* path2);
string normalize_path(const char* path);
string get_base_name(char const *name);
