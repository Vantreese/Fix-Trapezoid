#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


int fixtrapezoid(unsigned char *pix,
		unsigned char **pnmI, int W0, int H0,
		int W, int H,
		int x, int y,
		int ULx, int ULy,
		int URx, int URy,
		int LRx, int LRy,
		int LLx, int LLy
		);

/*
int interpolate(unsigned char *pix,
		unsigned char **pnmI, int W0, int H0,
		int W, int H,
		int x, int y,
		int ULx, int ULy,
		int URx, int URy,
		int LRx, int LRy,
		int LLx, int LLy
		);
*/

int pixvalue(unsigned char *pix,
		unsigned char **pnmI, int w, int h, double x, double y
		);

int pgmpixsize = sizeof(unsigned char);
int ppmpixsize = 3 * sizeof(unsigned char);

int pgmpixsizep = sizeof(unsigned char*);
int ppmpixsizep = 3 * sizeof(unsigned char*);

int pixsize;
int pixsizep;

int main(int argc, char *argv[])
{
	int width0 = 0, height0 = 0, max0 = 0;
	FILE *fi = stdin;
	FILE *fo = stdout;
	FILE *fe = stderr;
	unsigned char **pnmI;
	unsigned char **pnmO;
	int xloop, yloop;
	char imgkey[81];

	int ULx = 0;
	int ULy = 0;
	int URx = 0;
	int URy = 0;
	int LRx = 0;
	int LRy = 0;
	int LLx = 0;
	int LLy = 0;

	int W = 0;
	int H = 0;

	_setmode(_fileno(fo), _O_BINARY);

	if (argc > 1)
	{
		fi = fopen(argv[1], "rb");
	}
	else
	{
		fprintf(fe, "%s <PGM Image In> [ULx,ULy] [URx,URy] [LRx,LRy] "
			"[LLx,LLy] <to Width>x<to Height> <PGM Image Out>\n",
			argv[0]);
		return 1;
	}
	fscanf(fi, "%s\n%i %i\n%i\n", imgkey, &width0, &height0, &max0);
	fprintf(fe, "Key = %s\nWidth = %d\nHeight = %d\nMax = %d\n",
			imgkey, width0, height0, max0);
	URx = width0;
	LLy = height0;
	LRx = width0;
	LRy = height0;
	if (!strncmp(imgkey, "P5"))
	{
		pixsize = pgmpixsize;
		pixsizep = ppmpixsizep;
	}
	if (!strncmp(imgkey, "P6"))
	{
		pixsize = ppmpixsize;
		pixsizep = ppmpixsizep;
	}



	if (argc > 2)
	{
		sscanf(argv[2], "[%i,%i]", &ULx, &ULy);
	}
	if (argc > 3)
	{
		sscanf(argv[3], "[%i,%i]", &URx, &URy);
	}
	if (argc > 4)
	{
		sscanf(argv[4], "[%i,%i]", &LRx, &LRy);
	}
	if (argc > 5)
	{
		sscanf(argv[5], "[%i,%i]", &LLx, &LLy);
	}
	if (argc > 6)
	{
		sscanf(argv[6], "%ix%i", &W, &H);
	}
	if (argc > 7)
	{
		fo = fopen(argv[7], "wb"); 
	}


	fprintf(fe, "UL: [%d, %d]\n", ULx, ULy);
	fprintf(fe, "UR: [%d, %d]\n", URx, URy);
	fprintf(fe, "LR: [%d, %d]\n", LRx, LRy);
	fprintf(fe, "LL: [%d, %d]\n", LLx, LLy);
	fprintf(fe, "Readjust to: %dx%d\n", W, H);

	pnmI = calloc(height0, pixsizep);
	for (yloop = 0; yloop < height0; ++yloop)
	{
		pnmI[yloop] = calloc(width0, pixsize);
		fread(pnmI[yloop], pixsize, width0, fi);
	}
	fclose(fi);

	pnmO = calloc(H, pixsizep);
	for (yloop = 0; yloop < H; ++yloop)
	{
		pnmO[yloop] = calloc(W, pixsize);
		for (xloop = 0; xloop < W; ++xloop)
		{
			switch (pixsize)
			{
			case 1:
				fixtrapezoid(&(pnmO[yloop][xloop]),
					pnmI, width0, height0,
					W, H,
					xloop, yloop,
					ULx, ULy,
					URx, URy,
					LRx, LRy,
					LLx, LLy
					);
				break;
			case 3:
				fixtrapezoid(&(pnmO[yloop][3*xloop]),
					pnmI, width0, height0,
					W, H,
					xloop, yloop,
					ULx, ULy,
					URx, URy,
					LRx, LRy,
					LLx, LLy
					);
				break;
			}
		}
	}

	switch (pixsize)
	{
	case 1:
		fprintf(fo, "P5\n%d %d\n%d\n", W, H, max0);
		break;
	case 3:
		fprintf(fo, "P6\n%d %d\n%d\n", W, H, max0);
		break;
	}
	fflush(fo);
	for (yloop = 0; yloop < H; ++yloop)
	{
		fwrite(pnmO[yloop], pixsize, W, fo);
		fflush(fo);
	}
	fclose(fo);

	return 0;
}



int fixtrapezoid(unsigned char *pix,
		unsigned char **pnmI, int W0, int H0,
		int W, int H,
		int x, int y,
		int ULx, int ULy,
		int URx, int URy,
		int LRx, int LRy,
		int LLx, int LLy
		)
{
	double Wc = (double) (W - 1);
	double Hc = (double) (H - 1);
	double dULx = (double) (ULx);
	double dULy = (double) (ULy);
	double dURx = (double) (W - 1 - URx);
	double dURy = (double) (URy);
	double dLRx = (double) (W - 1 - LRx);
	double dLRy = (double) (H - 1 - LRy);
	double dLLx = (double) (LLx);
	double dLLy = (double) (H - 1 - LLy);
	double x0 = (double) x;
	double y0 = (double) y;

	double x1 = ((Hc - y0) * ((Wc - x0) * (dULx + x0) + x0 * (x0 - dURx))
			+ y0 * ((Wc - x0) * (x0 + dLLx) + x0 * (x0 - dLRx)))
		/ (Wc * Hc);
	double y1 = ((Wc - x0) * ((Hc - y0) * (dULy + y0) + y0 * (y0 - dLLy))
			+ x0 * ((Hc - y0) * (dURy + y0) + y0 * (y0 - dLRy)))
		/ (Wc * Hc);

	return pixvalue(pix, pnmI, W0, H0, x1, y1);
}



int pixvalue(unsigned char *pix,
		unsigned char **pnmI, int w, int h, double x, double y
		)
{
	int x1 = (int) x;
	int y1 = (int) y;
	int havepix = 1;

	if ((x1 < 0) || (y1 < 0))
	{
		havepix = 0;
	}
	if ((x1 >= w) || (y1 >= h))
	{
		havepix = 0;
	}
	if (havepix > 0)
	{
		switch (pixsize)
		{
		case 1:
			pix[0] = (pnmI[y1][x1]);
			break;
		case 3:
			pix[0] = (pnmI[y1][3*x1]);
			pix[1] = (pnmI[y1][3*x1+1]);
			pix[2] = (pnmI[y1][3*x1+2]);
			break;
		}
	}
	else
	{
		//pix[0] = 0;
	}
	return 0;
}



