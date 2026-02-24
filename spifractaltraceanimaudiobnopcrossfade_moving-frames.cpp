/*
 * Copyright (c) 2010-2018 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// spifractaltrace.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>

#include "FreeImage.h"

#include <direct.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <ctime>

#include <sstream>

#include "sndfile.h"

#if defined _M_X64
#include "opencv2/imgcodecs.hpp"
#endif

using namespace std;

ofstream myofstream;

string global_imagefolder = "C:\\temp";
string global_imageextension = ".jpg";
vector<string> global_imagefilenames;
vector<string> global_imagefilenames_tobedeleted;

string global_outputimagefolder = "C:\\temp(fractal-trace-anim-audio-bnop)";

int global_maxnumberofimagestoload = -1; //-1 for all, 1 for only first image, etc.

int global_maxnumberofframeperseedaudiofile = -1; //-1 for all, or greater than 0. For example, at 30 frames per sec, a value of 300 means 10 sec of generated frames

int global_outputvideoframepersecond = 30;

string global_ffmpegpath = "C:\\video\\ffmpeg";
string global_framefilenameprefix = "frame_";
string global_framefilenameext = global_imageextension;
string global_videofilenameext = ".mp4";

vector<string> global_audiofilenames; //for now, we will use only 1 audiofile
string global_audiofilename = "C:\\temp\\underworld-rez.wav";
int global_maxnumberofaudiofilestoprocess = -1; //-1 for all
int global_mergeaudiowithfinalvideo = 1;

double global_maxwindowwidthtranslationmultiple = 1.0; //1.0 for 1 windowwidth, 0.5 for half a windowwidth (i.e. windowwidth in x is the fractaltrace xmax-xmin)


#define OUTSIDE_TYPE_WRAP	0
#define OUTSIDE_TYPE_BLACK	1
#define OUTSIDE_TYPE_WHITE	2

int global_fractaltrace_outsidetype = 0; //0 for wrap, 1 for black, 2 for white
//int global_fractaltrace_outsidetype = 1; //0 for wrap, 1 for black, 2 for white

#define FRACTALTRACE_TYPE_MANDELBROT		0
#define FRACTALTRACE_TYPE_JULIA				1
#define FRACTALTRACE_TYPE_JULIA_MANDELBROT	2

//int global_fractaltrace_type = 0; //0 for mandelbrot using mandelbrot equation, 1 for julia using julia equation, 2 for mandelbrot using julia equation
int global_fractaltrace_type = 1; //0 for mandelbrot using mandelbrot equation, 1 for julia using julia equation, 2 for mandelbrot using julia equation
//int global_fractaltrace_type = 2; //0 for mandelbrot using mandelbrot equation, 1 for julia using julia equation, 2 for mandelbrot using julia equation

//int global_fractaltrace_depth = 3; //3 or 2 works great
//int global_fractaltrace_depth = 2; //3 or 2 works great
//int global_fractaltrace_depth = 1; //3 or 2 works great
int global_fractaltrace_depth = 3; //3 or 2 works great

double global_fractaltrace_bailout = 10000.0; //between 0.0 and 10000.0
//double global_fractaltrace_jx = 0.5; //between -50.0 and 50.0
//double global_fractaltrace_jy = 0.5; //between -50.0 and 50.0
//double global_fractaltrace_jx = 0.25; //between -50.0 and 50.0
//double global_fractaltrace_jy = 0.25; //between -50.0 and 50.0
//double global_fractaltrace_jx = 0.05; //between -50.0 and 50.0
//double global_fractaltrace_jy = 0.05; //between -50.0 and 50.0
double global_fractaltrace_jx = -0.5; //between -50.0 and 50.0
double global_fractaltrace_jy = 0.75; //between -50.0 and 50.0

/*
double global_fractaltrace_xmin = -1.0;
double global_fractaltrace_xmax = 0.5;
double global_fractaltrace_ymin = -1.0;
double global_fractaltrace_ymax = 1.0;
*/
/*
double global_fractaltrace_xmin = -0.5;
double global_fractaltrace_xmax = 0.5;
double global_fractaltrace_ymin = -0.5;
double global_fractaltrace_ymax = 0.5;
*/
/*
double global_fractaltrace_xmin = -1.0;
double global_fractaltrace_xmax = 0.5;
double global_fractaltrace_ymin = -1.0;
double global_fractaltrace_ymax = -0.5;
*/
/*
double global_fractaltrace_xmin = -1.2;
double global_fractaltrace_xmax = -0.5;
double global_fractaltrace_ymin = -1.0;
double global_fractaltrace_ymax = -0.2;
*/
double global_fractaltrace_xmin = -0.7;
double global_fractaltrace_xmax = 0.5;
double global_fractaltrace_ymin = -0.8;
double global_fractaltrace_ymax = 0.2;

int global_fractaltrace_tagoutputimagefilenames = 1; //0 for don't tag, 1 for tag

#define ANIM_TRANSLATION_TYPE_LEFT	0
#define ANIM_TRANSLATION_TYPE_RIGHT	1
#define ANIM_TRANSLATION_TYPE_UP	2
#define ANIM_TRANSLATION_TYPE_DOWN	3
const char* anim_translation_types[] = { "left", "right", "up", "down" };

string global_aubiotrackpath = "C:\\Users\\do-org\\Downloads\\aubio-0.4.6-win32\\bin\\aubiotrack";
vector<float> global_audiobeattimes_sec;
vector<int> global_audiobeattimes_framenumber;

int global_fasttranslationspeed = 1; //1 for fast, 0 for slow and -1 for adaptative to note change

string global_zoomwindowsfilename = "";

vector<string> global_zoomwindowsvector;

int global_newimageinverseprobability = 100; //between 1 and 100, i.e. 1 for newimage on every note, 100 for new image only once every 100 times (in probability, it controls a random variable)

string global_aubionotespath = "C:\\Users\\do-org\\Downloads\\aubio-0.4.6-win32\\bin\\aubionotes";
int global_aubionotesquantize = 3; //between 1 and 5 or 0 for no note events segmentation (variable called note quantization for the user, i.e. 3 means note change less than 3 midi note value are ignored so note events are clustered together)
vector<float> global_audionotetimes_midinote;
vector<float> global_audionotetimes_sec;
vector<float> global_segaudionotetimes_midinote;
vector<float> global_segaudionotetimes_sec;
vector<int> global_segaudionotetimes_framenumber;

float global_defaultimagechangeperiod_sec = -1.0; //no default image change when smaller than 0
int global_defaultimagechangeperiod_framenumber; //automatically derived from global_defaultimagechangeperiod_sec

string global_aubiopitchpath = "C:\\Users\\do-org\\Downloads\\aubio-0.4.6-win32\\bin\\aubiopitch";
float global_aubiopitchquantize_hz = 10.0; //between 0.0 and 500.0 or 0.0 for no pitch events segmentation (variable called pitch quantization for the user, i.e. 10.0 means pitch variation less than 10.0 hertz are ignored so pitch events are clustered together)
vector<float> global_audiopitchtimes_sec;
vector<float> global_audiopitchtimes_hz;
vector<float> global_segaudiopitchtimes_sec;
vector<float> global_segaudiopitchtimes_hz;
vector<int> global_segaudiopitchtimes_nclustered;
vector<int> global_segaudiopitchtimes_framenumber;

string global_aubioonsetpath = "C:\\Users\\do-org\\Downloads\\aubio-0.4.6-win32\\bin\\aubioonset";
float global_aubioonsetthreshold = 0.3; //between 0.001 and 0.900, defaults to 0.3 (Lower threshold values imply more onsets detected. Increasing this threshold should reduce the number of incorrect detections.) 
float global_aubioonsetminimumonsetinterval_sec = 0.020; //between 0.001 and ?, defaults to 0.020 (Set the minimum inter-onset interval, in seconds, the shortest interval between two consecutive onsets. Defaults to 0.020) 
vector<float> global_audioonsettimes_sec;
vector<float> global_segaudioonsettimes_sec;
vector<int> global_segaudioonsettimes_nclustered;
vector<int> global_segaudioonsettimes_framenumber;

string global_bnop = "beat"; //"beat", "note", "onset" or "pitch"
vector<int> global_segaudiobnoptimes_framenumber;

string global_imagemagickconvertpath = "C:\\Program Files (x86)\\ImageMagick-7.0.8-Q8\\convert"; //used by the 32 bit version only, no need with the 64bit version
int global_maxnumberofframeforcrossfades = -1; //must be greater than 2 for doing the crossfade, otherwise crossfade is completely disabled

float global_audiofileduration_sec; 
int global_audiofileduration_framenumber;
SNDFILE* global_pSNDFILE=NULL;
SF_INFO global_mySF_INFO;

vector<FIBITMAP*> global_imagehandles; 

