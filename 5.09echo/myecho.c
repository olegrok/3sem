#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[])
{
	const char key_n[] = "-n";
	const char key_ver[] = "--version";
	const char key_help[] = "--help";
	int i = 0;
	int flag = 0;

	if (argc == 1) {
		printf("\n");
		return 0;
	}
	if (argc >= 2) {
		if (!strcmp(key_ver, argv[1]) && argc == 2) {
			printf("Version 0.01 by Babin Oleg\n");
			return 0;
		}
		if (!strcmp(key_help, argv[1]) && argc == 2) {
			printf("Use ./my_echo \n");
			return 0;
		}
		flag = strcmp(key_n, argv[1]);
		if (flag != 0)
			i = 1;
		else
			i = 2;
		for (i; i < argc; i++) {
			printf("%s", argv[i]);
			if (i < argc - 1)
				printf(" ");
			else if (flag != 0)
				printf("\n");
		}

	}

	return 0;
}
