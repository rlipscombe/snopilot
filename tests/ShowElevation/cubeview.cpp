// cubeview.cpp : implementation of the CCubeView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "cube.h"

#include "cubedoc.h"
#include "cubeview.h"

#include "gl/gl.h"
#include "gl/glu.h"

#include <string>
#include <assert.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

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

unsigned char threeto8[8] =
{
	0, 0111>>1, 0222>>1, 0333>>1, 0444>>1, 0555>>1, 0666>>1, 0377
};

unsigned char twoto8[4] =
{
	0, 0x55, 0xaa, 0xff
};

unsigned char oneto8[2] =
{
	0, 255
};

static int defaultOverride[13] =
{
	0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
};

static PALETTEENTRY defaultPalEntry[20] =
{
	{ 0,   0,   0,    0 },
	{ 0x80,0,   0,    0 },
	{ 0,   0x80,0,    0 },
	{ 0x80,0x80,0,    0 },
	{ 0,   0,   0x80, 0 },
	{ 0x80,0,   0x80, 0 },
	{ 0,   0x80,0x80, 0 },
	{ 0xC0,0xC0,0xC0, 0 },

	{ 192, 220, 192,  0 },
	{ 166, 202, 240,  0 },
	{ 255, 251, 240,  0 },
	{ 160, 160, 164,  0 },

	{ 0x80,0x80,0x80, 0 },
	{ 0xFF,0,   0,    0 },
	{ 0,   0xFF,0,    0 },
	{ 0xFF,0xFF,0,    0 },
	{ 0,   0,   0xFF, 0 },
	{ 0xFF,0,   0xFF, 0 },
	{ 0,   0xFF,0xFF, 0 },
	{ 0xFF,0xFF,0xFF, 0 }
};

/////////////////////////////////////////////////////////////////////////////
// CCubeView

IMPLEMENT_DYNCREATE(CCubeView, CView)

BEGIN_MESSAGE_MAP(CCubeView, CView)
	//{{AFX_MSG_MAP(CCubeView)
	ON_COMMAND(ID_FILE_PLAY, OnFilePlay)
	ON_UPDATE_COMMAND_UI(ID_FILE_PLAY, OnUpdateFilePlay)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCubeView construction/destruction

CCubeView::CCubeView()
{
	m_pDC = NULL;
	m_play = FALSE;
}

CCubeView::~CCubeView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCubeView drawing

void CCubeView::OnDraw(CDC* /*pDC*/)
{
	CCubeDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	DrawScene();
}

/////////////////////////////////////////////////////////////////////////////
// CCubeView diagnostics

#ifdef _DEBUG
void CCubeView::AssertValid() const
{
	CView::AssertValid();
}

void CCubeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCubeDoc* CCubeView::GetDocument() // non-debug version is inline
{
	return STATIC_DOWNCAST(CCubeDoc, m_pDocument);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCubeView message handlers

void CCubeView::OnFilePlay()
{
	m_play = m_play ? FALSE : TRUE;
	if (m_play)
		SetTimer(1, 15, NULL);
	else
		KillTimer(1);
}

void CCubeView::OnUpdateFilePlay(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_play);
}

BOOL CCubeView::PreCreateWindow(CREATESTRUCT& cs)
{
	// An OpenGL window must be created with the following flags and must not
	// include CS_PARENTDC for the class style. Refer to SetPixelFormat
	// documentation in the "Comments" section for further information.
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	return CView::PreCreateWindow(cs);
}

	RecordA a;

	double elevation_data[1201][1201];

int CCubeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	Init(); // initialize OpenGL

	const char *filename = "092j02_0100_demw.dem";
		FILE *f = fopen(filename, "rb");
	if (!f)
	{
		fprintf(stderr, "Couldn't open file '%s'\n", filename);
		return -1;
	}

	// A .DEM file starts with a type A record.
	ReadRecordTypeA(&a, f);

	// Then there are num_rows * num_cols B records.  Generally num_rows == 1, num_cols == 1201.
	for (int col = 0; col < 1201; ++col)
	{
		RecordB b;
		ReadRecordTypeB(&b, f);

		for (int row = 0; row < 1201; ++row)
		{
			double elevation = b.elev_local_vert_datum + b.elevations[row];
			elevation_data[row][col] = elevation;
		}
	}

	fclose(f);

	return 0;
}

void CCubeView::OnDestroy()
{
	HGLRC   hrc;

	KillTimer(1);

	hrc = ::wglGetCurrentContext();

	::wglMakeCurrent(NULL,  NULL);

	if (hrc)
		::wglDeleteContext(hrc);

	if (m_pDC)
		delete m_pDC;

	CView::OnDestroy();
}

void CCubeView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if(cy > 0)
	{
		glViewport(0, 0, cx, cy);

		if((m_oldRect.right > cx) || (m_oldRect.bottom > cy))
			RedrawWindow();

		m_oldRect.right = cx;
		m_oldRect.bottom = cy;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0f, (GLdouble)cx/cy, 3.0f, 7.0f);
		glMatrixMode(GL_MODELVIEW);
	}
}