struct zoomwindow {
	float xmin, xmax, ymin, ymax;
}; 
vector<zoomwindow> global_zoomwindowsfloatsvector;

struct rhythmicstats {
	int beats, notes, onsets, pitches;
	float beatspersec, notespersec, onsetspersec, pitchespersec;
};

vector<rhythmicstats> global_segaudiobnoptimes_rhythmicstats;

float global_avgbeatspersec, global_avgnotespersec, global_avgonsetspersec, global_avgpitchespersec;
float global_maxbeatspersec, global_maxnotespersec, global_maxonsetspersec, global_maxpitchespersec;



void Terminate()
{
	for (int i = 0; i < global_imagehandles.size(); i++)
	{
		FIBITMAP* pDIB = global_imagehandles[i];
		FreeImage_Unload(pDIB);
	}
	if (myofstream.is_open()) myofstream.close();
}

void pixels_get(FIBITMAP* dib,
				int		  x,
				int       y,
				RGBQUAD*  pixel)
{
	int imagewidth = FreeImage_GetWidth(dib);
	int imageheight = FreeImage_GetHeight(dib);
	if(x < 0) x = 0; else if (imagewidth  <= x) x = imagewidth  - 1;
	if(y < 0) y = 0; else if (imageheight <= y) y = imageheight - 1;
	FreeImage_GetPixelColor(dib, x, y, pixel);
}

void pixels_get_biliner(FIBITMAP* dib,
						double  x,
						double  y,
						RGBQUAD* pixel)
{
	RGBQUAD A, B, C, D;
	double a, b, c, d;
	int    x1, y1, x2, y2;
	double dx, dy;
	double alpha=1.0;

	x1 = (int) floor (x);
	x2 = x1 + 1;
	y1 = (int) floor (y);
	y2 = y1 + 1;

	dx = x - (double) x1;
	dy = y - (double) y1;
	a  = (1.0 - dx) * (1.0 - dy);
	b  = dx * (1.0 - dy);
	c  = (1.0 - dx) * dy;
	d  = dx * dy;

	pixels_get (dib, x1, y1, &A);
	pixels_get (dib, x2, y1, &B);
	pixels_get (dib, x1, y2, &C);
	pixels_get (dib, x2, y2, &D);

	/*
	alpha = 1.0001*(a * (gdouble) A.a + b * (gdouble) B.a
					+ c * (gdouble) C.a + d * (gdouble) D.a);
	pixel->a = (guchar) alpha;

	if (pixel->a)
	*/
	if(1)
	{
		/*
		pixel->rgbRed = (unsigned char) ((a * (double) A.rgbRed * A.a
							+ b * (double) B.rgbRed * B.a
							+ c * (double) C.rgbRed * C.a
							+ d * (double) D.rgbRed * D.a) / alpha);
		pixel->rgbGreen = (unsigned char) ((a * (double) A.rgbGreen * A.a
							+ b * (double) B.rgbGreen * B.a
							+ c * (double) C.rgbGreen * C.a
							+ d * (double) D.rgbGreen * D.a) / alpha);
		pixel->rgbBlue = (unsigned char) ((a * (double) A.rgbBlue * A.a
							+ b * (double) B.rgbBlue * B.a
							+ c * (double) C.rgbBlue * C.a
							+ d * (double) D.rgbBlue * D.a) / alpha);
		*/
		pixel->rgbRed = (unsigned char) ((a * (double) A.rgbRed 
							+ b * (double) B.rgbRed 
							+ c * (double) C.rgbRed 
							+ d * (double) D.rgbRed ) );
		pixel->rgbGreen = (unsigned char) ((a * (double) A.rgbGreen 
							+ b * (double) B.rgbGreen 
							+ c * (double) C.rgbGreen 
							+ d * (double) D.rgbGreen ) );
		pixel->rgbBlue = (unsigned char) ((a * (double) A.rgbBlue 
							+ b * (double) B.rgbBlue 
							+ c * (double) C.rgbBlue 
							+ d * (double) D.rgbBlue ) );

	}
}

void pixels_set(FIBITMAP* dib,
				int  x,
				int  y,
				RGBQUAD* pixel)
{
	FreeImage_SetPixelColor(dib, x, y, pixel);
	return;
}
			
void mandelbrot (double  x,
				double  y,
				double *u,
				double *v)
{
	int    iter = 0;
	double xx   = x;
	double yy   = y;
	double x2   = xx * xx;
	double y2   = yy * yy;
	double tmp;

	while (iter < global_fractaltrace_depth)
	{
		tmp = x2 - y2 + x;
		yy  = 2 * xx * yy + y;
		xx  = tmp;
		x2  = xx * xx;
		y2  = yy * yy;
		iter++;
	}
	*u = xx;
	*v = yy;
}

void julia (double  x,
			double  y,
			double  jx,
			double  jy,
			double *u,
			double *v,
			double bailout2)
{
	int    i;
	double xx = x;
	double yy = y;
	
	for (i = 0; i < global_fractaltrace_depth; i++)
	{
		double x2, y2, tmp;

		x2 = xx * xx;
		y2 = yy * yy;
		tmp = x2 - y2 + jx;
		yy  = 2 * xx * yy + jy;
		xx  = tmp;

		if ((x2 + y2) > bailout2)
		break;
	}

	*u = xx;
	*v = yy;
}

void DoFractalTrace(FIBITMAP* input_dib, FIBITMAP* output_dib, double fractaltrace_xmin, double fractaltrace_xmax, double fractaltrace_ymin, double fractaltrace_ymax)
{
	int    x, y;
	RGBQUAD pixel;
	double scale_x, scale_y;
	double cx, cy;
	double px, py;

	int imagewidth = FreeImage_GetWidth(input_dib);
	int imageheight = FreeImage_GetHeight(input_dib);
	int selectionwidth = imagewidth;
	int selectionheight = imageheight;
	int selectionxmin = 0;
	int selectionxmax = selectionxmin+imagewidth;
	int selectionymin = 0;
	int selectionymax = selectionymin+imageheight;

	double bailout2 = global_fractaltrace_bailout * global_fractaltrace_bailout;

	/*
	gimp_progress_init (_("Fractal Trace"));
	*/

	scale_x = (fractaltrace_xmax - fractaltrace_xmin) / (double)selectionwidth;
	scale_y = (fractaltrace_ymax - fractaltrace_ymin) / (double)selectionheight;

	for (y = selectionymin; y < selectionymax; y++)
	{
		cy = selectionymin + (y - selectionymin) * scale_y;
		for (x = selectionxmin; x < selectionxmax; x++)
		{
			cx = fractaltrace_xmin + (x - selectionxmin) * scale_x;
			switch (global_fractaltrace_type)
			{
				case FRACTALTRACE_TYPE_MANDELBROT:
					mandelbrot (cx, cy, &px, &py);
					break;
				case FRACTALTRACE_TYPE_JULIA:
					julia (cx, cy, global_fractaltrace_jx, global_fractaltrace_jy, &px, &py, bailout2);
					break;
				case FRACTALTRACE_TYPE_JULIA_MANDELBROT:
					julia (cx, cy, cx, cy, &px, &py, bailout2);
					break;
				default:
					mandelbrot (cx, cy, &px, &py);
			}
			px = (px - fractaltrace_xmin) / scale_x + selectionxmin;
			py = (py - fractaltrace_ymin) / scale_y + selectionymin;
			if (0 <= px && px < imagewidth && 0 <= py && py < imageheight)
			{
				pixels_get_biliner (input_dib, px, py, &pixel);
			}
			else
			{
				/*
				switch (parameters.outside_type)
				{
				case OUTSIDE_TYPE_WRAP:
					px = fmod (px, image.width);
					py = fmod (py, image.height);
					if( px < 0.0) px += image.width;
					if (py < 0.0) py += image.height;
					pixels_get_biliner (px, py, &pixel);
					break;
				case OUTSIDE_TYPE_TRANSPARENT:
					pixel.r = pixel.g = pixel.b = 0;
					pixel.a = 0;
					break;
				case OUTSIDE_TYPE_BLACK:
					pixel.r = pixel.g = pixel.b = 0;
					pixel.a = 255;
					break;
				case OUTSIDE_TYPE_WHITE:
					pixel.r = pixel.g = pixel.b = 255;
					pixel.a = 255;
					break;
				}
				*/
				switch (global_fractaltrace_outsidetype)
				{
				case OUTSIDE_TYPE_WRAP:
					px = fmod (px, imagewidth);
					py = fmod (py, imageheight);
					if( px < 0.0) px += imagewidth;
					if (py < 0.0) py += imageheight;
					pixels_get_biliner (input_dib, px, py, &pixel);
					break;
					/*
				case OUTSIDE_TYPE_TRANSPARENT:
					pixel.r = pixel.g = pixel.b = 0;
					pixel.a = 0;
					break;
					*/
				case OUTSIDE_TYPE_BLACK:
					pixel.rgbRed = pixel.rgbGreen = pixel.rgbBlue = 0;
					//pixel.a = 255;
					break;
				case OUTSIDE_TYPE_WHITE:
					pixel.rgbRed = pixel.rgbGreen = pixel.rgbBlue = 255;
					//pixel.a = 255;
					break;
				}
			}
			pixels_set (output_dib, x, y, &pixel);
		}

		/*
		if (((y - selection.y1) % (selection.height / 100)) == 0)
		gimp_progress_update ((gdouble) (y-selection.y1) / selection.height);
		*/
	}
	return;
}


