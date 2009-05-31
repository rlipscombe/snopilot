/* DumpDem.cpp
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <malloc.h>
#include <assert.h>

struct RecordA {
	std::string filename;		// A40
	std::string producer;		// A60
								// 9 bytes
	int sw_corner_lon_deg;		// I4
	int sw_corner_lon_min;		// I2
	double sw_corner_lon_sec;	// F7.4
	int sw_corner_lat_deg;		// I4
	int sw_corner_lat_min;		// I2
	double sw_corner_lat_sec;	// F7.4
	char process_code;			// A1
								// 1 byte
	std::string sectional_id;	// A3
	std::string origin_code;	// A4
	int dem_level_code;			// I6
	int elev_pattern_code;		// I6
	int gnd_horiz_ref_code;		// I6
	int gnd_horiz_ref_zone;		// I6

	double proj_params[15];		// 15(D24.15)
	int horiz_unit;				// I6
	int vert_unit;				// I6
	int sides_in_poly;			// I6
	double sw_corner_lon;		// D24.15
	double sw_corner_lat;		// D24.15
	double nw_corner_lon;		// D24.15
	double nw_corner_lat;		// D24.15
	double ne_corner_lon;		// D24.15
	double ne_corner_lat;		// D24.15
	double se_corner_lon;		// D24.15
	double se_corner_lat;		// D24.15
	double min_elev;			// D24.15
	double max_elev;			// D24.15
	double ccw_angle;			// D24.15
	int elev_accur_code;		// I6

	double spatial_res_x;		// E12.6
	double spatial_res_y;		// E12.6
	double spatial_res_z;		// E12.6
	int num_rows;				// I6 -- usually 1
	int num_cols;				// I6 -- usually 1201
	int lgst_prim_cont_itv;		// I5
	int lgst_src_cont_itv;		// I1
	int smst_prim_cont_itv;		// I5
	int smst_src_cont_itv;		// I1
	int data_source_date;		// I4

	int inspection_date;		// I4
	char inspection_flag;		// A1
	int validation_flag;		// I1
	int suspect_area_flag;		// I2
	int vertical_datum;			// I2
	int horiz_datum;			// I2
	int data_edition;			// I4

	int pct_void;				// I4
	int edge_match_flag_w;		// I2
	int edge_match_flag_n;		// I2
	int edge_match_flag_e;		// I2
	int edge_match_flag_s;		// I2
	double vert_datum_shift;	// F7.2

	// Followed by padding to take it up to 1024 bytes
};

// A .DEM file in CDED format is 1201x1201.  Each Type B record is a column.  There are 1201 of them.  They each contain 1201 rows.

void ReadA(FILE *f, std::string *a, int n)
{
	char *temp = (char *)_alloca(n+1);
	fread(temp, n, 1, f);
	a->assign(temp, n);
}

void ReadA1(FILE *f, char *a)
{
	fread(a, 1, 1, f);
}

void ReadFiller(FILE *f, int n)
{
	char *temp = (char *)_alloca(n+1);
	fread(temp, n, 1, f);
}

void ReadI(int *i, int n, FILE *f)
{
	char *temp = (char *)_alloca(n+1);
	fread(temp, n, 1, f);
	temp[n] = '\0';

	*i = strtol(temp, NULL, 10);
}

void ReadI1(FILE *f, int *i)
{
	ReadI(i, 1, f);
}

void ReadI2(FILE *f, int *i)
{
	ReadI(i, 2, f);
}

void ReadI4(FILE *f, int *i)
{
	ReadI(i, 4, f);
}

void ReadI5(FILE *f, int *i)
{
	ReadI(i, 5, f);
}

void ReadI6(FILE *f, int *i)
{
	ReadI(i, 6, f);
}

void ReadDEF(double *g, int n, FILE *f)
{
	char *temp = (char *)_alloca(n+1);
	fread(temp, n, 1, f);
	temp[n] = '\0';

	*g = strtod(temp, NULL);
}

void ReadF7_4(FILE *f, double *g)
{
	ReadDEF(g, 7, f);
}

void ReadE12_6(FILE *f, double *g)
{
	ReadDEF(g, 12, f);
}

void ReadD24_15(FILE *f, double *g)
{
	ReadDEF(g, 24, f);
}

/** Records are aligned on 1024-byte boundaries.  When we finish reading one record, we call
 * this function to skip to the start of the next.
 */