void CCubeView::OnTimer(UINT nIDEvent)
{
	DrawScene();

	CView::OnTimer(nIDEvent);

	// Eat spurious WM_TIMER messages
	MSG msg;
	while(::PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

/////////////////////////////////////////////////////////////////////////////
// GL helper functions

void CCubeView::Init()
{
	PIXELFORMATDESCRIPTOR pfd;
	int         n;
	HGLRC       hrc;
	GLfloat     fMaxObjSize, fAspect;
	GLfloat     fNearPlane, fFarPlane;

	m_pDC = new CClientDC(this);

	ASSERT(m_pDC != NULL);

	if (!bSetupPixelFormat())
		return;

	n = ::GetPixelFormat(m_pDC->GetSafeHdc());
	::DescribePixelFormat(m_pDC->GetSafeHdc(), n, sizeof(pfd), &pfd);

	hrc = wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(), hrc);

	GetClientRect(&m_oldRect);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);

	if (m_oldRect.bottom)
		fAspect = (GLfloat)m_oldRect.right/m_oldRect.bottom;
	else    // don't divide by zero, not that we should ever run into that...
		fAspect = 1.0f;
	fNearPlane = 4.5f;
	fFarPlane = 4.0f;
	fMaxObjSize = 3.0f;
	m_fRadius = fNearPlane + fMaxObjSize / 2.0f;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, fAspect, fNearPlane, fFarPlane);
	glMatrixMode(GL_MODELVIEW);
}

BOOL CCubeView::bSetupPixelFormat()
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
		1,                              // version number
		PFD_DRAW_TO_WINDOW |            // support window
		  PFD_SUPPORT_OPENGL |          // support OpenGL
		  PFD_DOUBLEBUFFER,             // double buffered
		PFD_TYPE_RGBA,                  // RGBA type
		24,                             // 24-bit color depth
		0, 0, 0, 0, 0, 0,               // color bits ignored
		0,                              // no alpha buffer
		0,                              // shift bit ignored
		0,                              // no accumulation buffer
		0, 0, 0, 0,                     // accum bits ignored
		32,                             // 32-bit z-buffer
		0,                              // no stencil buffer
		0,                              // no auxiliary buffer
		PFD_MAIN_PLANE,                 // main layer
		0,                              // reserved
		0, 0, 0                         // layer masks ignored
	};
	int pixelformat;

	if ( (pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0 )
	{
		MessageBox("ChoosePixelFormat failed");
		return FALSE;
	}

	if (SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
	{
		MessageBox("SetPixelFormat failed");
		return FALSE;
	}

	return TRUE;
}

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

void CCubeView::DrawScene(void)
{
	static BOOL     bBusy = FALSE;
	static GLfloat  wAngleX = 300.0f;
	static GLfloat  wAngleY = 0.0f;
	static GLfloat  wAngleZ = 144.0f;

	if(bBusy)
		return;
	bBusy = TRUE;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
#if 1
		glTranslatef(0.0f, 0.0f, -m_fRadius);
#endif
#if 1
		glRotatef(wAngleX, 1.0f, 0.0f, 0.0f);
		glRotatef(wAngleY, 0.0f, 1.0f, 0.0f);
		glRotatef(wAngleZ, 0.0f, 0.0f, 1.0f);
#endif
		glScalef(4.0f, 4.0f, 4.0f);
#if 1
		//wAngleX += 1.0f;
		//wAngleY += 1.0f;
		wAngleZ += 1.0f;
#endif


		int col_incr = 1;
		int row_incr = 1;

	double size_x = 0.75*19.2;
	double size_y = 0.75*30.9;

	for (int col = 0; col < 1201-col_incr; col += col_incr)
	{
		for (int row = 0; row < 1201-row_incr; row += row_incr)
		{
			double sw_elevation = elevation_data[row][col];
			double se_elevation = elevation_data[row][col+col_incr];
			double nw_elevation = elevation_data[row+row_incr][col];
			double ne_elevation = elevation_data[row+row_incr][col+col_incr];

			double sw_x = col * size_x;
			double se_x = (col+col_incr) * size_x;
			double nw_x = sw_x;
			double ne_x = se_x;

			double sw_y = row * size_y;
			double nw_y = (row+row_incr) * size_y;
			double ne_y = nw_y;
			double se_y = sw_y;

			double offset_x = 12500;
			double offset_y = 14000;

			sw_x -= offset_x;
			se_x -= offset_x;
			nw_x -= offset_x;
			ne_x -= offset_x;

			sw_y -= offset_y;
			se_y -= offset_y;
			nw_y -= offset_y;
			ne_y -= offset_y;

			double scale = 8000;

			glBegin(GL_TRIANGLE_STRIP);
				COLORREF clr = GetElevationColour(nw_elevation, a.min_elev, a.max_elev);
				float r = GetRValue(clr)/255.0;
				float g = GetGValue(clr)/255.0;
				float b = GetBValue(clr)/255.0;

				//r = row/1201.0;
				glColor3f(r, g, b);
				glVertex3f(sw_x/scale, sw_y/scale, sw_elevation/scale);

				glColor3f(r, g, b);
				glVertex3f(nw_x/scale, nw_y/scale, nw_elevation/scale);

				glColor3f(r, g, b);
				glVertex3f(se_x/scale, se_y/scale, se_elevation/scale);

				glColor3f(r, g, b);
				glVertex3f(ne_x/scale, ne_y/scale, ne_elevation/scale);
			glEnd();
		}
	}
	glPopMatrix();

	glFinish();
	SwapBuffers(wglGetCurrentDC());

	bBusy = FALSE;
}

BOOL CCubeView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}