void MapImageSpaceOntoFractalSpace(int x, int y, float* p_xx, float* p_yy)
{
	return;
}

void MapFractalSpaceOntoImageSpace(float xx, float yy, int* p_x, int* p_y)
{
	return;
}


std::string space2underscore(std::string text) {
    for(std::string::iterator it = text.begin(); it != text.end(); ++it) {
        if(*it == ' ') {
            *it = '_';
        }
    }
    return text;
}

std::string dot2dash(std::string text) {
    for(std::string::iterator it = text.begin(); it != text.end(); ++it) {
        if(*it == '.') {
            *it = '-';
        }
    }
    return text;
}

std::string ampersand2underscore(std::string text) {
	for (std::string::iterator it = text.begin(); it != text.end(); ++it) {
		if (*it == '&') {
			*it = '_';
		}
	}
	return text;
}

float RandomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

int RandomInt(int lowest, int highest)
{
	int range = (highest - lowest) + 1;
	int random_integer = lowest + int(range*rand() / (RAND_MAX + 1.0));
	return random_integer;
}

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}




PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }


int _tmain(int argc, _TCHAR* argv[])
{
	//0)
	LPSTR *szArgList;
	int nArgs;
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	LPWSTR *szArgListW;
	int nArgsW;
	szArgListW = CommandLineToArgvW(GetCommandLineW(), &nArgsW);
	if( NULL == szArgListW )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}

	if(nArgs>1)
	{
		global_imagefolder = szArgList[1];
	}
	if(nArgs>2)
	{
		global_outputimagefolder = szArgList[2]; //for safety, this program does not create the output folder, it must be created by the user or nothing gets saved
	}
	if(nArgs>3)
	{
		global_imageextension = szArgList[3];
	}
	if(nArgs>4)
	{
		global_maxnumberofimagestoload = atoi(szArgList[4]); //-1 for all images in the global_imagefolder
	}
	if(nArgs>5)
	{
		global_fractaltrace_outsidetype = atoi(szArgList[5]); //0 for wrap
	}
	if(nArgs>6)
	{
		global_fractaltrace_type = atoi(szArgList[6]); //0 for mandelbrot, 1 for julia
	}
	if(nArgs>7)
	{
		global_fractaltrace_depth = atoi(szArgList[7]); //3 or 2 works great
	}
	if(nArgs>8)
	{
		global_fractaltrace_bailout = atof(szArgList[8]); 
	}
	if(nArgs>9)
	{
		global_fractaltrace_jx = atof(szArgList[9]); 
	}
	if(nArgs>10)
	{
		global_fractaltrace_jy = atof(szArgList[10]); 
	}
	if(nArgs>11)
	{
		global_fractaltrace_xmin = atof(szArgList[11]); 
	}
	if(nArgs>12)
	{
		global_fractaltrace_xmax = atof(szArgList[12]); 
	}
	if(nArgs>13)
	{
		global_fractaltrace_ymin = atof(szArgList[13]); 
	}
	if(nArgs>14)
	{
		global_fractaltrace_ymax = atof(szArgList[14]); 
	}
	if(nArgs>15)
	{
		global_audiofilename = szArgList[15]; 
	}
	if(nArgs>16)
	{
		global_maxnumberofframeperseedaudiofile = atoi(szArgList[16]); 
	}
	if(nArgs>17)
	{
		global_maxwindowwidthtranslationmultiple = atof(szArgList[17]); 
	}
	if(nArgs>18)
	{
		global_outputvideoframepersecond = atoi(szArgList[18]); 
	}
	if(nArgs>19)
	{
		global_ffmpegpath = szArgList[19];
	}
	if(nArgs>20)
	{
		global_framefilenameprefix = szArgList[20];
	}
	if(nArgs>21)
	{
		global_videofilenameext = szArgList[21];
	}
	if(nArgs>22)
	{
		global_aubiotrackpath = szArgList[22];
	}
	if(nArgs>23)
	{
		global_mergeaudiowithfinalvideo = atoi(szArgList[23]); 
	}
	if(nArgs>24)
	{
		global_fasttranslationspeed = atoi(szArgList[24]); 
	}
	if(nArgs>25)
	{
		global_zoomwindowsfilename = szArgList[25];
	}
	if(nArgs>26)
	{
		global_newimageinverseprobability = atoi(szArgList[26]); 
	}
	if(global_newimageinverseprobability<1) global_newimageinverseprobability=1;
	if(global_newimageinverseprobability>100) global_newimageinverseprobability=100;
	if(nArgs>27)
	{
		global_aubionotespath = szArgList[27];
	}
	if(nArgs>28)
	{
		global_aubionotesquantize = atoi(szArgList[28]); 
	}
	if(global_aubionotesquantize<0) global_aubionotesquantize=0;
	if(global_aubionotesquantize>5) global_aubionotesquantize=5;
	if(nArgs>29)
	{
		global_defaultimagechangeperiod_sec = atof(szArgList[29]); 
	}
	global_defaultimagechangeperiod_framenumber = floor((global_defaultimagechangeperiod_sec*global_outputvideoframepersecond)+0.5); 	
	if(nArgs>30)
	{
		global_aubiopitchpath = szArgList[30];
	}
	if(nArgs>31)
	{
		global_aubiopitchquantize_hz = atof(szArgList[31]); 
	}
	if(global_aubiopitchquantize_hz<0.0) global_aubiopitchquantize_hz=0.0;
	if(global_aubiopitchquantize_hz>500.0) global_aubiopitchquantize_hz=500.0;

	if(nArgs>32)
	{
		global_aubioonsetpath = szArgList[32];
	}
	if(nArgs>33)
	{
		global_aubioonsetthreshold = atof(szArgList[33]); 
	}
	if(global_aubioonsetthreshold<0.001) global_aubioonsetthreshold=0.001;
	if(global_aubioonsetthreshold>0.900) global_aubioonsetthreshold=0.900;
	if(nArgs>34)
	{
		global_aubioonsetminimumonsetinterval_sec = atof(szArgList[34]); 
	}
	if(global_aubioonsetminimumonsetinterval_sec<0.0) global_aubioonsetminimumonsetinterval_sec=0.020;

	if(nArgs>35)
	{
		global_bnop = szArgList[35];
	}
	if(global_bnop!="beat" && global_bnop!="note" && global_bnop!="onset" && global_bnop!="pitch")
	{
		global_bnop="beat";
	}
	if(nArgs>36)
	{
		global_imagemagickconvertpath = szArgList[36];
	}
	if(nArgs>37)
	{
		global_maxnumberofframeforcrossfades = atoi(szArgList[37]);
	}

	myofstream.open("debug.txt", std::ios::out);
	myofstream << "spifractaltraceanimaudiobnopcrossfade.exe" << endl << endl;


	//0)
	//////////////////////////
	//initialize random number
	//////////////////////////
	srand((unsigned)time(0));

	//////////////////////////////////////////////
	//if don't exist, create dir for output frames 
	//////////////////////////////////////////////
	_mkdir(global_outputimagefolder.c_str());

	/////////////////////////
	//check audio file length
	/////////////////////////
	global_pSNDFILE = sf_open(global_audiofilename.c_str(), SFM_READ, &global_mySF_INFO);
    //assert(global_mySF_INFO.samplerate == 44100);
    //assert(global_mySF_INFO.channels == 2);
	if(global_pSNDFILE==NULL)
	{
		myofstream << "error, can't open audio file" << endl << endl;
		cout << "error, can't open audio file" << endl;
		return 1;
	}
    float file1duration_s;
    file1duration_s = ((float)global_mySF_INFO.frames) / ((float)global_mySF_INFO.samplerate);
    global_audiofileduration_sec = file1duration_s; 
	global_audiofileduration_framenumber = (int) floor(global_audiofileduration_sec*global_outputvideoframepersecond);
	sf_close(global_pSNDFILE);


	//1)
	//////////////////////////
	//populate image filenames
	//////////////////////////
	if (global_imagefolder != "" && global_imageextension != "")
	{
		//1) execute cmd line to get all folder's image filenames
		string quote = "\"";
		string pathfilter;
		string path = global_imagefolder;
		//pathfilter = path + "\\*.bmp";
		pathfilter = path + "\\*" + global_imageextension;
		string systemcommand;
		//systemcommand = "DIR " + quote + pathfilter + quote + "/B /O:N > wsic_filenames.txt"; //wsip tag standing for wav set (library) instrumentset (class) populate (function)
		systemcommand = "DIR " + quote + pathfilter + quote + "/B /S /O:N > spifractaltrace_filenames.txt"; // /S for adding path into "spiwtmvs_filenames.txt"
		system(systemcommand.c_str());
		//2) load in all "spiwtmvs_filenames.txt" file
		//vector<string> global_imagefilenames;
		ifstream ifs("spifractaltrace_filenames.txt");
		string temp;
		while (getline(ifs, temp))
		{
			//txtfilenames.push_back(path + "\\" + temp);
			global_imagefilenames.push_back(temp);
		}
	}
	////////////////////////////////////////////
	//load all images and populate image handles
	////////////////////////////////////////////
	for (int imagefilenameindex = 0; imagefilenameindex < global_imagefilenames.size(); imagefilenameindex++)
	{
		FIBITMAP* pDIB = NULL;
		if (global_imageextension == ".jpg" || global_imageextension == ".JPG" || global_imageextension == ".jpeg" || global_imageextension == ".JPEG")
		{
			pDIB = FreeImage_Load(FIF_JPEG, global_imagefilenames[imagefilenameindex].c_str(), JPEG_DEFAULT);
		}
		else if (global_imageextension == ".bmp" || global_imageextension == ".BMP")
		{
			pDIB = FreeImage_Load(FIF_BMP, global_imagefilenames[imagefilenameindex].c_str(), BMP_DEFAULT);
		}
		else if (global_imageextension == ".png" || global_imageextension == ".PNG")
		{
			pDIB = FreeImage_Load(FIF_PNG, global_imagefilenames[imagefilenameindex].c_str(), PNG_DEFAULT);
		}
		else if (global_imageextension == ".tif" || global_imageextension == ".TIF" || global_imageextension == ".tiff" || global_imageextension == ".TIFF")
		{
			pDIB = FreeImage_Load(FIF_TIFF, global_imagefilenames[imagefilenameindex].c_str(), TIFF_DEFAULT);
		}
		else
		{
			if (myofstream.is_open()) myofstream << "error, unsupported image format" << endl << endl;
			cout << "error, unsupported image format" << endl;
			Terminate();
			return 1;
		}
		if (pDIB == NULL)
		{
			if (myofstream.is_open()) myofstream << "error loading imagefilenameindex " << imagefilenameindex << endl << endl;
			cout << "error loading default image" << endl;
			Terminate();
			return 1;
		}
		//ensure 24 bit
		FIBITMAP* p24bitDIB = FreeImage_ConvertTo24Bits(pDIB);
		if (p24bitDIB == NULL)
		{
			if (myofstream.is_open()) myofstream << "error converting to 24 bit imagefilenameindex " << imagefilenameindex << endl << endl;
			cout << "error converting image to 24 bit" << endl;
			Terminate();
			return 1;
		}
		FreeImage_Unload(pDIB);
		pDIB = NULL;
		//ensure 1920x1080
		int nwidth = FreeImage_GetWidth(p24bitDIB);
		int nheight = FreeImage_GetHeight(p24bitDIB);
		if (nwidth != 1920 || nheight != 1080)
		{
			FIBITMAP* pRescaledDIB = FreeImage_Rescale(p24bitDIB, 1920, 1080, FILTER_BICUBIC);
			if (pRescaledDIB == NULL)
			{
				if (myofstream.is_open()) myofstream << "error, found an image that could not be rescaled at 1920x1080, imagefilenameindex " << imagefilenameindex << endl << endl;
				cout << "error, found an image that could not be rescaled at 1920x1080" << endl;
				Terminate();
				return 1;
			}
			else
			{
				FreeImage_Unload(p24bitDIB);
				p24bitDIB = pRescaledDIB;
			}
		}
		//populate image handles
		global_imagehandles.push_back(p24bitDIB); //FIBITMAP*
	}

	//////////////////////////
	//populate audio filenames (only 1 for now)
	//////////////////////////
	global_audiofilenames.push_back(global_audiofilename);


	/////////////////////////////////
	//populate audio beat time vector
	/////////////////////////////////
	if (global_audiofilename != "")
	{
		//1) execute cmd line to get all audiofile's beat times
		string quote = "\"";
		string systemcommand;
		systemcommand = global_aubiotrackpath + " " + quote + global_audiofilename + quote + " > aubiotrack_beattimes.txt"; 
		system(systemcommand.c_str());
		//2) load in all "aubiotrack_beattimes.txt" file
		ifstream ifs("aubiotrack_beattimes.txt");
		string temp;
		while (getline(ifs, temp))
		{
			global_audiobeattimes_sec.push_back(atof(temp.c_str()));
		}
		//3) quantize to frame numbers
		int prev_framenumber = -1;
		for (vector<float>::iterator iter = global_audiobeattimes_sec.begin() ; iter != global_audiobeattimes_sec.end(); ++iter)
		{  
			//*iter is a float
			int framenumber = floor(((*iter)*global_outputvideoframepersecond)+0.5);
			if(framenumber==0 || framenumber==1 || framenumber==prev_framenumber) continue; 
			global_audiobeattimes_framenumber.push_back(framenumber);
			prev_framenumber = framenumber;
		}
		if(prev_framenumber<global_audiofileduration_framenumber) global_audiobeattimes_framenumber.push_back(global_audiofileduration_framenumber); //add very last frame
	}

	//////////////////////////////
	//populate zoom windows vector
	//////////////////////////////
	if (global_zoomwindowsfilename != "")
	{
		//1) load in all zoom windows
		ifstream ifs(global_zoomwindowsfilename);
		string temp;
		while (getline(ifs, temp))
		{
			global_zoomwindowsvector.push_back(temp);
		}
	}
	//populate zoom windows floats vector
	if (global_zoomwindowsvector.size()>0)
	{
		for (int zoomwindowindex = 0; zoomwindowindex < global_zoomwindowsvector.size(); zoomwindowindex++)
		{
			stringstream mystringstream(global_zoomwindowsvector[zoomwindowindex]);
			string item;
			char delimiter = '\t';
			float tempfloat[4];
			tempfloat[0] = global_fractaltrace_xmin;
			tempfloat[1] = global_fractaltrace_xmax;
			tempfloat[2] = global_fractaltrace_ymin;
			tempfloat[3] = global_fractaltrace_ymax;
			int itemp = 0;
			while (getline(mystringstream, item, delimiter))
			{
				if (itemp>3)
				{
					if (myofstream.is_open()) myofstream << "error, parsing string, found more than 4 substrings" << endl << endl;
					cout << "error, parsing string, found more than 4 substrings" << endl;
					Terminate();
					return 1;
				}
				tempfloat[itemp] = atof(item.c_str());
				itemp++;
			}
			struct zoomwindow myzoomwindow;
			myzoomwindow.xmin = tempfloat[0];
			myzoomwindow.xmax = tempfloat[1];
			myzoomwindow.ymin = tempfloat[2];
			myzoomwindow.ymax = tempfloat[3];
			global_zoomwindowsfloatsvector.push_back(myzoomwindow);
		}
	}


	/////////////////////////////////
	//populate audio note time vector
	/////////////////////////////////
	if (global_audiofilename != "")
	{
		//1) execute cmd line to get all audiofile's note times
		string quote = "\"";
		string systemcommand;
		systemcommand = global_aubionotespath + " " + quote + global_audiofilename + quote + " > aubiotrack_notetimes.txt"; 
		system(systemcommand.c_str());
		//2) load in all "aubiotrack_notetimes.txt" file
		ifstream ifs("aubiotrack_notetimes.txt");
		string temp;
		while (getline(ifs, temp))
		{
			stringstream mystringstream(temp);
			string item;
			char delimiter = '\t';
			float tempfloat[3];
			tempfloat[0] = 0.0;
			tempfloat[1] = 0.0;
			tempfloat[2] = 0.0;
			int itemp = 0;
			while (getline(mystringstream, item, delimiter))
			{
				if(itemp>2)
				{
					myofstream << "error, parsing string, found more than 3 substrings" << endl << endl;
					cout << "error, parsing string, found more than 3 substrings" << endl;
					Terminate(); 
					return 1;
				}
				tempfloat[itemp] = atof(item.c_str());
				itemp++;
			}
			if(itemp==3)
			{
				global_audionotetimes_midinote.push_back(tempfloat[0]);
				global_audionotetimes_sec.push_back(tempfloat[1]);
			}
		}
		//3) segment note times when unsignificant midi note change, that is when less than global_aubionotesquantize
		myofstream << "segmented audio note times, begin" << endl;
		float fprev_midinote = 0.0;
		for (int i=0; i<global_audionotetimes_sec.size(); i++)
		{  
			float fdiff = fabs(global_audionotetimes_midinote[i]-fprev_midinote);
			int idiff = (int) fdiff;
			if(idiff>global_aubionotesquantize || global_aubionotesquantize==0)
			{
				global_segaudionotetimes_sec.push_back(global_audionotetimes_sec[i]);
				myofstream << global_audionotetimes_sec[i] << endl;
				fprev_midinote = global_audionotetimes_midinote[i];
			}
		}
		myofstream << "segmented audio note times, end" << endl;
		//4) quantize to frame numbers
		int prev_framenumber = -1;
		for (vector<float>::iterator iter = global_segaudionotetimes_sec.begin() ; iter != global_segaudionotetimes_sec.end(); ++iter)
		{  
			//*iter is a float
			int framenumber = floor(((*iter)*global_outputvideoframepersecond)+0.5);
			if(framenumber==0 || framenumber==1 || framenumber==prev_framenumber) continue; 
			global_segaudionotetimes_framenumber.push_back(framenumber);
			prev_framenumber = framenumber;
		}
		if(prev_framenumber<global_audiofileduration_framenumber) global_segaudionotetimes_framenumber.push_back(global_audiofileduration_framenumber); //add very last frame
	}


	//////////////////////////////////
	//populate audio pitch time vector
	//////////////////////////////////
	if (global_audiofilename != "")
	{
		//1) execute cmd line to get all audiofile's pitch change times
		string quote = "\"";
		string systemcommand;
		systemcommand = global_aubiopitchpath + " " + quote + global_audiofilename + quote + " > aubiotrack_pitchtimes.txt"; 
		system(systemcommand.c_str());
		//2) load in all "aubiotrack_pitchtimes.txt" file
		ifstream ifs("aubiotrack_pitchtimes.txt");
		string temp;
		while (getline(ifs, temp))
		{
			stringstream mystringstream(temp);
			string item;
			//char delimiter = '\t';
			char delimiter = ' ';
			float tempfloat[2];
			tempfloat[0] = 0.0;
			tempfloat[1] = 0.0;
			int itemp = 0;
			while (getline(mystringstream, item, delimiter))
			{
				if(itemp>1)
				{
					myofstream << "error, parsing string, found more than 2 substrings" << endl << endl;
					cout << "error, parsing string, found more than 2 substrings" << endl;
					Terminate(); 
					return 1;
				}
				tempfloat[itemp] = atof(item.c_str());
				itemp++;
			}
			if(itemp==2)
			{
				global_audiopitchtimes_sec.push_back(tempfloat[0]);
				global_audiopitchtimes_hz.push_back(tempfloat[1]);
			}
		}
		//3) segment pitch times when unsignificant pitch change, that is when less than global_aubiopitchsquantize_hz
		myofstream << "segmented audio pitch times, begin" << endl;
		float fprev_hz = 0.0;
		for (int i=0; i<global_audiopitchtimes_sec.size(); i++)
		{  
			float fdiff = fabs(global_audiopitchtimes_hz[i]-fprev_hz);
			if(fdiff>global_aubiopitchquantize_hz || global_aubiopitchquantize_hz==0.0)
			{
				global_segaudiopitchtimes_sec.push_back(global_audiopitchtimes_sec[i]);
				myofstream << global_audiopitchtimes_sec[i] << endl;
				fprev_hz = global_audiopitchtimes_hz[i];
			}
		}
		myofstream << "segmented audio pitch times, end" << endl;
		//4) quantize to frame numbers AND kick out overlapping events
		int prev_framenumber = -1;
		for (vector<float>::iterator iter = global_segaudiopitchtimes_sec.begin() ; iter != global_segaudiopitchtimes_sec.end(); ++iter)
		{  
			//*iter is a float
			int framenumber = floor(((*iter)*global_outputvideoframepersecond)+0.5);
			if(framenumber==0 || framenumber==1 || framenumber==prev_framenumber) continue; 
			global_segaudiopitchtimes_framenumber.push_back(framenumber);
			prev_framenumber = framenumber;
		}
		if(prev_framenumber<global_audiofileduration_framenumber) global_segaudiopitchtimes_framenumber.push_back(global_audiofileduration_framenumber); //add very last frame
		//5) todo: do a second pass to compute global_segaudiopitchtimes_nclustered
	}

	//////////////////////////////////
	//populate audio onset time vector
	//////////////////////////////////
	if (global_audiofilename != "")
	{
		//0) convert variables into string
		char bufferthreshold[256];
		sprintf(bufferthreshold, "-t %f", global_aubioonsetthreshold);
		char bufferminimuminterval[256];
		sprintf(bufferminimuminterval, "-M %f", global_aubioonsetminimumonsetinterval_sec);
		//1) execute cmd line to get all audiofile's onset times
		string quote = "\"";
		string systemcommand;
		systemcommand = global_aubioonsetpath + " " + bufferthreshold + " " + bufferminimuminterval + " " + quote + global_audiofilename + quote + " > aubiotrack_onsettimes.txt"; 
		system(systemcommand.c_str());
		//2) load in all "aubiotrack_onsettimes.txt" file
		ifstream ifs("aubiotrack_onsettimes.txt");
		string temp;
		while (getline(ifs, temp))
		{
			global_audioonsettimes_sec.push_back(atof(temp.c_str()));
		}		
		//3) segment onset times
		myofstream << "segmented audio onset times, begin" << endl;
		float fprev_sec = 0.0;
		for (int i=0; i<global_audioonsettimes_sec.size(); i++)
		{  
			float fdiff = fabs(global_audioonsettimes_sec[i]-fprev_sec);
			if(fdiff>global_aubioonsetminimumonsetinterval_sec)
			{
				global_segaudioonsettimes_sec.push_back(global_audioonsettimes_sec[i]);
				myofstream << global_audioonsettimes_sec[i] << endl;
				fprev_sec = global_audioonsettimes_sec[i];
			}
		}
		myofstream << "segmented audio onset times, end" << endl;
		//4) quantize to frame numbers AND kick out overlapping events
		int prev_framenumber = -1;
		for (vector<float>::iterator iter = global_segaudioonsettimes_sec.begin() ; iter != global_segaudioonsettimes_sec.end(); ++iter)
		{  
			//*iter is a float
			int framenumber = floor(((*iter)*global_outputvideoframepersecond)+0.5);
			if(framenumber==0 || framenumber==1 || framenumber==prev_framenumber) continue; 
			global_segaudioonsettimes_framenumber.push_back(framenumber);
			prev_framenumber = framenumber;
		}
		if(prev_framenumber<global_audiofileduration_framenumber) global_segaudioonsettimes_framenumber.push_back(global_audiofileduration_framenumber); //add very last frame
		//5) todo: do a second pass to compute global_segaudiopitchtimes_nclustered
	}

	if(global_bnop=="beat")
	{
		global_segaudiobnoptimes_framenumber = global_audiobeattimes_framenumber;
	}
	else if(global_bnop=="note")
	{
		global_segaudiobnoptimes_framenumber = global_segaudionotetimes_framenumber;
	}
	else if(global_bnop=="onset")
	{
		global_segaudiobnoptimes_framenumber = global_segaudioonsettimes_framenumber;
	}
	else if(global_bnop=="pitch")
	{
		global_segaudiobnoptimes_framenumber = global_segaudiopitchtimes_framenumber;
	}


	/////////////////////////////
	//compute rhythmic statistics
	/////////////////////////////
	myofstream << endl << endl << "rhythmic statistics (begin)" << endl;
	myofstream << global_bnop << " selected" << endl;
	myofstream << "frames" << "\t" << "beats" << "\t" << "notes" << "\t" << "onsets" << "\t" << "pitches" << "\t" << "beats/s" << "\t" << "notes/s" << "\t" << "onsets/s" << "\t" << "pitches/s" << endl;
	int numberofbnop = 0;
	float sum1 = 0.0;
	float sum2 = 0.0;
	float sum3 = 0.0;
	float sum4 = 0.0;
	int framenumber_start = 1;
	int framenumber_next;
	//int framenumber_absolute;
	for (vector<int>::iterator iter = global_segaudiobnoptimes_framenumber.begin(); iter != global_segaudiobnoptimes_framenumber.end(); ++iter)
	{
		numberofbnop++;

		//*iter is an int
		framenumber_next = *iter;
		int maxnumberofframeforthisbnop = framenumber_next - framenumber_start;

		//for (int iframe = 0; iframe < maxnumberofframeforthisbnop; iframe++)
		//framenumber_absolute = framenumber_start + iframe;

		rhythmicstats myrhythmicstats = { 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0}; //beats, notes, onsets, pitches; beatspersec, notespersec, onsetspersec, pitchespersec;
		//beats
		for (vector<int>::iterator iter1 = global_audiobeattimes_framenumber.begin(); iter1 != global_audiobeattimes_framenumber.end(); ++iter1)
		{
			//*iter is an int
			int framenumber_this = *iter1;
			if (framenumber_this < framenumber_start) continue;
			else if (framenumber_this >= framenumber_start && framenumber_this < framenumber_next) myrhythmicstats.beats++;
			else break;
		}
		//notes
		for (vector<int>::iterator iter1 = global_segaudionotetimes_framenumber.begin(); iter1 != global_segaudionotetimes_framenumber.end(); ++iter1)
		{
			//*iter is an int
			int framenumber_this = *iter1;
			if (framenumber_this < framenumber_start) continue;
			else if (framenumber_this >= framenumber_start && framenumber_this < framenumber_next) myrhythmicstats.notes++;
			else break;
		}
		//onsets
		for (vector<int>::iterator iter1 = global_segaudioonsettimes_framenumber.begin(); iter1 != global_segaudioonsettimes_framenumber.end(); ++iter1)
		{
			//*iter is an int
			int framenumber_this = *iter1;
			if (framenumber_this < framenumber_start) continue;
			else if (framenumber_this >= framenumber_start && framenumber_this < framenumber_next) myrhythmicstats.onsets++;
			else break;
		}
		//pitches
		for (vector<int>::iterator iter1 = global_segaudiopitchtimes_framenumber.begin(); iter1 != global_segaudiopitchtimes_framenumber.end(); ++iter1)
		{
			//*iter is an int
			int framenumber_this = *iter1;
			if (framenumber_this < framenumber_start) continue;
			else if (framenumber_this >= framenumber_start && framenumber_this < framenumber_next) myrhythmicstats.pitches++;
			else break;
		}

		float intervallength_sec = maxnumberofframeforthisbnop / (float)(global_outputvideoframepersecond);
		myrhythmicstats.beatspersec = myrhythmicstats.beats / intervallength_sec;
		myrhythmicstats.notespersec = myrhythmicstats.notes / intervallength_sec;
		myrhythmicstats.onsetspersec = myrhythmicstats.onsets / intervallength_sec;
		myrhythmicstats.pitchespersec = myrhythmicstats.pitches / intervallength_sec;

		sum1 += myrhythmicstats.beatspersec;
		sum2 += myrhythmicstats.notespersec;
		sum3 += myrhythmicstats.onsetspersec;
		sum4 += myrhythmicstats.pitchespersec;
		if (myrhythmicstats.beatspersec > global_maxbeatspersec) global_maxbeatspersec = myrhythmicstats.beatspersec;
		if (myrhythmicstats.notespersec > global_maxnotespersec) global_maxnotespersec = myrhythmicstats.notespersec;
		if (myrhythmicstats.onsetspersec > global_maxonsetspersec) global_maxonsetspersec = myrhythmicstats.onsetspersec;
		if (myrhythmicstats.pitchespersec > global_maxpitchespersec) global_maxpitchespersec = myrhythmicstats.pitchespersec;

		global_segaudiobnoptimes_rhythmicstats.push_back(myrhythmicstats);
		myofstream << framenumber_start << "," << framenumber_next-1 << "\t\t" << myrhythmicstats.beats << "\t\t" << myrhythmicstats.notes << "\t\t" << myrhythmicstats.onsets << "\t\t" << myrhythmicstats.pitches << "\t" << myrhythmicstats.beatspersec << "\t" << myrhythmicstats.notespersec << "\t" << myrhythmicstats.onsetspersec << "\t" << myrhythmicstats.pitchespersec << endl;

		//framenumber_start = framenumber_absolute + 1;
		framenumber_start = framenumber_next;
	}
	global_avgbeatspersec = sum1 / numberofbnop;
	global_avgnotespersec = sum2 / numberofbnop; 
	global_avgonsetspersec = sum3 / numberofbnop; 
	global_avgpitchespersec = sum4 / numberofbnop;
	myofstream << "global_avgbeatspersec: " << global_avgbeatspersec << ", global_maxbeatspersec: " << global_maxbeatspersec << endl;
	myofstream << "global_avgnotespersec: " << global_avgnotespersec << ", global_maxnotespersec: " << global_maxnotespersec << endl;
	myofstream << "global_avgonsetspersec: " << global_avgonsetspersec << ", global_maxonsetspersec: " << global_maxonsetspersec << endl;
	myofstream << "global_avgpitchespersec: " << global_avgpitchespersec << ", global_maxpitchespersec: " << global_maxpitchespersec << endl;
	myofstream << "rhythmic statistics (end)" << endl << endl;


	myofstream.flush();
	myofstream.close();


	//2)
	/////////////////
	//generate frames
	/////////////////
	string newfilename="";
	string newfolder=global_outputimagefolder;
	string newframefilenameprefix=global_framefilenameprefix; //"frame_";
	string newframefolder="";

	int ii=0;
	for (vector<string>::iterator it = global_audiofilenames.begin() ; it != global_audiofilenames.end(); ++it)
	{    
		ii++;
		//if(ii==2) break;
		if(global_maxnumberofaudiofilestoprocess==0) break;
		if(global_maxnumberofaudiofilestoprocess>0 && ii>global_maxnumberofaudiofilestoprocess) break;

		cout << endl << *it << endl;
		string filename = (*it).substr((*it).find_last_of("\\/")+1);
		cout << filename << endl;

		//ensure filename has no space, replace each space with an underscore
		newfilename = space2underscore(filename);
		//ensure filename has no &, replace each & with an and
		newfilename = ampersand2underscore(newfilename);
		//ensure filename has only one dot, replace each extra dot with a dash
		char path_buffer[_MAX_PATH];  
		char drive[_MAX_DRIVE];  
		char dir[_MAX_DIR];  
		char fname[_MAX_FNAME];  
		char ext[_MAX_EXT];  
		strcpy_s(path_buffer, _MAX_PATH, newfilename.c_str());
		_splitpath(path_buffer, NULL, NULL, fname, ext); 
		if(global_fractaltrace_tagoutputimagefilenames==0)
		{
			newframefolder = dot2dash(fname);
			newfilename = newframefolder + global_videofilenameext;
		}
		//tag outputfilename
		else if(global_fractaltrace_tagoutputimagefilenames==1)
		{
			newfilename = dot2dash(fname);

			char bufferdepth[1024];
			sprintf(bufferdepth, "%d", global_fractaltrace_depth);
			char bufferxmin[1024];
			char bufferxmax[1024];
			char bufferymin[1024];
			char bufferymax[1024];
			char bufferjx[1024];
			char bufferjy[1024];
			sprintf(bufferxmin, "%.4f", global_fractaltrace_xmin);
			sprintf(bufferxmax, "%.4f", global_fractaltrace_xmax);
			sprintf(bufferymin, "%.4f", global_fractaltrace_ymin);
			sprintf(bufferymax, "%.4f", global_fractaltrace_ymax);
			sprintf(bufferjx, "%.4f", global_fractaltrace_jx);
			sprintf(bufferjy, "%.4f", global_fractaltrace_jy);

			newfilename+="(";
			newfilename+="depth";
			newfilename+=bufferdepth;
			if (global_zoomwindowsvector.size() == 0)
			{
				newfilename += "xmin";
				newfilename += bufferxmin;
				newfilename += "xmax";
				newfilename += bufferxmax;
				newfilename += "ymin";
				newfilename += bufferymin;
				newfilename += "ymax";
				newfilename += bufferymax;
			}
			else if (global_zoomwindowsvector.size() > 0)
			{
				newfilename += "xycoor-in-";
				newfilename += global_zoomwindowsfilename;
				newfilename += "-";
			}
			if(global_fractaltrace_type==1)
			{
				newfilename+="jx";
				newfilename+=bufferjx;
				newfilename+="jy";
				newfilename+=bufferjy;
			}
			newfilename+=")";

			newfilename += "(";
			if(global_fasttranslationspeed==1) newfilename += "fast";
			else if (global_fasttranslationspeed == 0) newfilename += "slow";
			else if (global_fasttranslationspeed == -1) newfilename += "adaptative";
			newfilename += ")";


			char bufferprob[1024];
			sprintf(bufferprob, "%d", global_newimageinverseprobability);
			newfilename += "(";
			newfilename += "ip";
			newfilename += bufferprob;
			newfilename += ")";

			char buffernq[1024];
			sprintf(buffernq, "%d", global_aubionotesquantize);
			newfilename += "(";
			newfilename += "nq";
			newfilename += buffernq;
			newfilename += ")";

			char buffericp[1024];
			sprintf(buffericp, "%.4f", global_defaultimagechangeperiod_sec);
			newfilename += "(";
			newfilename += "icp";
			newfilename += buffericp;
			newfilename += ")";

			char bufferpq[1024];
			sprintf(bufferpq, "%.4f", global_aubiopitchquantize_hz);
			newfilename += "(";
			newfilename += "pq";
			newfilename += bufferpq;
			newfilename += ")";

			char bufferot[1024];
			sprintf(bufferot, "%.4f", global_aubioonsetthreshold);
			newfilename += "(";
			newfilename += "ot";
			newfilename += bufferot;
			newfilename += ")";

			char bufferoi[1024];
			sprintf(bufferoi, "%.4f", global_aubioonsetminimumonsetinterval_sec);
			newfilename += "(";
			newfilename += "oi";
			newfilename += bufferoi;
			newfilename += ")";

			newfilename += "(";
			newfilename += global_bnop;
			newfilename += ")";

			if (global_maxnumberofframeforcrossfades > 2)
			{
				char buffercf[1024];
				sprintf(buffercf, "%d", global_maxnumberofframeforcrossfades);
				newfilename += "(";
				newfilename += "cf";
				newfilename += buffercf;
				newfilename += ")";
			}

			newfilename = dot2dash(newfilename);
			newframefolder = newfilename;
			newfilename+= global_videofilenameext;
		}
		else
		{
			newframefolder = dot2dash(fname);
			newfilename = newframefolder + global_videofilenameext;
		}
		string finalnewframefolder = global_outputimagefolder + "\\" + newframefolder;// + "(" + anim_translation_types[itranslation] + ")";
		_mkdir(finalnewframefolder.c_str());


		/////////////////////////////
		//random pick a default image
		/////////////////////////////
		int imagehandleindex = RandomInt(0, global_imagehandles.size() - 1);
		FIBITMAP* p24bitDIBdefault = global_imagehandles[imagehandleindex];

		//////////////////////////////
		//do the fractal trace mapping
		//////////////////////////////
		int index_bnopinterval = -1;
		int framenumber_start = 1;
		int framenumber_next;
		int framenumber_absolute;
		//for (vector<int>::iterator iter = global_audiobeattimes_framenumber.begin() ; iter != global_audiobeattimes_framenumber.end(); ++iter)
		//for (vector<int>::iterator iter = global_segaudionotetimes_framenumber.begin() ; iter != global_segaudionotetimes_framenumber.end(); ++iter)
		//for (vector<int>::iterator iter = global_segaudiopitchtimes_framenumber.begin() ; iter != global_segaudiopitchtimes_framenumber.end(); ++iter)
		//for (vector<int>::iterator iter = global_segaudioonsettimes_framenumber.begin() ; iter != global_segaudioonsettimes_framenumber.end(); ++iter)
		for (vector<int>::iterator iter = global_segaudiobnoptimes_framenumber.begin() ; iter != global_segaudiobnoptimes_framenumber.end(); ++iter)
		{  
			index_bnopinterval++;

			//*iter is a float
			framenumber_next = *iter;
			int maxnumberofframeforthisbnop = framenumber_next - framenumber_start;

			/////////////////////////
			//random pick a new image
			/////////////////////////
			int imagehandleindex = RandomInt(0, global_imagehandles.size()-1);
			FIBITMAP* p24bitDIB = global_imagehandles[imagehandleindex];

			////////////////////////////////
			//random pick a translation type
			////////////////////////////////
			int itranslation = RandomInt(0,3); 

			///////////////////////////
			//random pick a zoom window
			///////////////////////////
			if(global_zoomwindowsvector.size()>0)
			{
				int zoomwindowindex = RandomInt(0, global_zoomwindowsfloatsvector.size()-1);
				struct zoomwindow myzoomwindow = global_zoomwindowsfloatsvector[zoomwindowindex];
				global_fractaltrace_xmin = myzoomwindow.xmin;
				global_fractaltrace_xmax = myzoomwindow.xmax;
				global_fractaltrace_ymin = myzoomwindow.ymin;
				global_fractaltrace_ymax = myzoomwindow.ymax;
			}

			////////////////////////////////////////////////////////
			//randomly decide if will use a new image or the default
			////////////////////////////////////////////////////////
			int iprobability = RandomInt(1,global_newimageinverseprobability);


			for(int iframe=0; iframe<maxnumberofframeforthisbnop; iframe++)
			{
				framenumber_absolute = framenumber_start + iframe;

				///////////////////////////////////////////////////////////////////////////
				//if forced to change default image every x sec AND time has arrived, do it
				///////////////////////////////////////////////////////////////////////////
				if(global_defaultimagechangeperiod_sec>0.0 && global_defaultimagechangeperiod_framenumber>0)
				{
					if( (framenumber_absolute % global_defaultimagechangeperiod_framenumber)==0 )
					{
						int imagehandleindex = RandomInt(0, global_imagehandles.size() - 1);
						p24bitDIBdefault = global_imagehandles[imagehandleindex];
					}
				}

				char buf[256];
				sprintf(buf, "%06d", framenumber_absolute);
				//filename += buf;
				string finalnewfilename = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
				double frame_fractaltrace_xmin = global_fractaltrace_xmin;
				double frame_fractaltrace_xmax = global_fractaltrace_xmax;
				double frame_fractaltrace_ymin = global_fractaltrace_ymin;
				double frame_fractaltrace_ymax = global_fractaltrace_ymax;
				double fdivider = 300.0; //translation should be somewhat slow
				if(global_fasttranslationspeed==1)
				{
					fdivider = maxnumberofframeforthisbnop; //translation should be somewhat fast
					if(fdivider<5.0) fdivider = 5.0;
				}
				else if(global_fasttranslationspeed==-1 && global_avgnotespersec!=0.0)
				{
					//fdivider = (1 - (global_segaudiobnoptimes_rhythmicstats[index_bnopinterval].notespersec/global_maxnotespersec))*600.0;
					//fdivider = ((global_maxnotespersec - global_segaudiobnoptimes_rhythmicstats[index_bnopinterval].notespersec) / global_avgnotespersec)*200.0; 
					//translation should be somewhat adaptative to note change
					float fnotespersec = global_segaudiobnoptimes_rhythmicstats[index_bnopinterval].notespersec;
					if (fnotespersec < 0.5) fdivider = 1200.0;
					else if (fnotespersec < 1.0) fdivider = 800.0;
					else if (fnotespersec < 1.5) fdivider = 600.0;
					else if (fnotespersec < 2.0) fdivider = 400.0;
					else if (fnotespersec < 3.0) fdivider = 300.0;
					else if (fnotespersec < 4.0) fdivider = 200.0;
					else if (fnotespersec < 5.0) fdivider = 100.0;
					else if (fnotespersec < 6.0) fdivider = 50.0;
					else fdivider = 25.0;
					if (fdivider > 1500.0) fdivider = 1500.0;
					if (fdivider < 5.0) fdivider = 5.0;
				}
				double xstep = (global_fractaltrace_xmax - global_fractaltrace_xmin)*global_maxwindowwidthtranslationmultiple / (double)fdivider;
				double ystep = (global_fractaltrace_ymax - global_fractaltrace_ymin)*global_maxwindowwidthtranslationmultiple / (double)fdivider;
				if(iframe==0) cout << "fdivider=" << fdivider << endl;

				switch(itranslation) 
				{
					case ANIM_TRANSLATION_TYPE_LEFT : 
								frame_fractaltrace_xmin = global_fractaltrace_xmin-iframe*xstep;
								frame_fractaltrace_xmax = global_fractaltrace_xmax-iframe*xstep;
								break;       
					case ANIM_TRANSLATION_TYPE_RIGHT : 
								frame_fractaltrace_xmin = global_fractaltrace_xmin+iframe*xstep;
								frame_fractaltrace_xmax = global_fractaltrace_xmax+iframe*xstep;
								break;
					case ANIM_TRANSLATION_TYPE_UP : 
								frame_fractaltrace_ymin = global_fractaltrace_ymin-iframe*ystep;
								frame_fractaltrace_ymax = global_fractaltrace_ymax-iframe*ystep;
								break;       
					case ANIM_TRANSLATION_TYPE_DOWN : 
								frame_fractaltrace_ymin = global_fractaltrace_ymin+iframe*ystep;
								frame_fractaltrace_ymax = global_fractaltrace_ymax+iframe*ystep;
								break;
				}
				//////////////////////////////////////////
				//either pick the default or the new image
				//////////////////////////////////////////
				FIBITMAP* pNew24bitDIB;
				if(iprobability==1)
				{
					//new image
					pNew24bitDIB = FreeImage_Clone(p24bitDIB);
				}
				else
				{
					//default image
					pNew24bitDIB = FreeImage_Clone(p24bitDIBdefault);
				}
				if (pNew24bitDIB) 
				{
					// clone succeeded!

					if (1) //1 normaly, 0 when debuging
					{
						if (iprobability == 1)
						{
							//new image
							DoFractalTrace(p24bitDIB, pNew24bitDIB, frame_fractaltrace_xmin, frame_fractaltrace_xmax, frame_fractaltrace_ymin, frame_fractaltrace_ymax);
						}
						else
						{
							//default image
							DoFractalTrace(p24bitDIBdefault, pNew24bitDIB, frame_fractaltrace_xmin, frame_fractaltrace_xmax, frame_fractaltrace_ymin, frame_fractaltrace_ymax);
						}

						//save final image buffer
						cout << finalnewfilename << " (out of " << global_audiofileduration_framenumber << ")" << endl;
						FreeImage_Save(FIF_JPEG, pNew24bitDIB, finalnewfilename.c_str());
					}

					/*
					//mark filename for delete
					if(iframe!=0 && iframe!=middleframeindex && iframe!=endframeindex)
					{
						global_imagefilenames_tobedeleted.push_back(finalnewfilename);
					}
					*/
					//FreeImage_Unload(p24bitDIB);
					FreeImage_Unload(pNew24bitDIB);
					pNew24bitDIB=NULL;
				}
				else
				{
					if(myofstream.is_open()) myofstream << "error, cloning 24bitDIB, may be low memory" << endl << endl;
					cout << "error, cloning 24bitDIB, may be low memory" << endl;
					Terminate(); 
					return 1;
				}
			} 
			//FreeImage_Unload(p24bitDIB);
			//p24bitDIB=NULL;
			framenumber_start = framenumber_absolute+1;
		}
		//FreeImage_Unload(p24bitDIBdefault);
		//p24bitDIBdefault=NULL;


		///////////////////
		//do the crossfades
		///////////////////
		if(global_maxnumberofframeforcrossfades>2)
		{
			framenumber_start = 1;
			framenumber_next;
			framenumber_absolute;
			//for (vector<int>::iterator iter = global_segaudiobnoptimes_framenumber.begin() ; iter != global_segaudiobnoptimes_framenumber.end(); ++iter)
			for (vector<int>::iterator iter = global_segaudiobnoptimes_framenumber.begin() ; iter < global_segaudiobnoptimes_framenumber.end(); ++iter)
			{  
				//*iter is an int
				framenumber_next = *iter;
				int maxnumberofframeforthisbnop = framenumber_next - framenumber_start;
				int maxnumberofframeforthefollowingbnop = *(iter+1) - framenumber_next;
				int numberofframeforthiscrossfade = min(global_maxnumberofframeforcrossfades, maxnumberofframeforthisbnop/2);
				if(numberofframeforthiscrossfade>2 && numberofframeforthiscrossfade<maxnumberofframeforthefollowingbnop)
				{
					//crossfade the two series of frames
					int framenumber_boundary1 = framenumber_next - numberofframeforthiscrossfade - 1;
					int framenumber_boundary2 = framenumber_next;
					float fpercent = 100.0f/(numberofframeforthiscrossfade+1.0f); 
					for(int index=0; index<numberofframeforthiscrossfade; index++)
					{
						//identify pair of frames to alpha blend
						int framenumber_first = framenumber_boundary1 + (index+1); //ascending serie
						int framenumber_second = framenumber_boundary2 + (numberofframeforthiscrossfade+1) - (index+1); //descending serie

						float fpercent_first = fpercent*(index+1);
						float fpercent_second = 100.0f-fpercent_first;

						char buf[256];
						sprintf(buf, "%06d", framenumber_first);
						string finalnewfilename_first = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
						sprintf(buf, "%06d", framenumber_second);
						string finalnewfilename_second = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;

#if defined _M_IX86
						char buffercompose[256];
						sprintf(buffercompose, "%f,%f", fpercent_first, fpercent_second);
						string systemcommand;
						string quote = "\"";
						//systemcommand = quote + global_imagemagickconvertpath + quote + " " + quote + finalnewfilename_first + quote + " " + quote + finalnewfilename_second + quote + " -compose blend -define compose:args=" + buffercompose + " -composite " + global_outputimagefolder + "\\" + global_framefilenameprefix + "%06d" + global_framefilenameext;
						systemcommand = quote + global_imagemagickconvertpath + quote + " " + quote + finalnewfilename_first + quote + " " + quote + finalnewfilename_second + quote + " -compose blend -define compose:args=" + buffercompose + " -composite " + quote + finalnewfilename_first + quote;
						systemcommand = quote + systemcommand + quote; //wrap systemcommand with quote again
						cout << endl << systemcommand << endl << endl;
						if (myofstream.is_open()) myofstream << endl << systemcommand << endl << endl;
						system(systemcommand.c_str());
#elif defined _M_X64
						cv::Mat src1 = cv::imread(finalnewfilename_first.c_str());
						cv::Mat src2 = cv::imread(finalnewfilename_second.c_str());
						//if (src1.empty()) { cout << "Error loading src1, " << finalnewfilename_first << endl; Terminate();  return 1; }
						//if (src2.empty()) { cout << "Error loading src2, " << finalnewfilename_second << endl; Terminate();  return 1; }
						if (!src1.empty() && !src2.empty())
						{
							float falpha_first = fpercent_first / 100.0f;
							float falpha_second = fpercent_second / 100.0f;
							cv::Mat dst;
							cv::addWeighted(src1, falpha_first, src2, falpha_second, 0.0, dst);
							cv::imwrite(finalnewfilename_first.c_str(), dst);
							cout << finalnewfilename_first << endl;
						}
#endif
					}

					/*
					//rename crossfaded frames
					for(int index=0; index<numberofframeforthiscrossfade; index++)
					{
						//int framenumber_first = framenumber_next - numberofframeforthiscrossfade;
						//int framenumber_last = framenumber_next - 1;
						framenumber_absolute = framenumber_next - numberofframeforthiscrossfade + index;
						//src filename
						sprintf(buf, "%06d", index+1);
						string tempnewfilename_crossfaded = global_outputimagefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
						//dest filename
						sprintf(buf, "%06d", framenumber_absolute);
						string finalnewfilename_crossfaded = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
						//del previously computed frame
						DeleteFileA(finalnewfilename_crossfaded.c_str());
						//rename crossfaded frame
						rename(tempnewfilename_crossfaded.c_str(), finalnewfilename_crossfaded.c_str());
					}
					*/
				}
				//framenumber_start = framenumber_absolute+1;
				framenumber_start = framenumber_next;
			}
		}
		//////////////////////////////////////////////////////
		//now, wrap up all these frames into a .mp4 video file
		//////////////////////////////////////////////////////
		char bufferfps[64];
		int integerfps = (int)global_outputvideoframepersecond;
		if (integerfps<1) integerfps = 1;
		if (integerfps>60) integerfps = 60;
		sprintf(bufferfps, "%d", integerfps);
		char bufferscale[64];
		sprintf(bufferscale, "%dx%d", 1920, 1080);

		string systemcommand;
		string quote = "\"";
		if(global_mergeaudiowithfinalvideo)
		{
			string finalvideofilename = finalnewframefolder + "(with-audio)" + global_videofilenameext;

			systemcommand = global_ffmpegpath + " " + "-i " + quote + global_audiofilename + quote + " -r " + bufferfps + " -s " + bufferscale + " -start_number 1 -i " + finalnewframefolder + "\\" + global_framefilenameprefix + "%6d" + global_framefilenameext \
				+ " -vcodec libx264 -crf 25 -pix_fmt yuv420p " + finalvideofilename;
		}
		else
		{
			string finalvideofilename = finalnewframefolder + global_videofilenameext;

			systemcommand = global_ffmpegpath + " -r " + bufferfps + " -s " + bufferscale + " -start_number 1 -i " + finalnewframefolder + "\\" + global_framefilenameprefix + "%6d" + global_framefilenameext \
				+ " -vcodec libx264 -crf 25 -pix_fmt yuv420p " + finalvideofilename;
		}

		/*
		//debug begin
		FILE* pFILE1 = fopen("commands.txt", "w");
		if (pFILE1)
		{
			fprintf(pFILE1, "%s\n", systemcommand.c_str());
			fclose(pFILE1);
		}
		//debug end
		*/
		systemcommand = quote + systemcommand + quote; //wrap systemcommand with quote again
		cout << endl;
		cout << systemcommand << endl;
		cout << endl;
		system(systemcommand.c_str());
		
		/*
		///////////////////////////////////
		//finaly, delete most of the frames
		///////////////////////////////////
		for (vector<string>::iterator it2 = global_imagefilenames_tobedeleted.begin() ; it2 != global_imagefilenames_tobedeleted.end(); ++it2)
		{    
			DeleteFileA((*it2).c_str());
		}
		global_imagefilenames_tobedeleted.empty();
		*/
		//if(myofstream.is_open()) myofstream.close();
		//Terminate();
	}

	Terminate();
	return 0;
}

