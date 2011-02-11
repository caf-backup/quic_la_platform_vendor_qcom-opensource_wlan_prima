#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#define LINE_LEN	256

typedef struct sDelta {
	char name[LINE_LEN];
	char val[LINE_LEN];
}tDelta;

char *upper(char *str)
{
	static char ustr[LINE_LEN];
	int i;
	int len = strlen(str);

	for (i = 0; i < len; i++) {
		if (islower(str[i])) {
			ustr[i] = toupper(str[i]);
		} else {
			ustr[i] = str[i];
		}
	}
	ustr[i] = '\0';

	return(ustr);
}

int
main(int argc, char **argv)
{
	FILE *fpr;
	FILE *fprc;
	FILE *fpw;
	char line[LINE_LEN];
	tDelta delta[LINE_LEN];
	int i;
	int idelta;
	int flag = 0;
	int count = 0;
	int c;
	char srcdir[LINE_LEN];
	char dstdir[LINE_LEN];
	char infile[LINE_LEN];
	char outfile[LINE_LEN];
	char deltafile[LINE_LEN];

	printf("Usage : %s [-s src ] [-d dst dir]\n", argv[0]);
	printf("\t-s to specify source directory (for cfg.txt.orig, cfg.delta)\n");
	printf("\t-d to specify destination directory (for outputting cfg.txt)\n");

	strcpy(srcdir, "/tmp");
	strcpy(dstdir, "/tmp");

	while ((c = getopt(argc, argv, "sd")) != EOF) {

		switch (c) {
		case 's':
			if (optind > 0 && optind < argc &&
					argv[optind][0] != '-') {

				strcpy(srcdir, argv[optind]);

			} else {

				printf("Too few arguments to -s\n");
				return -1;

			}
			break;

		case 'd':
			if (optind > 0 && optind < argc &&
					argv[optind][0] != '-') {

				strcpy(dstdir, argv[optind]);

			} else {

				printf("Too few arguments to -d\n");
				return -1;

			}
			break;

		default:
			printf("Invalid Argument '%c'\n", c);
			return -1;
			break;

		}

	}

	sprintf(deltafile, "%s/%s", srcdir, "cfg.delta");
	sprintf(infile, "%s/%s", srcdir, "cfg.txt.orig");
	sprintf(outfile, "%s/%s", dstdir, "cfg.txt");

	printf("deltafile %s, infile %s, outfile %s\n",
			deltafile, infile, outfile);

	i = 0;

	if ((fprc = fopen(deltafile, "r")) == NULL) {

		printf("No delta file found %s\n", deltafile);
		// No Delta file found

	} else {

		while (fgets(line, LINE_LEN, fprc)) {

			sscanf(line, "%s %s", delta[i].name, delta[i].val);
			i++;

		}

		fclose(fprc);

	}

	idelta = i;

	if ((fpr = fopen(infile, "r")) == NULL) {

		printf("Unable to open file for Reading %s\n", infile);
		exit(1);

	}

	if ((fpw = fopen(outfile, "w")) == NULL) {

		printf("Unable to open file for Writting %s\n", outfile);
		exit(2);

	}

	fflush(stdout);

	while (fgets(line, LINE_LEN, fpr)) {
		char name[LINE_LEN];
		name[0] = '\0';

		if (flag == 0) {

			sscanf(line, "%s", name);

			for (i = 0; i < idelta; i++) {

				if (strcmp(name, upper(delta[i].name)) == 0) {

						flag = 1;
						count = 1;
						break;

				}
			}

			fputs(line, fpw);

		} else {
			char min[LINE_LEN];
			char max[LINE_LEN];
			char def[LINE_LEN];

			count++;

			if ((count == 4) || (count == 7)) {

				if (count == 7) {

					flag = 0;

				}

				sscanf(line, "%s %s %s", min, max, def);

				fprintf(fpw, "%s    %s    %s\n", min, max,
						delta[i].val);

			} else {

				fputs(line, fpw);

			}

		}

	}

	fclose(fpr);
	fclose(fpw);

	exit(0);
}