void SeekToNextRecord(FILE *f)
{
	long pos = ftell(f);
	if (pos % 1024)
	{
		long extra = 1024 - (pos % 1024);
		fseek(f, extra, SEEK_CUR);

		pos = ftell(f);
		assert((pos % 1024) == 0);
	}
}

void ReadRecordTypeA(RecordA *a, FILE *f)
{
	ReadA(f, &a->filename, 40);
	ReadA(f, &a->producer, 60);
	ReadFiller(f, 9);
	ReadI4(f, &a->sw_corner_lon_deg);
	ReadI2(f, &a->sw_corner_lon_min);
	ReadF7_4(f, &a->sw_corner_lon_sec);
	ReadI4(f, &a->sw_corner_lat_deg);
	ReadI2(f, &a->sw_corner_lat_min);
	ReadF7_4(f, &a->sw_corner_lat_sec);
	ReadA1(f, &a->process_code);
	ReadFiller(f, 1);
	ReadA(f, &a->sectional_id, 3);
	ReadA(f, &a->origin_code, 4);
	ReadI6(f, &a->dem_level_code);
	ReadI6(f, &a->elev_pattern_code);		// I6
	ReadI6(f, &a->gnd_horiz_ref_code);		// I6
	ReadI6(f, &a->gnd_horiz_ref_zone);		// I6

	for (int i = 0; i < 15; ++i)
		ReadD24_15(f, &a->proj_params[i]);
	ReadI6(f, &a->horiz_unit);
	ReadI6(f, &a->vert_unit);
	ReadI6(f, &a->sides_in_poly);
	ReadD24_15(f, &a->sw_corner_lon);
	ReadD24_15(f, &a->sw_corner_lat);
	ReadD24_15(f, &a->nw_corner_lon);
	ReadD24_15(f, &a->nw_corner_lat);
	ReadD24_15(f, &a->ne_corner_lon);
	ReadD24_15(f, &a->ne_corner_lat);
	ReadD24_15(f, &a->se_corner_lon);
	ReadD24_15(f, &a->se_corner_lat);
	ReadD24_15(f, &a->min_elev);
	ReadD24_15(f, &a->max_elev);
	ReadD24_15(f, &a->ccw_angle);
	ReadI6(f, &a->elev_accur_code);

	ReadE12_6(f, &a->spatial_res_x);
	ReadE12_6(f, &a->spatial_res_y);
	ReadE12_6(f, &a->spatial_res_z);
	ReadI6(f, &a->num_rows);
	assert(a->num_rows == 1);
	ReadI6(f, &a->num_cols);
	assert(a->num_cols == 1201);
	ReadI5(f, &a->lgst_prim_cont_itv);
	ReadI1(f, &a->lgst_src_cont_itv);
	ReadI5(f, &a->smst_prim_cont_itv);
	ReadI1(f, &a->smst_src_cont_itv);
	ReadI4(f, &a->data_source_date);

	ReadI4(f, &a->inspection_date);
	ReadA1(f, &a->inspection_flag);
	ReadI1(f, &a->validation_flag);
	ReadI2(f, &a->suspect_area_flag);
	ReadI2(f, &a->vertical_datum);
	ReadI2(f, &a->horiz_datum);
	ReadI4(f, &a->data_edition);

	ReadI4(f, &a->pct_void);
	ReadI2(f, &a->edge_match_flag_w);
	ReadI2(f, &a->edge_match_flag_n);
	ReadI2(f, &a->edge_match_flag_e);
	ReadI2(f, &a->edge_match_flag_s);
	ReadF7_4(f, &a->vert_datum_shift);

	SeekToNextRecord(f);
}

struct RecordB
{
	int row_id;	// I6
	int col_id;	// I6
	int num_rows;	// I6 -- usually 1201
	int num_cols;	// I6 -- usually 1
	double first_elev_lat;	// D24.15
	double first_elev_lon;	// D24.15
	double elev_local_vert_datum;	// D24.15
	double min_elev;	// D24.15
	double max_elev;	// D24.15

