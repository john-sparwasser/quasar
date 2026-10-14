// Quasar: the virtual filesystem keeps source files between patches, validated by a stat. This
// patches three times, rewriting the included file in between (same size, same second), and checks
// that each patch assembled the file as it then was.
// Usage: asar-file-cache-test [asar_dll_path] [path_to_unheadered_SMW_ROM_file] [output_directory]

#include "asardll.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

static bool write_file(const std::string& path, const char* text)
{
	FILE* f = fopen(path.c_str(), "wb");
	if (!f) return false;
	fputs(text, f);
	fclose(f);
	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		printf("Usage: asar-file-cache-test [asar_dll_path] [path_to_unheadered_SMW_ROM_file] [output_directory]\n");
		return 0;
	}
	if (!asar_init_with_dll_path(argv[1]))
	{
		printf("Failed to load %s\n", argv[1]);
		return 1;
	}

	int buflen = asar_maxromsize();
	std::vector<char> rom((size_t)buflen);
	FILE* romfile = fopen(argv[2], "rb");
	if (!romfile)
	{
		printf("Failed to open %s\n", argv[2]);
		return 1;
	}
	int romlen_original = (int)fread(rom.data(), 1, (size_t)buflen, romfile);
	fclose(romfile);
	std::vector<char> original(rom.begin(), rom.begin() + romlen_original);

	std::string dir = std::string(argv[3]) + "/";
	std::string root = dir + "file-cache-root.asm";
	std::string inc = dir + "file-cache-inc.asm";
	if (!write_file(root, "org $008000\nincsrc file-cache-inc.asm\n"))
	{
		printf("Failed to write into %s\n", argv[3]);
		return 1;
	}

	const char* contents[] = { "db $11\n", "db $22\n", "db $33\n" };
	int failed = 0;
	for (int i = 0; i < 3; i++)
	{
		write_file(inc, contents[i]);
		memcpy(rom.data(), original.data(), original.size());
		int romlen = romlen_original;

		struct patchparams params;
		memset(&params, 0, sizeof(params));
		params.structsize = (int)sizeof(params);
		params.patchloc = root.c_str();
		params.romdata = rom.data();
		params.buflen = buflen;
		params.romlen = &romlen;
		params.should_reset = true;

		if (!asar_patch_ex(&params))
		{
			int count;
			const struct errordata* errors = asar_geterrors(&count);
			for (int j = 0; j < count; j++) printf("%s\n", errors[j].fullerrdata);
			failed++;
			continue;
		}

		unsigned char expected = (unsigned char)(0x11 * (i + 1));
		unsigned char actual = (unsigned char)rom[0];
		if (actual != expected)
		{
			printf("Patch %d assembled $%02X where $%02X was expected: the included file was served stale.\n", i + 1, actual, expected);
			failed++;
		}
	}

	asar_close();
	printf(failed ? "File cache test failed.\n" : "File cache test succeeded.\n");
	return failed ? 1 : 0;
}