	// This is then followed by num_rows * num_cols elevation entries for the profile.
	// They're relative to elev_local_vert_datum.
	int elevations[1201];
};

void ReadRecordTypeB(RecordB *b, FILE *f)
{
	ReadI6(f, &b->row_id);
	ReadI6(f, &b->col_id);
	ReadI6(f, &b->num_rows);
	assert(b->num_rows == 1201);
	ReadI6(f, &b->num_cols);
	assert(b->num_cols == 1);
	ReadD24_15(f, &b->first_elev_lat);
	ReadD24_15(f, &b->first_elev_lon);
	ReadD24_15(f, &b->elev_local_vert_datum);
	ReadD24_15(f, &b->min_elev);
	ReadD24_15(f, &b->max_elev);

	// Because of the way the file is partitioned into records, the first record
	// has the B header, followed by 146 elevation points
	// (4I6 and 5D24.15 is 144 bytes, leaving 880 bytes of I6 data, so 146 points).
	//
	// Then there's 6 records, each with 170 points in (170I6 is 1020 bytes).
	//
	// Then there's 1 record, with 35 points in.

	for (int i = 0; i < 146; ++i)
	{
		ReadI6(f, &b->elevations[i]);
	}

	SeekToNextRecord(f);

	// Then there's 6 records of 170 elevations.
	for (int j = 0; j < 6; ++j)
	{
		for (int i = 0; i < 170; ++i)
		{
			int k = 146 + (j * 170) + i;
			ReadI6(f, &b->elevations[k]);
		}

		SeekToNextRecord(f);
	}

	// Then there's a record with 35 points in.
	for (int i = 0; i < 35; ++i)
	{
		int k = 146 + (6*170) + i;
		ReadI6(f, &b->elevations[k]);
	}

#if 0
	for (int i = 0; i < 1201; ++i)
	{
		ReadI6(f, &b->elevations[i]);
		if (b->elevations[i] == 0)
		{
			long pos = ftell(f);
			fprintf(stderr, "Warning: Found zero elevation value at byte %d (0x%x)\n", pos, pos);

			printf("row_id = %d\n", b->row_id);
			printf("col_id = %d\n", b->col_id);
			printf("num_rows = %d\n", b->num_rows);
			printf("num_cols = %d\n", b->num_cols);
			printf("first_elev_lat = %f\n", b->first_elev_lat);
			printf("first_elev_lon = %f\n", b->first_elev_lon);
			printf("elev_local_vert_datum = %f\n", b->elev_local_vert_datum);
			printf("min_elev = %f\n", b->min_elev);
			printf("max_elev = %f\n", b->max_elev);

			for (int row = 0; row <= i; ++row)
			{
				printf("elevation[%d] = %d\n", row, b->elevations[row]);
			}

			exit(2);
		}
	}
#endif

	SeekToNextRecord(f);
}

#if 0
COLORREF GetElevationColour(double elevation, double min_elev, double max_elev)
{
	// Divide the mountain into seven ranges.
	double elev_range = max_elev - min_elev;
	double elev_relative = elevation - min_elev;

	if (elev_relative < elev_range / 7)
	{
		// Bottom chunk: black->white.
		double range_bottom = 0;
		double range_top = elev_range / 7;
		double m = 255 / (range_top - range_bottom);
		int grey = (int)(m * (elev_relative - range_bottom));
		return RGB(grey, grey, grey);
	}
	else if (elev_relative < 2 * elev_range / 7)
	{
		// Next bit: black->blue
		double range_bottom = elev_range / 7;
		double range_top = 2 * elev_range / 7;
		double m = 255 / (range_top - range_bottom);
		int grey = (int)(m * (elev_relative - range_bottom));
		return RGB(0, 0, grey);
	}
	else if (elev_relative < 3 * elev_range / 7)
	{
		// black->green
		double range_bottom = 2 * elev_range / 7;
		double range_top = 3 * elev_range / 7;
		double m = 255 / (range_top - range_bottom);
		int grey = (int)(m * (elev_relative - range_bottom));
		return RGB(0, grey, 0);
	}
	else if (elev_relative < 4 * elev_range / 7)
	{
		double range_bottom = 3 * elev_range / 7;
		double range_top = 4 * elev_range / 7;
		double m = 255 / (range_top - range_bottom);
		int grey = (int)(m * (elev_relative - range_bottom));
		return RGB(0, grey, grey);
	}
	else if (elev_relative < 5 * elev_range / 7)
	{
		double range_bottom = 4 * elev_range / 7;
		double range_top = 5 * elev_range / 7;
		double m = 255 / (range_top - range_bottom);
		int grey = (int)(m * (elev_relative - range_bottom));
		return RGB(grey,0,0);
	}
	else if (elev_relative < 6 * elev_range / 7)
	{
		double range_bottom = 5 * elev_range / 7;
		double range_top = 6 * elev_range / 7;
		double m = 255 / (range_top - range_bottom);
		int grey = (int)(m * (elev_relative - range_bottom));
		return RGB(grey,0,grey);
	}
	else
	{
		double range_bottom = 6 * elev_range / 7;
		double range_top = 7 * elev_range / 7;
		double m = 255 / (range_top - range_bottom);
		int grey = (int)(m * (elev_relative - range_bottom));
		return RGB(grey,grey,0);
	}
}
#endif

#if 0
COLORREF GetElevationColour(double elevation, double min_elev, double max_elev)
{
	const double min_interesting_elev = 750.0;
	const double max_interesting_elev = 1950.0;

	// We've only got a particular range of elevations that we're interested in,
	// so we'll colour everything else in a false colour.
	if (elevation < min_interesting_elev)
		return RGB(0,0,255);

	if (elevation > max_interesting_elev)
		return RGB(0,0,128);

	// OK.  We'll do this in black and white for now.  We need to wedge 256 values
	// in the gap between min_elev and max_elev.
	// The easiest way to do this is to draw a line so that min_elev => 0x00
	// and max_elev => 0xFF.  Using y=mx+c, we can work out the colour for
	// any elevation.

	double m = 255 / (max_interesting_elev - min_interesting_elev);
	int grey = (int)(m * (elevation - min_interesting_elev));
	if (grey >= 0 && grey <= 255)
	{
		if (grey & 1) 
			return RGB(grey, grey, grey);
		else
			return RGB(255-grey,255-grey,255-grey);
	}
	else
		return RGB(255,0,0);
}
#endif

#if 1
COLORREF GetElevationColour(double elevation, double min_elev, double max_elev)
{
	// OK.  We'll do this in black and white for now.  We need to wedge 256 values
	// in the gap between min_elev and max_elev.
	// The easiest way to do this is to draw a line so that min_elev => 0x00
	// and max_elev => 0xFF.  Using y=mx+c, we can work out the colour for
	// any elevation.

	double m = 255 / (max_elev - min_elev);
	double y = m * (elevation - min_elev);
	if (y >= 0 && y <= 255)
		return RGB(y, y, y);
	else
		return RGB(255,0,0);
}
#endif

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage:\n\tDumpDem filename.dem filename.bmp\n");
		return 1;
	}

	const char *filename = argv[1];

	FILE *f = fopen(filename, "rb");
	if (!f)
	{
		fprintf(stderr, "Couldn't open file '%s'\n", filename);
		return 1;
	}

	// A .DEM file starts with a type A record.
	RecordA a;

	ReadRecordTypeA(&a, f);

	int imageBytes = dib::GetBitmapBytes(1201, 1201, 24);

	// Write that out to a .BMP file.  A .BMP file starts with a BITMAPFILEHEADER
	BITMAPFILEHEADER bmfh;
	bmfh.bfType = MAKEWORD('B','M');
	bmfh.bfSize = 0x420c0a;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = 0x0036;

	// That's then followed by a BITMAPINFO structure.
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = 0x28;
	bmi.bmiHeader.biWidth = 1201;
	bmi.bmiHeader.biHeight = 1201;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = imageBytes;
	bmi.bmiHeader.biXPelsPerMeter = 0x2e23;
	bmi.bmiHeader.biYPelsPerMeter = 0x2e23;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;

	// Then it's followed by the pixel data.
	char *pixelData = (char *)malloc(imageBytes);
	memset(pixelData, 0xFF, imageBytes);

	// Then there are num_rows * num_cols B records.  Generally num_rows == 1, num_cols == 1201.
	for (int col = 0; col < 1201; ++col)
	{
		RecordB b;
		ReadRecordTypeB(&b, f);

		for (int row = 0; row < 1201; ++row)
		{
			RGBTRIPLE *pixel = dib::GetPixelPtr(pixelData, 1201, 1201, 24, col, row);

			double elevation = b.elev_local_vert_datum + b.elevations[row];
			COLORREF cr = GetElevationColour(elevation, a.min_elev, a.max_elev);
//			COLORREF cr = RGB(0,0,0);

			pixel->rgbtRed = GetRValue(cr);
			pixel->rgbtGreen = GetGValue(cr);
			pixel->rgbtBlue = GetBValue(cr);
		}
	}

	fclose(f);

	// On the bitmap, mark the point at 50.11168N 122.925697W in blue.
	// We know where the SW corner of the bitmap is.  Each pixel is 0.75 arc-sec
	// So, we need to subtract our position from the SW corner, and convert into arc-sec.

	// The SW corner position in the A record is a six-part fucked up value:
	//a.sw_corner_lat_deg;
	//a.sw_corner_lat_min;
	//a.sw_corner_lat_sec;
	//a.sw_corner_lon_deg;
	//a.sw_corner_lon_min;
	//a.sw_corner_lon_sec;

	// If we convert this into arc-seconds, it'll be easier to deal with:
	double sw_corner_lat_arcsecs = (a.sw_corner_lat_deg * 3600) + (a.sw_corner_lat_min * 60) + a.sw_corner_lat_sec;
	double sw_corner_lon_arcsecs = (a.sw_corner_lon_deg * 3600) + (a.sw_corner_lon_min * 60) + a.sw_corner_lon_sec;
	// Note: this value is also available elsewhere in the A record.

	// Then, if we have our position in arc-seconds:
	double pos_lat_arcsecs = 50.111168 * 3600;
	double pos_lon_arcsecs = -122.925697 * 3600;

	// ...we can work out the difference in arc-seconds:
	double diff_lat_arcsecs = pos_lat_arcsecs - sw_corner_lat_arcsecs;
	double diff_lon_arcsecs = pos_lon_arcsecs - sw_corner_lon_arcsecs;

	// Then we can convert this into pixels:
	int pos_x_pels = (int)(diff_lon_arcsecs / 0.75);
	int pos_y_pels = (int)(diff_lat_arcsecs / 0.75);

	// And, finally, draw a different coloured pixel at that position:
	assert(pos_x_pels >= 0 && pos_x_pels < 1201);
	assert(pos_y_pels >= 0 && pos_y_pels < 1201);
	RGBTRIPLE *pos_ptr = dib::GetPixelPtr(pixelData, 1201, 1201, 24, pos_x_pels, pos_y_pels);
	pos_ptr->rgbtRed = 0;
	pos_ptr->rgbtGreen = 0xFF;
	pos_ptr->rgbtBlue = 0;

	for (int y = 0; y < 1201; ++y)
	{
		RGBTRIPLE *p = dib::GetPixelPtr(pixelData, 1201, 1201, 24, pos_x_pels, y);
		p->rgbtRed = 0;
		p->rgbtGreen = 0xFF;
		p->rgbtBlue = 0;
	}

	for (int x = 0; x < 1201; ++x)
	{
		RGBTRIPLE *p = dib::GetPixelPtr(pixelData, 1201, 1201, 24, x, pos_y_pels);
		p->rgbtRed = 0;
		p->rgbtGreen = 0xFF;
		p->rgbtBlue = 0;
	}

	HANDLE hBmp = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hBmp != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		WriteFile(hBmp, &bmfh, sizeof(BITMAPFILEHEADER), &bytesWritten, NULL);
		WriteFile(hBmp, &bmi, sizeof(BITMAPINFOHEADER), &bytesWritten, NULL);
		WriteFile(hBmp, pixelData, imageBytes, &bytesWritten, NULL);
		CloseHandle(hBmp);
	}
	else
		fprintf(stderr, "CreateFile failed, '%s', error = %d.\n", argv[2], GetLastError());

	free(pixelData);

	return 0;
}
