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
//2021jan23, spi, begin
#include <map>
//2021jan23, spi, end

#include <ctime>

#include <sstream>

#include "sndfile.h"

#if defined _M_X64
#include "opencv2/imgcodecs.hpp"
#endif

#include <errno.h>

#include <random>

using namespace std;

ofstream myofstream;

//string global_imagefolder = "C:\\temp";
string global_imagefolder = "D:\\temp";
string global_imageextension = ".jpg";
vector<string> global_imagefilenames;
vector<string> global_imagefilenames_tobedeleted;

//string global_outputimagefolder = "C:\\temp(fractal-trace-anim-audio-bnop)";
string global_outputimagefolder = "D:\\temp(fractal-trace-anim-audio-bnop)";

int global_maxnumberofimagestoload = -1; //-1 for all, 1 for only 1 randomly picked image, etc.

int global_maxnumberofframeperseedaudiofile = -1; //-1 for all, or greater than 0. For example, at 30 frames per sec, a value of 300 means 10 sec of generated frames

int global_outputvideoframepersecond = 30;

//string global_ffmpegpath = "C:\\video\\ffmpeg";
string global_ffmpegpath = "D:\\spibin\\ffmpeg\\ffmpeg";
string global_framefilenameprefix = "frame_";
string global_framefilenameext = global_imageextension;
string global_videofilenameext = ".mp4";

vector<string> global_audiofilenames; //for now, we will use only 1 audiofile
//string global_audiofilename = "C:\\temp\\underworld-rez.wav";
string global_audiofilename = "D:\\temp\\underworld-rez.wav";
int global_maxnumberofaudiofilestoprocess = -1; //-1 for all
int global_mergeaudiowithfinalvideo = 1;

double global_maxwindowwidthtranslationmultiple = 1.0; //1.0 for 1 windowwidth, 0.5 for half a windowwidth (i.e. windowwidth in x is the fractaltrace xmax-xmin)


#define OUTSIDE_TYPE_WRAP						0
#define OUTSIDE_TYPE_BLACK						1
#define OUTSIDE_TYPE_WHITE						2
//2021jan23, spi, begin
#define OUTSIDE_TYPE_WRAP_WITHVARIABILITYMATRIX	3
//to allocate a 9x9 variability matrix, define VARIABILITYMATRIXSIZE to be 9, fractaltrace algorithm will use 81 input frames
//VARIABILITYMATRIXSIZE must be odd and minimum 3, i.e. 9 for a 9x9 matrix to define a context space, around the center image, of 4 extra images in each direction
#define VARIABILITYMATRIXSIZE		9
//2021jan23, spi, end

int global_fractaltrace_outsidetype = 0; //0 for wrap, 1 for black, 2 for white, 3 for wrap with variability
//int global_fractaltrace_outsidetype = 1; //0 for wrap, 1 for black, 2 for white, 3 for wrap with variability

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

//int global_fractaltrace_tagoutputimagefilenames = 1; //0 for don't tag, 1 for tag
int global_fractaltrace_tagoutputimagefilenames = 0; //0 for don't tag, 1 for tag

#define ANIM_TRANSLATION_TYPE_LEFT	0
#define ANIM_TRANSLATION_TYPE_RIGHT	1
#define ANIM_TRANSLATION_TYPE_UP	2
#define ANIM_TRANSLATION_TYPE_DOWN	3
const char* anim_translation_types[] = { "left", "right", "up", "down" };

//string global_aubiotrackpath = "C:\\Users\\do-org\\Downloads\\aubio-0.4.6-win32\\bin\\aubiotrack";
string global_aubiotrackpath = "D:\\spibin\\aubio\\aubio-0.4.6-win32\\bin\\aubiotrack";
vector<float> global_audiobeattimes_sec;
//2021july21, spi, begin
vector<int> global_audiobeattimes_flag;
//2021july21, spi, end
vector<int> global_audiobeattimes_framenumber;

int global_fasttranslationspeed = 1; //1 for fast, 0 for slow and -1 for adaptative to note change

string global_zoomwindowsfilename = "";

vector<string> global_zoomwindowsvector;

int global_newimageinverseprobability = 100; //between 1 and 100, i.e. 1 for newimage on every note, 100 for new image only once every 100 times (in probability, it controls a random variable)

//string global_aubionotespath = "C:\\Users\\do-org\\Downloads\\aubio-0.4.6-win32\\bin\\aubionotes";
string global_aubionotespath = "D:\\spibin\\aubio\\aubio-0.4.6-win32\\bin\\aubionotes";
int global_aubionotesquantize = 3; //between 1 and 5 or 0 for no note events segmentation (variable called note quantization for the user, i.e. 3 means note change less than 3 midi note value are ignored so note events are clustered together)
vector<float> global_audionotetimes_midinote;
vector<float> global_audionotetimes_sec;
vector<float> global_segaudionotetimes_midinote;
vector<float> global_segaudionotetimes_sec;
vector<int> global_segaudionotetimes_framenumber;

float global_defaultimagechangeperiod_sec = -1.0; //no default image change when smaller than 0
int global_defaultimagechangeperiod_framenumber; //automatically derived from global_defaultimagechangeperiod_sec

//string global_aubiopitchpath = "C:\\Users\\do-org\\Downloads\\aubio-0.4.6-win32\\bin\\aubiopitch";
string global_aubiopitchpath = "D:\\spibin\\aubio\\aubio-0.4.6-win32\\bin\\aubiopitch";
float global_aubiopitchquantize_hz = 10.0; //between 0.0 and 500.0 or 0.0 for no pitch events segmentation (variable called pitch quantization for the user, i.e. 10.0 means pitch variation less than 10.0 hertz are ignored so pitch events are clustered together)
vector<float> global_audiopitchtimes_sec;
vector<float> global_audiopitchtimes_hz;
vector<float> global_segaudiopitchtimes_sec;
vector<float> global_segaudiopitchtimes_hz;
vector<int> global_segaudiopitchtimes_nclustered;
vector<int> global_segaudiopitchtimes_framenumber;

//string global_aubioonsetpath = "C:\\Users\\do-org\\Downloads\\aubio-0.4.6-win32\\bin\\aubioonset";
string global_aubioonsetpath = "D:\\spibin\\aubio\\aubio-0.4.6-win32\\bin\\aubioonset";
float global_aubioonsetthreshold = 0.3; //between 0.001 and 0.900, defaults to 0.3 (Lower threshold values imply more onsets detected. Increasing this threshold should reduce the number of incorrect detections.) 
float global_aubioonsetminimumonsetinterval_sec = 0.020; //between 0.001 and ?, defaults to 0.020 (Set the minimum inter-onset interval, in seconds, the shortest interval between two consecutive onsets. Defaults to 0.020) 
vector<float> global_audioonsettimes_sec;
vector<float> global_segaudioonsettimes_sec;
vector<int> global_segaudioonsettimes_nclustered;
vector<int> global_segaudioonsettimes_framenumber;

string global_bnop = "beat"; //"beat", "note", "onset" or "pitch"
vector<int> global_segaudiobnoptimes_framenumber;

//2021july21, spi, begin
int global_minnumberofbeats_beforechange = -1;
int global_maxnumberofbeats_beforechange = -1;
string global_modestring ="";
//2021july21, spi, end

//string global_imagemagickconvertpath = "C:\\Program Files (x86)\\ImageMagick-7.0.8-Q8\\convert"; //used by the 32 bit version only, no need with the 64bit version
string global_imagemagickconvertpath = ""; //used by the 32 bit version only, no need with the 64bit version
int global_maxnumberofframeforcrossfades = -1; //must be greater than 2 for doing the crossfade, otherwise crossfade is completely disabled

float global_audiofileduration_sec; 
int global_audiofileduration_framenumber;
SNDFILE* global_pSNDFILE=NULL;
SF_INFO global_mySF_INFO;

vector<FIBITMAP*> global_imagehandles; 
//2021jan23, spi, begin
//2021july21, spi, begin
int global_debug = 0;
//1 when input images comes from a video sequence, 0 when input images are all independent
//#define INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES					0
int global_INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES = 0;
//1 for wide screen black strips removal (by cropping), 0 for not doing it
//#define INPUTIMAGES_ARE_WIDESCREENFRAMES					0
int global_INPUTIMAGES_ARE_WIDESCREENFRAMES = 0;
//1 for scalling down input images to the same size (the size of the smallest image), 0 for not doing it
//#define INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE		1
//#define INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE		0
int global_INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE = 0;
int global_OUTPUTIMAGES_ARE_TOBESCALEDTOFAAPRODUCTTEMPLATESIZE = 0;
//greater than 0 for scalling down input images to the same size (specifying smallest image width), -1 for not specifying it
#define INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_WIDTH		-1
//#define INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_WIDTH		1920
//greater than 0 for scalling down input images to the same size (specifying smallest image height), -1 for not specifying it
#define INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_HEIGHT	-1
//#define INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_HEIGHT	1080
//for future implementation
#define INPUTIMAGES_ARE_TOOMANYTOLOADALLATONCEINMEMORY		0
//
//mode OUTSIDE_TYPE_WRAP_WITHVARIABILITYMATRIX implementation requires these variables, instead of wraping around a unique input image, 
//in this mode, the fractaltrace algorithm will wrap around n input images (where n equals global_vmsize squared)
map<FIBITMAP*, int> global_imagehandlesmap;
//VARIABILITYMATRIXSIZE must be odd and minimum 3, i.e. 9 for a 9x9 matrix to define a context space, around the center image, of 4 extra images in each direction
int global_vm[VARIABILITYMATRIXSIZE][VARIABILITYMATRIXSIZE]; //variability matrix to be initialized with imagehandles vector index offsets
//2021jan23, spi, end
//2021july21, spi, end

//2021sept17, spi, begin
int global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES = 0;
int global_INPUTIMAGES_ARE_TOBEUSEDFROMSTART = 0;
//2021sept17, spi, end

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

//spi, begin, generalized version
int global_nwidth_2k = 1920;
int global_nheight_2k = 1080;
int global_nwidth_4k = 3840;
int global_nheight_4k = 2160;
int global_nwidth_12k = 12000;
int global_nheight_12k = 12000;
//2021august18, spi, begin
//FAA product template generalization
//2021august18_fineartamerica_product-templates.txt
int global_faa_product_template_nwidth[] = {12000, 3543, 560, 3400, 2700, 8640, 7875, 1245, 1125, \
											1725, 3600, 4464, 7104, 1756, 830, 5100, 2325, 2925, \
											3825, 1800};
int global_faa_product_template_nheight[] = {12000, 4783, 1180, 6800, 1120, 8640, 9375, 1170, 1900, \
											1050, 2700, 4464, 7392, 2481, 830, 6000, 2325, 2925, \
											2475, 5256};
#define GLOBAL_FAA_PRODUCT_TEMPLATE_NUMBER	20
const char* global_name_faa_product_template[] = { "generic", "apparel", "batterycharger", "beachtowel", "coffeemug", "duvetcover", "fleeceblanket", "masks", "phonecover", \
													"pouch", "puzzle", "roundtowel", "showercurtain", "spiralnotebook", "stickers", "tapestry", "throwpillow", "totebag", \
													"weekenderbag", "yogamat"};
//2021august18, spi, end
int global_newzoomwindowforeachframe = 0; //0 for no, 1 for yes
int global_output_scale_k = 12; //2 for 2K (1920x1080), 4 for 4K (3840x2160) or 12 for 12K (12000x12000), defaults to 12
string global_output_video = "no"; //"yes" or "no", defaults to "no"
int global_input_upscale = 0; //0 for no never upscale, 1 for yes slight upscale allowed, 2 for yes massive upscale allowed
int global_input_downscale = 0; //0 for no split will crop only, 1 for yes slight downscale allowed, 2 for yes massive downscale allowed
//spi, end, generalized version

std::string space2underscore(std::string text) {
	for (std::string::iterator it = text.begin(); it != text.end(); ++it) {
		if (*it == ' ') {
			*it = '_';
		}
	}
	return text;
}

std::string dot2dash(std::string text) {
	for (std::string::iterator it = text.begin(); it != text.end(); ++it) {
		if (*it == '.') {
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
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}


void unloadallimages()
{
	if (global_imagehandles.empty()) return;
	string s1 = "unloading all images";
	string s2 = " ...";
	string s = s1 + s2;
	cout << s << endl;
	if (myofstream.is_open())
	{
		myofstream << s << endl;
	}
	for (int i = 0; i < global_imagehandles.size(); i++)
	{
		FIBITMAP* pDIB = global_imagehandles[i];
		if (pDIB) FreeImage_Unload(pDIB);
	}
	global_imagehandles.clear();
	global_imagehandlesmap.clear();
	string s3 = "done, ";
	s = s3 + s + ".";
	cout << s << endl << endl;
	if (myofstream.is_open())
	{
		myofstream << s << endl << endl;
	}
}


void Terminate()
{
	unloadallimages();
	if (myofstream.is_open()) 
	{
		myofstream << "terminated." << endl;
		myofstream.close();
	}
	cout << "terminated" << endl;
	//2021oct31, spi, begin
	//system("pause");
	//exit(EXIT_FAILURE);
	exit(EXIT_SUCCESS);
	//2021oct31, spi, end
}

//The event signaled when the app should be terminated.
HANDLE g_hTerminateEvent = NULL;
//Handles events that would normally terminate a console application. 
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);

//Called by the operating system in a separate thread to handle an app-terminating event. 
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_C_EVENT ||
		dwCtrlType == CTRL_BREAK_EVENT ||
		dwCtrlType == CTRL_CLOSE_EVENT)
	{
		// CTRL_C_EVENT - Ctrl+C was pressed 
		// CTRL_BREAK_EVENT - Ctrl+Break was pressed 
		// CTRL_CLOSE_EVENT - Console window was closed 
		Terminate();
		// Tell the main thread to exit the app 
		::SetEvent(g_hTerminateEvent);
		return TRUE;
	}

	//Not an event handled by this function.
	//The only events that should be able to
	//reach this line of code are events that
	//should only be sent to services. 
	return FALSE;
}


//2021oct31, spi, begin
//to backup global_imagefilenames
vector<string> global_imagefilenames_backup;
FIBITMAP* global_p24bitDIBdefault = NULL;
FIBITMAP* global_pNew24bitDIB = NULL;

//picks n images, consecutive or not, depending upon the user-defined flags
void picknew_imagefilenamessubset_from_imagefilenamesbackup(vector<string> &vectorofstring)
{
	if (!vectorofstring.empty())
	{
		vectorofstring.clear();
	}
	//pick n images
	//vector<string> tmpvector;
	if (global_INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES || global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES)
	{
		int imageindexoffset = 0;
		if (global_INPUTIMAGES_ARE_TOBEUSEDFROMSTART)
		{
			//pick image index 0
			imageindexoffset = 0;
		}
		else if (global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES)
		{
			//pick a random image index offset
			imageindexoffset = RandomInt(0, global_imagefilenames_backup.size() - 1 - global_maxnumberofimagestoload);
		}
		//pick n consecutive images
		for (int icount = 0; icount < global_maxnumberofimagestoload; icount++)
		{
			int imageindex = imageindexoffset + icount;
			string imagetokeep = global_imagefilenames_backup.at(imageindex);
			vectorofstring.push_back(imagetokeep);
		}
	}
	else
	{
		//randomly pick n non-consecutive images
		for (int icount = 0; icount < global_maxnumberofimagestoload; icount++)
		{
			int imageindex = RandomInt(0, global_imagefilenames_backup.size() - 1);
			string imagetokeep = global_imagefilenames_backup.at(imageindex);
			//global_imagefilenames.erase(global_imagefilenames.begin() + imageindex);
			vectorofstring.push_back(imagetokeep);
		}
	}
}


//loads and pre-process all images into memory
//as well as populates image handles map and variability matrix
//so fractaltrace algo can compute new frames
//returns 0 on success and non-zero on failure
int reloadallimages_and_populateimagehandles()
{
	string s1 = "calling reloadallimages_and_populateimagehandles()";
	string s2 = " ...";
	string s = s1 + s2;
	cout << s << endl;
	if (myofstream.is_open())
	{
		myofstream << s << endl;
	}

	unloadallimages(); //free memory
	////////////////////////////////////////////
	//load all images and populate image handles
	////////////////////////////////////////////
	for (int imagefilenameindex = 0; imagefilenameindex < global_imagefilenames.size(); imagefilenameindex++)
	{
		FIBITMAP* pDIB = NULL;
		if (global_imageextension == ".jpg" || global_imageextension == ".JPG" || global_imageextension == ".jpeg" || global_imageextension == ".JPEG")
		{
			//pDIB = FreeImage_Load(FIF_JPEG, global_imagefilenames[imagefilenameindex].c_str(), JPEG_DEFAULT); 
			pDIB = FreeImage_Load(FIF_JPEG, global_imagefilenames[imagefilenameindex].c_str(), JPEG_EXIFROTATE);
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
		//2021jan23, spi, begin
		pDIB = p24bitDIB;
		if (global_INPUTIMAGES_ARE_WIDESCREENFRAMES) //if input images are widescreen formatted (i.e. 16:9 image with blacked out strip at the frame's top and bottom)
		{
			int originalwidth = FreeImage_GetWidth(p24bitDIB);
			int originalheight = FreeImage_GetHeight(p24bitDIB);
			int newwidth = originalwidth;
			int newheight = (originalwidth * 9 / 16);
			newwidth = newwidth * 90 / 100; //let's just take 90% to really kick out the black strips
			newheight = newheight * 90 / 100;
			int left = (originalwidth - newwidth) / 2;
			int top = (originalheight - newheight) / 2;
			int right = left + newwidth - 1;
			int bottom = top + newheight - 1;
			FIBITMAP* pCopiedDIB = FreeImage_Copy(p24bitDIB, left, top, right, bottom);
			if (pCopiedDIB == NULL)
			{
				if (myofstream.is_open()) myofstream << "error, found an image from which a widescreen 16:9 subframe could not be extracted, imagefilenameindex " << imagefilenameindex << endl << endl;
				cout << "error, ffound an image from which a widescreen 16:9 subframe could not be extracted" << endl;
				Terminate();
				return 1;
			}
			else
			{
				FreeImage_Unload(p24bitDIB);
				p24bitDIB = pCopiedDIB;
			}
		}
		//2021jan23, spi, end
		if (1) //generalized version, under development
		{
			//check input image size
			int nwidth = FreeImage_GetWidth(p24bitDIB);
			int nheight = FreeImage_GetHeight(p24bitDIB);
			int narea = nwidth * nheight;
			if ((global_output_scale_k == 2) && (nwidth != global_nwidth_2k || nheight != global_nheight_2k))
			{
				//1) upscale now to 2K if ... upscaling needed AND massive upscale allowed or slight upscaling allowed
				if (((narea < (global_nwidth_2k * global_nheight_2k)) && (global_input_upscale == 2))
					|| ((narea < (global_nwidth_2k * global_nheight_2k)) && (narea > (global_nwidth_2k * global_nheight_2k * 0.75)) && (global_input_upscale == 1)))
				{
					FIBITMAP* pRescaledDIB = FreeImage_Rescale(p24bitDIB, global_nwidth_2k, global_nheight_2k, FILTER_BICUBIC);
					if (pRescaledDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be upscaled at 1920x1080, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be upscaled at 1920x1080" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pRescaledDIB;
					}
				}
				//2.1) downscale now to 2K if ... downscaling needed AND massive downscale allowed or slight downscaling allowed
				else if (((narea > (global_nwidth_2k * global_nheight_2k)) && (global_input_downscale == 2))
					|| ((narea > (global_nwidth_2k * global_nheight_2k)) && (narea < (global_nwidth_2k * global_nheight_2k * 1.25)) && (global_input_downscale == 1)))
				{
					FIBITMAP* pRescaledDIB = FreeImage_Rescale(p24bitDIB, global_nwidth_2k, global_nheight_2k, FILTER_BICUBIC);
					if (pRescaledDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be downscaled at 1920x1080, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be downscaled at 1920x1080" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pRescaledDIB;
					}
				}
				//2.2) downscale now to 2k if ... downscaling needed AND can be splitted then rescaled with slight downscaling allowed
				else if ((narea > (global_nwidth_2k * global_nheight_2k)) && (global_input_downscale == 1))
				{
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, a 12K input is approximately 72 2K images and a 4K input is 4 2K images, n random extracts could do the job instead
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					int min_left_offset = 0;
					int max_left_offset = nwidth - global_nwidth_2k;
					int min_top_offset = 0;
					int max_top_offset = nheight - global_nheight_2k;
					if (max_top_offset <= 0 || max_left_offset <= 0)
					{
						if (myofstream.is_open()) myofstream << "error, cannot crop image to 1920x1080, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, cannot crop image to 1920x1080" << endl;
						Terminate();
						return 1;
					}
					/* //2020june13, note, cannot insert these new cropped bitmap in here, would need to create a new image list
					int nloop = 1;
					if (global_imagefilenames.size() == 1 && (nwidth == global_nwidth_12k) && (nheight == global_nheight_12k)) nloop = 72;
					if (global_imagefilenames.size() == 1 && (nwidth == global_nwidth_4k) && (nheight == global_nheight_4k)) nloop = 4;
					*/
					int left_offset = RandomInt(min_left_offset, max_left_offset);
					int top_offset = RandomInt(min_top_offset, max_top_offset);
					FIBITMAP* pCopiedDIB = FreeImage_Copy(p24bitDIB, left_offset, top_offset, left_offset + global_nwidth_2k, top_offset + global_nheight_2k);
					if (pCopiedDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be cropped at 1920x1080, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be cropped at 1920x1080" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pCopiedDIB;
					}
				}
				//2.3) randomly crop now to 2k if ... downscaling needed AND no massive downscale allowed AND no splitting allowed
				else if ((narea > (global_nwidth_2k * global_nheight_2k)) && (global_input_downscale == 0))
				{
					int min_left_offset = 0;
					int max_left_offset = nwidth - global_nwidth_2k;
					int min_top_offset = 0;
					int max_top_offset = nheight - global_nheight_2k;
					if (max_top_offset <= 0 || max_left_offset <= 0)
					{
						if (myofstream.is_open()) myofstream << "error, cannot crop image to 1920x1080, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, cannot crop image to 1920x1080" << endl;
						Terminate();
						return 1;
					}
					int left_offset = RandomInt(min_left_offset, max_left_offset);
					int top_offset = RandomInt(min_top_offset, max_top_offset);
					FIBITMAP* pCopiedDIB = FreeImage_Copy(p24bitDIB, left_offset, top_offset, left_offset + global_nwidth_2k, top_offset + global_nheight_2k);
					if (pCopiedDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be cropped at 1920x1080, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be cropped at 1920x1080" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pCopiedDIB;
					}
				}
			}
			else if ((global_output_scale_k == 4) && (nwidth != global_nwidth_4k || nheight != global_nheight_4k))
			{
				//1) upscale now to 4K if ... upscaling needed AND massive upscale allowed or slight upscaling allowed
				if (((narea < (global_nwidth_4k * global_nheight_4k)) && (global_input_upscale == 2))
					|| ((narea < (global_nwidth_4k * global_nheight_4k)) && (narea > (global_nwidth_4k * global_nheight_4k * 0.75)) && (global_input_upscale == 1)))
				{
					FIBITMAP* pRescaledDIB = FreeImage_Rescale(p24bitDIB, global_nwidth_4k, global_nheight_4k, FILTER_BICUBIC);
					if (pRescaledDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be rescaled at 3840x2160, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be rescaled at 3840x2160" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pRescaledDIB;
					}
				}
				//2.1) downscale now to 4K if ... downscaling needed AND massive downscale allowed or slight downscaling allowed
				else if (((narea > (global_nwidth_4k * global_nheight_4k)) && (global_input_downscale == 2))
					|| ((narea > (global_nwidth_4k * global_nheight_4k)) && (narea < (global_nwidth_4k * global_nheight_4k * 1.25)) && (global_input_downscale == 1)))
				{
					FIBITMAP* pRescaledDIB = FreeImage_Rescale(p24bitDIB, global_nwidth_4k, global_nheight_4k, FILTER_BICUBIC);
					if (pRescaledDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be downscaled at 3840x2160, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be downscaled at 3840x2160" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pRescaledDIB;
					}
				}
				//2.2) downscale now to 4k if ... downscaling needed AND can be splitted then rescaled with slight downscaling allowed
				else if ((narea > (global_nwidth_4k * global_nheight_4k)) && (global_input_downscale == 1))
				{
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, a 12K input is approximately 72 2K images and a 4K input is 4 2K images, n random extracts could do the job instead
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					int min_left_offset = 0;
					int max_left_offset = nwidth - global_nwidth_4k;
					int min_top_offset = 0;
					int max_top_offset = nheight - global_nheight_4k;
					if (max_top_offset <= 0 || max_left_offset <= 0)
					{
						if (myofstream.is_open()) myofstream << "error, cannot crop image to 3840x2160, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, cannot crop image to 3840x2160" << endl;
						Terminate();
						return 1;
					}
					/* //2020june13, note, cannot insert these new cropped bitmap in here, would need to create a new image list
					int nloop = 1;
					if (global_imagefilenames.size() == 1 && (nwidth == global_nwidth_12k) && (nheight == global_nheight_12k)) nloop = 72;
					if (global_imagefilenames.size() == 1 && (nwidth == global_nwidth_4k) && (nheight == global_nheight_4k)) nloop = 4;
					*/
					int left_offset = RandomInt(min_left_offset, max_left_offset);
					int top_offset = RandomInt(min_top_offset, max_top_offset);
					FIBITMAP* pCopiedDIB = FreeImage_Copy(p24bitDIB, left_offset, top_offset, left_offset + global_nwidth_4k, top_offset + global_nheight_4k);
					if (pCopiedDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be cropped at 3840x2160, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be cropped at 3840x2160" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pCopiedDIB;
					}
				}
				//2.3) randomly crop now to 4k if ... downscaling needed AND no massive downscale allowed AND no splitting allowed
				else if ((narea > (global_nwidth_4k * global_nheight_4k)) && (global_input_downscale == 0))
				{
					int min_left_offset = 0;
					int max_left_offset = nwidth - global_nwidth_4k;
					int min_top_offset = 0;
					int max_top_offset = nheight - global_nheight_4k;
					if (max_top_offset <= 0 || max_left_offset <= 0)
					{
						if (myofstream.is_open()) myofstream << "error, cannot crop image to 3840x2160, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, cannot crop image to 3840x2160" << endl;
						Terminate();
						return 1;
					}
					int left_offset = RandomInt(min_left_offset, max_left_offset);
					int top_offset = RandomInt(min_top_offset, max_top_offset);
					FIBITMAP* pCopiedDIB = FreeImage_Copy(p24bitDIB, left_offset, top_offset, left_offset + global_nwidth_4k, top_offset + global_nheight_4k);
					if (pCopiedDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be cropped at 3840x2160, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be cropped at 3840x2160" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pCopiedDIB;
					}
				}

			}
			else if ((global_output_scale_k == 12) && (nwidth != global_nwidth_12k || nheight != global_nheight_12k))
			{
				//1) upscale now to 12K if ... upscaling needed AND massive upscale allowed or slight upscaling allowed
				if (((narea < (global_nwidth_12k * global_nheight_12k)) && (global_input_upscale == 2))
					|| ((narea < (global_nwidth_12k * global_nheight_12k)) && (narea > (global_nwidth_12k * global_nheight_12k * 0.75)) && (global_input_upscale == 1)))
				{
					FIBITMAP* pRescaledDIB = FreeImage_Rescale(p24bitDIB, global_nwidth_12k, global_nheight_12k, FILTER_BICUBIC);
					if (pRescaledDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be rescaled at 12000x12000, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be rescaled at 12000x12000" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pRescaledDIB;
					}

				}
				//2.1) downscale now to 12K if ... downscaling needed AND massive downscale allowed or slight downscaling allowed
				else if (((narea > (global_nwidth_12k * global_nheight_12k)) && (global_input_downscale == 2))
					|| ((narea > (global_nwidth_12k * global_nheight_12k)) && (narea < (global_nwidth_12k * global_nheight_12k * 1.25)) && (global_input_downscale == 1)))
				{
					FIBITMAP* pRescaledDIB = FreeImage_Rescale(p24bitDIB, global_nwidth_12k, global_nheight_12k, FILTER_BICUBIC);
					if (pRescaledDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be downscaled at 12000x12000, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be downscaled at 12000x12000" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pRescaledDIB;
					}
				}
				//2.2) downscale now to 12k if ... downscaling needed AND can be splitted then rescaled with slight downscaling allowed
				else if ((narea > (global_nwidth_12k * global_nheight_12k)) && (global_input_downscale == 1))
				{
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, a 12K input is approximately 72 2K images and a 4K input is 4 2K images, n random extracts could do the job instead
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					//2020june13, initially implementing the randomly crop now instead of splitting input image in here
					int min_left_offset = 0;
					int max_left_offset = nwidth - global_nwidth_12k;
					int min_top_offset = 0;
					int max_top_offset = nheight - global_nheight_12k;
					if (max_top_offset <= 0 || max_left_offset <= 0)
					{
						if (myofstream.is_open()) myofstream << "error, cannot crop image to 12000x12000, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, cannot crop image to 12000x12000" << endl;
						Terminate();
						return 1;
					}
					/* //2020june13, note, cannot insert these new cropped bitmap in here, would need to create a new image list
					int nloop = 1;
					if (global_imagefilenames.size() == 1 && (nwidth == global_nwidth_12k) && (nheight == global_nheight_12k)) nloop = 72;
					if (global_imagefilenames.size() == 1 && (nwidth == global_nwidth_4k) && (nheight == global_nheight_4k)) nloop = 4;
					*/
					int left_offset = RandomInt(min_left_offset, max_left_offset);
					int top_offset = RandomInt(min_top_offset, max_top_offset);
					FIBITMAP* pCopiedDIB = FreeImage_Copy(p24bitDIB, left_offset, top_offset, left_offset + global_nwidth_12k, top_offset + global_nheight_12k);
					if (pCopiedDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be cropped at 3840x2160, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be cropped at 12000x12000" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pCopiedDIB;
					}
				}
				//2.3) randomly crop now to 12k if ... downscaling needed AND no massive downscale allowed AND no splitting allowed
				else if ((narea > (global_nwidth_12k * global_nheight_12k)) && (global_input_downscale == 0))
				{
					int min_left_offset = 0;
					int max_left_offset = nwidth - global_nwidth_12k;
					int min_top_offset = 0;
					int max_top_offset = nheight - global_nheight_12k;
					if (max_top_offset <= 0 || max_left_offset <= 0)
					{
						if (myofstream.is_open()) myofstream << "error, cannot crop image to 12000x12000, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, cannot crop image to 12000x12000" << endl;
						Terminate();
						return 1;
					}
					int left_offset = RandomInt(min_left_offset, max_left_offset);
					int top_offset = RandomInt(min_top_offset, max_top_offset);
					FIBITMAP* pCopiedDIB = FreeImage_Copy(p24bitDIB, left_offset, top_offset, left_offset + global_nwidth_12k, top_offset + global_nheight_12k);
					if (pCopiedDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be cropped at 12000x12000, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be cropped at 12000x12000" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(p24bitDIB);
						p24bitDIB = pCopiedDIB;
					}
				}

			}
			else if (((global_output_scale_k == 12) && (nwidth == global_nwidth_12k) && (nheight == global_nheight_12k))
				|| ((global_output_scale_k == 4) && (nwidth == global_nwidth_4k) && (nheight == global_nheight_4k))
				|| ((global_output_scale_k == 2) && (nwidth == global_nwidth_2k) && (nheight == global_nheight_2k))
				)
			{
				//cases where input images are already the same size as the required output frames
				//no rescaling required
			}
			else
			{
				if (myofstream.is_open()) myofstream << "error, cannot perform rescaling in this case for global_output_scale_k parameter " << global_output_scale_k << endl << endl;
				cout << "error, cannot perform rescaling in this case for global_output_scale_k parameter " << global_output_scale_k << endl;
				Terminate();
				return 1;
			}
		}
		if (0) //old code, leaving it here while generalized version under development
		{
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
		}
		//populate image handles
		global_imagehandles.push_back(p24bitDIB); //FIBITMAP*
	}
	//2021jan23, spi, begin
	//
	if (global_INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE)
	{
		//find min image size and max image size
		int minwidth = INT_MAX;
		int minheight = INT_MAX;
		int maxwidth = INT_MIN;
		int maxheight = INT_MIN;
		for (int imagefilenameindex = 0; imagefilenameindex < global_imagehandles.size(); imagefilenameindex++)
		{
			FIBITMAP* pDIB = global_imagehandles.at(imagefilenameindex);
			if (pDIB)
			{
				int nwidth = FreeImage_GetWidth(pDIB);
				if (nwidth < minwidth) minwidth = nwidth;
				if (nwidth > maxwidth) maxwidth = nwidth;
				int nheight = FreeImage_GetHeight(pDIB);
				if (nheight < minheight) minheight = nheight;
				if (nheight > maxheight) maxheight = nheight;
			}
		}
		//ensure minwidth is a multiple of 4
		if ((minwidth % 4) != 0)
		{
			minwidth = minwidth - (minwidth % 4);
		}
		if ((INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_WIDTH > 0) && (INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_WIDTH < minwidth))
		{
			minwidth = INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_WIDTH;
		}
		if ((INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_HEIGHT > 0) && (INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_HEIGHT < minheight))
		{
			minheight = INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE_HEIGHT;
		}
		//crop all input images to the same size (the size of the smallest width and height found)
		for (int imagefilenameindex = 0; imagefilenameindex < global_imagehandles.size(); imagefilenameindex++)
		{
			FIBITMAP* pDIB = global_imagehandles.at(imagefilenameindex);
			if (pDIB)
			{
				bool needcropping = false;
				int nwidth = FreeImage_GetWidth(pDIB);
				int nheight = FreeImage_GetHeight(pDIB);
				if (nwidth > minwidth) needcropping = true;
				if (nheight > minheight) needcropping = true;
				if (needcropping)
				{
					int min_left_offset = 0;
					int max_left_offset = nwidth - minwidth;
					int min_top_offset = 0;
					int max_top_offset = nheight - minheight;
					//if (max_top_offset <= 0 || max_left_offset <= 0)
					if (max_top_offset < 0 || max_left_offset < 0)
					{
						if (myofstream.is_open()) myofstream << "error, cannot crop image to minwidth by minheight, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, cannot crop image to minwidth by minheight" << endl;
						Terminate();
						return 1;
					}
					int left_offset = RandomInt(min_left_offset, max_left_offset);
					int top_offset = RandomInt(min_top_offset, max_top_offset);
					FIBITMAP* pCopiedDIB = FreeImage_Copy(pDIB, left_offset, top_offset, left_offset + minwidth, top_offset + minheight);
					if (pCopiedDIB == NULL)
					{
						if (myofstream.is_open()) myofstream << "error, found an image that could not be cropped at minwidth by minheight, imagefilenameindex " << imagefilenameindex << endl << endl;
						cout << "error, found an image that could not be cropped at minwidth by minheight" << endl;
						Terminate();
						return 1;
					}
					else
					{
						FreeImage_Unload(pDIB);
						pDIB = pCopiedDIB;
						//2021fev07, spi, begin
						global_imagehandles[imagefilenameindex] = pDIB;
						//2021fev07, spi, end
					}

				}
			}
		}
	}

	//2021oct31, spi, begin
	global_imagehandlesmap.clear();
	//2021oct31, spi, end
	//2021jan23, spi, begin
	//populate image handles map
	for (int imagefilenameindex = 0; imagefilenameindex < global_imagehandles.size(); imagefilenameindex++)
	{
		FIBITMAP* pDIB = global_imagehandles.at(imagefilenameindex);
		if (pDIB)
		{
			global_imagehandlesmap.insert(pair<FIBITMAP*, int>(pDIB, imagefilenameindex));
		}
		else
		{
			if (myofstream.is_open()) myofstream << "error, found an image that is null, imagefilenameindex " << imagefilenameindex << endl << endl;
			cout << "error, found an image that is null" << endl;
			Terminate();
			return 1;
		}
	}
	//2021jan23, spi, end

	//populate variability matrix
	int min_imageindexoffset = 0; //min imagehandles vector index offset
	int max_imageindexoffset = global_imagehandles.size() - 1; //max imagehandles vector index offset
	for (int ix = 0; ix < VARIABILITYMATRIXSIZE; ix++)
	{
		for (int iy = 0; iy < VARIABILITYMATRIXSIZE; iy++)
		{
			global_vm[ix][iy] = RandomInt(min_imageindexoffset, max_imageindexoffset); //variability matrix to be initialized with imagehandles vector index offsets
		}
	}
	//2021jan23, spi, end
	string s3 = "done, ";
	s = s3 + s1 + ".";
	cout << s << endl << endl;
	if (myofstream.is_open())
	{
		myofstream << s << endl << endl;
	}
	return 0; //success
}

int randompicknewdefaultimage_and_prepareoutputimagebuffer()
{
	string s1 = "calling randompicknewdefaultimage_and_prepareoutputimagebuffer()";
	string s2 = " ...";
	string s = s1 + s2;
	cout << s << endl;
	if (myofstream.is_open())
	{
		myofstream << s << endl;
	}
	//validation
	if (global_imagehandles.size() < 1)
	{
		string s1 = "error, global_imagehandles has no images";
		if (myofstream.is_open())
		{
			myofstream << s1 << endl << endl;
		}
		cout << s1 << endl;
		Terminate();
		return 1;
	}



	/////////////////////////////
	//random pick a default image
	/////////////////////////////
	int imagehandleindex = RandomInt(0, global_imagehandles.size() - 1);
	global_p24bitDIBdefault = global_imagehandles[imagehandleindex];
	if (global_p24bitDIBdefault==NULL)
	{
		cout << "error, global_p24bitDIBdefault is null" << endl;
		Terminate();
		return 1;
	}

	/////////////////////////////
	//prepare output image buffer
	/////////////////////////////
	if(global_pNew24bitDIB) FreeImage_Unload(global_pNew24bitDIB);
	global_pNew24bitDIB = FreeImage_Clone(global_p24bitDIBdefault);
	//spi, begin, generalized
	if (1) //in the generalized version, we do rescale output image buffers to 2k, 4k or 12k (12k is 12000x12000 pixels)
	{
		//ensure buffer has desired output scale
		int nwidth = FreeImage_GetWidth(global_pNew24bitDIB);
		int nheight = FreeImage_GetHeight(global_pNew24bitDIB);
		if ((global_output_scale_k == 2) && (nwidth != global_nwidth_2k || nheight != global_nheight_2k))
		{
			FIBITMAP* pRescaledDIB = FreeImage_Rescale(global_pNew24bitDIB, global_nwidth_2k, global_nheight_2k, FILTER_BICUBIC);
			if (pRescaledDIB == NULL)
			{
				if (myofstream.is_open()) myofstream << "error, output image could not be rescaled at 1920x1080" << endl << endl;
				cout << "error, output image could not be rescaled at 1920x1080" << endl;
				Terminate();
				return 1;
			}
			else
			{
				FreeImage_Unload(global_pNew24bitDIB);
				global_pNew24bitDIB = pRescaledDIB;
			}
		}
		else if ((global_output_scale_k == 4) && (nwidth != global_nwidth_4k || nheight != global_nheight_4k))
		{
			FIBITMAP* pRescaledDIB = FreeImage_Rescale(global_pNew24bitDIB, global_nwidth_4k, global_nheight_4k, FILTER_BICUBIC);
			if (pRescaledDIB == NULL)
			{
				if (myofstream.is_open()) myofstream << "error, output image could not be rescaled at 3840x2160" << endl << endl;
				cout << "error, output image could not be rescaled at 3840x2160" << endl;
				Terminate();
				return 1;
			}
			else
			{
				FreeImage_Unload(global_pNew24bitDIB);
				global_pNew24bitDIB = pRescaledDIB;
			}
		}
		else if ((global_output_scale_k == 12) && (nwidth != global_nwidth_12k || nheight != global_nheight_12k))
		{
			FIBITMAP* pRescaledDIB = FreeImage_Rescale(global_pNew24bitDIB, global_nwidth_12k, global_nheight_12k, FILTER_BICUBIC);
			if (pRescaledDIB == NULL)
			{
				if (myofstream.is_open()) myofstream << "error, output image could not be rescaled at 12000x12000" << endl << endl;
				cout << "error, output image could not be rescaled at 12000x12000" << endl;
				Terminate();
				return 1;
			}
			else
			{
				FreeImage_Unload(global_pNew24bitDIB);
				global_pNew24bitDIB = pRescaledDIB;
			}
		}
	}
	//spi, end, generalized
	string s3 = "done, ";
	s = s3 + s + ".";
	cout << s << endl << endl;
	if (myofstream.is_open())
	{
		myofstream << s << endl << endl;
	}
	return 0;
}
//2021oct31, spi, end





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

	//spi, begin, generalized
	/*
	int imagewidth = FreeImage_GetWidth(input_dib);
	int imageheight = FreeImage_GetHeight(input_dib);
	int selectionwidth = imagewidth;
	int selectionheight = imageheight;
	int selectionxmin = 0;
	int selectionxmax = selectionxmin+imagewidth;
	int selectionymin = 0;
	int selectionymax = selectionymin+imageheight;
	*/
	int imagewidth = FreeImage_GetWidth(input_dib);
	int imageheight = FreeImage_GetHeight(input_dib);
	int selectionwidth = FreeImage_GetWidth(output_dib);
	int selectionheight = FreeImage_GetHeight(output_dib);
	int selectionxmin = 0;
	int selectionxmax = selectionxmin + selectionwidth;
	int selectionymin = 0;
	int selectionymax = selectionymin + selectionheight;
	//spi, end, generalized

	//2021jan23, spi, begin
	//0) when OUTSIDE_TYPE_WRAP_WITHVARIABILITYMATRIX is enabled AND available
	//1) get global_imagehandles index from input_dib AND assert frames are all the same size
	//2) instead of standard input_dib image frame wrap, use alternate frame indexes
	//3) a 12K image is 72 2K images, and 3x3 is 9, and 5x5 is 25, and 7x7 is 49, and 9x9 is 81
	//   therefore we can use a predefined global 9x9 matrix of input_dib index offsets
	//find offset using
	//global_imagehandlesmap.find(input_dib)
	//2021jan23, spi, end

	double bailout2 = global_fractaltrace_bailout * global_fractaltrace_bailout;

	//gimp_progress_init (_("Fractal Trace"));

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
				switch (global_fractaltrace_outsidetype)
				{
				case OUTSIDE_TYPE_WRAP:
					px = fmod (px, imagewidth);
					py = fmod (py, imageheight);
					if( px < 0.0) px += imagewidth;
					if (py < 0.0) py += imageheight;
					pixels_get_biliner (input_dib, px, py, &pixel);
					break;

				//2021jan23, spi, begin
				case OUTSIDE_TYPE_WRAP_WITHVARIABILITYMATRIX:
					{
						double px_param, px_fractpart, px_intpart;
						double py_param, py_fractpart, py_intpart;
						px_param = px / (double)imagewidth;
						py_param = py / (double)imageheight;
						px_fractpart = modf(px_param, &px_intpart);
						py_fractpart = modf(py_param, &py_intpart);
						int vm_edgesize = (VARIABILITYMATRIXSIZE - 1) / 2; //i.e. for 9x9 matrix, edgesize is 4, 4 in all directions around a center
						int indexpx = ((int)px_intpart) % (vm_edgesize + 1); //indexpx will vary between -vm_edgesize to +vm_edgesize with 0 at center
						int indexpy = ((int)py_intpart) % (vm_edgesize + 1); //indexpy will vary between -vm_edgesize to +vm_edgesize with 0 at center
						int vm_indexpx = indexpx + vm_edgesize; //vm_indexpx will vary between 0 to 2*vm_edgesize with vm_edgesize at center
						int vm_indexpy = indexpy + vm_edgesize; //vm_indexpy will vary between 0 to 2*vm_edgesize with vm_edgesize at center
						map<FIBITMAP*, int>::iterator imagehandlesmap_it;
						imagehandlesmap_it = global_imagehandlesmap.find(input_dib);
						int inputframe_index = imagehandlesmap_it->second;
						int alternateframe_index = (inputframe_index + global_vm[vm_indexpx][vm_indexpy]) % (global_imagehandlesmap.size());
						FIBITMAP* alternateframe_dib = global_imagehandles.at(alternateframe_index);
						px = fmod(px, imagewidth);
						py = fmod(py, imageheight);
						if (px < 0.0) px += imagewidth;
						if (py < 0.0) py += imageheight;
						if (alternateframe_dib) pixels_get_biliner(alternateframe_dib, px, py, &pixel);
						else pixels_get_biliner(input_dib, px, py, &pixel);
					}
					break;
				//2021jan23, spi, end

				//case OUTSIDE_TYPE_TRANSPARENT:
				//	pixel.r = pixel.g = pixel.b = 0;
				//	pixel.a = 0;
				//	break;
					
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

		
		//if (((y - selection.y1) % (selection.height / 100)) == 0)
		//gimp_progress_update ((gdouble) (y-selection.y1) / selection.height);
		
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
		global_audiofilename = szArgList[15]; //ffmpeg will use this filename for merging audio to video, aubio will convert this audio file to a mono .wav file for analysis
	}
	if(nArgs>16)
	{
		global_maxnumberofframeperseedaudiofile = atoi(szArgList[16]); //unused
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
		//note: this path is not used in the 64bit version, opencv computer vision library has been fully integrated instead
		global_imagemagickconvertpath = szArgList[36];
	}
	if(nArgs>37)
	{
		
		//note: must be greater than 2 for doing the crossfade, otherwise crossfade is completely disabled
		//i.e. using 120 would mean crossfade over 4 seconds at 30 fps
		//i.e. using 3 is the smallest value for crossfade to occur
		global_maxnumberofframeforcrossfades = atoi(szArgList[37]); //defaults to -1 for no crossfade
	}
	if (nArgs > 38)
	{
		//i.e. defaults to 0 for no, so the fractaltrace zoom window does not change drastically from frame to frame
		//i.e. this parameter has been introduced when seeking to use each frame as an individual artwork, a case for which
		//i.e. high degree of variability between frames is prefered. this parameter can also be set to 1 if a very glitchy
		//i.e. video is prefered considering zoomwindows.txt file usually specifies many very different fractal space zoom areas.
		global_newzoomwindowforeachframe = atoi(szArgList[38]); //0 for no, 1 for yes
	}
	int default_output_scale_k = global_output_scale_k;
	if (nArgs > 39)
	{
		global_output_scale_k = atoi(szArgList[39]); //2 for 2K (1920x1080) output frames, 4 for 4K (3840x2160) output frames or 12 for 12K (12000x12000) output images, defaults to 12
	}
	if ( (global_output_scale_k != 2) && (global_output_scale_k != 4) && (global_output_scale_k != 12) ) global_output_scale_k = default_output_scale_k;
	if (nArgs > 40)
	{
		global_output_video = szArgList[40]; //"yes" or "no", defaults to "no"
		if(global_output_video=="YES") global_output_video="yes";
		if (global_output_video == "NO") global_output_video = "no";
		if ((global_output_video!="yes") && (global_output_video != "no")) global_output_video = "no";
	}
	int default_input_upscale = global_input_upscale;
	if (nArgs > 41)
	{
		global_input_upscale = atoi(szArgList[41]); //0 for no never upscale, 1 for yes slight upscale allowed, 2 for yes massive upscale allowed
	}
	if ((global_input_upscale != 0) && (global_input_upscale != 1) && (global_input_upscale != 2)) global_input_upscale = default_input_upscale;

	int default_input_downscale = global_input_downscale;
	if (nArgs > 42)
	{
		global_input_downscale = atoi(szArgList[42]); //0 for no split will crop only, 1 for yes slight downscale allowed, 2 for yes massive downscale allowed
	}
	if ((global_input_downscale != 0) && (global_input_downscale != 1) && (global_input_downscale != 2)) global_input_downscale = default_input_downscale;
	//2021july21, spi, begin
	if (nArgs > 43)
	{
		global_minnumberofbeats_beforechange = atoi(szArgList[43]); //only in effect when global_bnop is set to "beat" and is greater than -1, i.e. if one wants the changes to always occur somewhere between 4 and 8 beats, this variable should be set to 4
	}
	if (nArgs > 44)
	{
		global_maxnumberofbeats_beforechange = atoi(szArgList[44]); //only in effect when global_bnop is set to "beat" and is greater than -1, i.e. if one wants the changes to always occur somewhere between 4 and 8 beats, this variable should be set to 8
	}
	if (nArgs > 45)
	{
		//note: global_modestring can be left empty or can combine many modes 
		global_modestring = szArgList[45];
		if (!global_modestring.empty()) 
		{
			std::transform(global_modestring.begin(), global_modestring.end(), global_modestring.begin(), ::toupper);
			if (global_modestring.find("DEBUG") != std::string::npos)
			{
				//found!
				global_debug = true;
			}
			else
			{
				global_debug = false;
			}
			if (global_modestring.find("INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES") != std::string::npos)
			{
				//found!
				global_INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES = 1;
			}
			else
			{
				global_INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES = 0;
			}
			if (global_modestring.find("INPUTIMAGES_ARE_WIDESCREENFRAMES") != std::string::npos)
			{
				//found!
				global_INPUTIMAGES_ARE_WIDESCREENFRAMES = 1;
			}
			else
			{
				global_INPUTIMAGES_ARE_WIDESCREENFRAMES = 0;
			}
			if (global_modestring.find("INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE") != std::string::npos)
			{
				//found!
				global_INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE = 1;
			}
			else
			{
				global_INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE = 0;
			}
			if (global_modestring.find("OUTPUTIMAGES_ARE_TOBESCALEDTOFAAPRODUCTTEMPLATESIZE") != std::string::npos)
			{
				//found!
				global_OUTPUTIMAGES_ARE_TOBESCALEDTOFAAPRODUCTTEMPLATESIZE = 1;
			}
			else
			{
				global_OUTPUTIMAGES_ARE_TOBESCALEDTOFAAPRODUCTTEMPLATESIZE = 0;
			}
			//2021sept17, spi, begin
			if (global_modestring.find("INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES") != std::string::npos)
			{
				//found!
				global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES = 1;
			}
			else
			{
				global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES = 0;
			}
			//
			if (global_modestring.find("INPUTIMAGES_ARE_TOBEUSEDFROMSTART") != std::string::npos)
			{
				//found!
				global_INPUTIMAGES_ARE_TOBEUSEDFROMSTART = 1;
			}
			else
			{
				global_INPUTIMAGES_ARE_TOBEUSEDFROMSTART = 0;
			}
			//2021sept17, spi, end 
		}
	}
	//2021july21, spi, end

	//Auto-reset, initially non-signaled event 
	g_hTerminateEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	//Add the break handler
	::SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);


	myofstream.open("debug.txt", std::ios::out);
	if (myofstream.is_open())
	{
		myofstream << "spifractaltraceanimaudiobnopcrossfadegeneralized_vs2017.exe" << endl << endl;
		myofstream << "nArgs is :" << nArgs << endl << endl;
		myofstream << "modestring is: " << global_modestring << endl;
		myofstream << "global_debug is: " << global_debug << endl;
		myofstream << "global_INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES is: " << global_INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES << endl;
		myofstream << "global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES is: " << global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES << endl;
		myofstream << "global_INPUTIMAGES_ARE_TOBEUSEDFROMSTART is: " << global_INPUTIMAGES_ARE_TOBEUSEDFROMSTART << endl;
		myofstream << "global_INPUTIMAGES_ARE_WIDESCREENFRAMES is: " << global_INPUTIMAGES_ARE_WIDESCREENFRAMES << endl;
		myofstream << "global_INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE is: " << global_INPUTIMAGES_ARE_TOBESCALEDDOWNTOIDENTICALSIZE << endl;
		myofstream << "global_OUTPUTIMAGES_ARE_TOBESCALEDTOFAAPRODUCTTEMPLATESIZE is: " << global_OUTPUTIMAGES_ARE_TOBESCALEDTOFAAPRODUCTTEMPLATESIZE << endl << endl;
		myofstream << "global_fractaltrace_outsidetype is: " << global_fractaltrace_outsidetype << endl;
		myofstream << "global_fractaltrace_type is: " << global_fractaltrace_type << endl;
		myofstream << "global_fractaltrace_depth is: " << global_fractaltrace_depth << endl;
		myofstream << "global_maxnumberofframeforcrossfades is: " << global_maxnumberofframeforcrossfades << endl;
		myofstream << "global_newzoomwindowforeachframe is: " << global_newzoomwindowforeachframe << endl;
		myofstream << "global_input_upscale is: " << global_input_upscale << endl;
		myofstream << "global_input_downscale is: " << global_input_downscale << endl << endl;
		myofstream.flush();
	}

	if (global_maxnumberofimagestoload == 0)
	{
		cout << "error, global_maxnumberofimagestoload should not be 0, exiting app" << endl;
		if (myofstream.is_open())
		{
			myofstream << "error, global_maxnumberofimagestoload should not be 0, exiting app" << endl;
			myofstream.close();
		}
		return 1; //exit app
	}

	//0)
	//////////////////////////
	//initialize random number
	//////////////////////////
	//srand((unsigned)time(0));
	std::random_device rd;
	srand(rd());

	//////////////////////////////////////////////
	//if don't exist, create dir for output frames 
	//////////////////////////////////////////////
	//_mkdir(global_outputimagefolder.c_str());
	errno_t err;
	char buffer[256];
	if (_mkdir(global_outputimagefolder.c_str()) == ENOENT) //path not found
	{
		_get_errno(&err);
		strerror_s(buffer, 256, err);
		myofstream << "error creating dir (global_outputimagefolder), " << buffer << endl;
		cout << "error creating dir (global_outputimagefolder), " << buffer << endl;
		Terminate();
		return 1;
	}


	//2021july21, spi, begin
	/////////////////////////////////////////////////
	//convert audio from any audio format to mono wav
	/////////////////////////////////////////////////
	string mono_audiofilename = "mono_audiofilename.wav";
	string systemcommand;
	string quote = "\"";
	if (global_audiofilename != "")
	{
		systemcommand = global_ffmpegpath + " " + "-y -i " + quote + global_audiofilename + quote + " -ac 1 " + mono_audiofilename; //-y for forcing overwrite
		systemcommand = quote + systemcommand + quote; //wrap systemcommand with quote again
		cout << endl;
		cout << systemcommand << endl;
		cout << endl;
		system(systemcommand.c_str());
	}
	//2021july21, spi, end

	/////////////////////////
	//check audio file length
	/////////////////////////
	//global_pSNDFILE = sf_open(global_audiofilename.c_str(), SFM_READ, &global_mySF_INFO);
	global_pSNDFILE = sf_open(mono_audiofilename.c_str(), SFM_READ, &global_mySF_INFO);
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
			if (!temp.empty()) //2021oct31, added for safety by spi
			{
				global_imagefilenames.push_back(temp);
			}
		}
		//2021oct31, spi, begin
		//backup global_imagefilenames
		global_imagefilenames_backup = global_imagefilenames;
		if (myofstream.is_open())
		{
			myofstream << "global_imagefilenames_backup has now " << global_imagefilenames_backup .size() << " filenames" << endl << endl;
		}
		//2021oct31, spi, end
	}

	//2021jan23, spi, begin
	if ( (global_maxnumberofimagestoload > 0) && (global_imagefilenames.size()> global_maxnumberofimagestoload) )
	{
		//2021oct31, spi, begin
		//pick n images, consecutive or not, depending upon the user-defined flags
		picknew_imagefilenamessubset_from_imagefilenamesbackup(global_imagefilenames);
		//2021oct31, spi, end
	}
	//2021jan23, spi, end


	//2021oct31, spi, begin
	////////////////////////////////////////////
	//load all images and populate image handles
	////////////////////////////////////////////
	int iresult = reloadallimages_and_populateimagehandles();
	if (iresult)
	{
		Terminate();
		return iresult; //on error
	}
	//2021oct31, spi, end


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
		//2021july21, spi, begin
		//systemcommand = global_aubiotrackpath + " " + quote + global_audiofilename + quote + " > aubiotrack_beattimes.txt"; 
		systemcommand = global_aubiotrackpath + " " + quote + mono_audiofilename + quote + " > aubiotrack_beattimes.txt";
		//2021july21, spi, end
		system(systemcommand.c_str());
		//2) load in all "aubiotrack_beattimes.txt" file
		ifstream ifs("aubiotrack_beattimes.txt");
		string temp;
		while (getline(ifs, temp))
		{
			global_audiobeattimes_sec.push_back(atof(temp.c_str()));
		}
		//2021july21, spi, begin
		//will now flag out some beats so the output video does not change on every beat
		//but will rather change once every 4 to 8 beats for fm-org customer 
		//2.5) flag in and out beats according to caller's preferences
		int idbeat = 0;
		int idbeat_lastchange = 0;
		int idbeat_nextchange = global_audiobeattimes_sec.size();
		for (vector<float>::iterator iter = global_audiobeattimes_sec.begin(); iter != global_audiobeattimes_sec.end(); ++iter)
		{
			idbeat++;
			//*iter is a float
			if( (global_minnumberofbeats_beforechange>-1) && (global_maxnumberofbeats_beforechange>-1) && (idbeat>1) && ((idbeat-idbeat_lastchange)<idbeat_nextchange) )
			{
				global_audiobeattimes_flag.push_back(0); //to be dropped, flaged out, no change on this beat
			}
			else
			{
				idbeat_lastchange = idbeat;
				global_audiobeattimes_flag.push_back(1); //to be preserved, flaged in, change will occur on this beat
				idbeat_nextchange = RandomInt(global_minnumberofbeats_beforechange, global_maxnumberofbeats_beforechange);
			}
		}
		/*
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
		*/
		//3) quantize to frame numbers
		int prev_framenumber = -1;
		vector<float>::iterator iter1;
		vector<int>::iterator iter2;
		for (iter1 = global_audiobeattimes_sec.begin(), iter2 = global_audiobeattimes_flag.begin(); iter1 != global_audiobeattimes_sec.end() && iter2 != global_audiobeattimes_flag.end(); ++iter1, ++iter2)
		{
			//*iter is a float
			int framenumber = floor(((*iter1)*global_outputvideoframepersecond) + 0.5);
			if (framenumber == 0 || framenumber == 1 || framenumber == prev_framenumber || *iter2==0) continue;
			global_audiobeattimes_framenumber.push_back(framenumber);
			prev_framenumber = framenumber;
		}
		//2021july21, spi, end
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
		//2021july21, spi, begin
		//systemcommand = global_aubionotespath + " " + quote + global_audiofilename + quote + " > aubiotrack_notetimes.txt"; 
		systemcommand = global_aubionotespath + " " + quote + mono_audiofilename + quote + " > aubiotrack_notetimes.txt";
		//2021july21, spi, end
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
		//2021july21, spi, begin
		//systemcommand = global_aubiopitchpath + " " + quote + global_audiofilename + quote + " > aubiotrack_pitchtimes.txt"; 
		systemcommand = global_aubiopitchpath + " " + quote + mono_audiofilename + quote + " > aubiotrack_pitchtimes.txt";
		//2021july21, spi, end
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
		//2021july21, spi, begin
		//systemcommand = global_aubioonsetpath + " " + bufferthreshold + " " + bufferminimuminterval + " " + quote + global_audiofilename + quote + " > aubiotrack_onsettimes.txt";
		systemcommand = global_aubioonsetpath + " " + bufferthreshold + " " + bufferminimuminterval + " " + quote + mono_audiofilename + quote + " > aubiotrack_onsettimes.txt";
		//2021july21, spi, end
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




	//2)
	/////////////////
	//generate frames
	/////////////////
	string newfilename="";
	string newfolder=global_outputimagefolder;
	string newframefilenameprefix=global_framefilenameprefix; //"frame_";
	string newframefolder="";

	int ii=0;
	for (vector<string>::iterator it = global_audiofilenames.begin(); it != global_audiofilenames.end(); ++it)
	{
		ii++;
		//if(ii==2) break;
		if (global_maxnumberofaudiofilestoprocess == 0) break;
		if (global_maxnumberofaudiofilestoprocess > 0 && ii > global_maxnumberofaudiofilestoprocess) break;

		cout << endl << *it << endl;
		string filename = (*it).substr((*it).find_last_of("\\/") + 1);
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
		if (global_fractaltrace_tagoutputimagefilenames == 0)
		{
			newframefolder = dot2dash(fname);
			newfilename = newframefolder + global_videofilenameext;
		}
		else if (global_fractaltrace_tagoutputimagefilenames == 1)
		{
			//tag outputfilename
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

			newfilename += "(";
			newfilename += "depth";
			newfilename += bufferdepth;
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
			if (global_fractaltrace_type == 1)
			{
				newfilename += "jx";
				newfilename += bufferjx;
				newfilename += "jy";
				newfilename += bufferjy;
			}
			newfilename += ")";

			newfilename += "(";
			if (global_fasttranslationspeed == 1) newfilename += "fast";
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
			newfilename += global_videofilenameext;
		}
		/*
		else
		{
			newframefolder = dot2dash(fname);
			newfilename = newframefolder + global_videofilenameext;
		}
		*/
		string finalnewframefolder = global_outputimagefolder + "\\" + newframefolder;// + "(" + anim_translation_types[itranslation] + ")";
		errno_t err;
		char buffer[256];
		if (_mkdir(finalnewframefolder.c_str()) == -1)
		{
			_get_errno(&err);
			strerror_s(buffer, 256, err);
			myofstream << "error creating dir, " << buffer << endl;
			cout << "error creating dir, " << buffer << endl;
			Terminate();
			return 1;
		}

		myofstream.flush();
		//myofstream.close();

		//random pick a default image AND prepare output buffer 
		int iresult = randompicknewdefaultimage_and_prepareoutputimagebuffer();
		if (iresult) return iresult; //on error


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
			//2021oct31, spi, begin
			if ((global_fractaltrace_outsidetype==OUTSIDE_TYPE_WRAP_WITHVARIABILITYMATRIX) && global_newzoomwindowforeachframe && (iter!= global_segaudiobnoptimes_framenumber.begin()) && ((global_maxnumberofimagestoload > 0) && (global_imagefilenames_backup.size() > global_maxnumberofimagestoload)) )
			{
				//for each new video segment ...
				//if we want high variability AND we loaded less than all images
				//reload a new subset of images
				picknew_imagefilenamessubset_from_imagefilenamesbackup(global_imagefilenames);
				//for this new image filenames subset, reload all images and repopulate image handles
				int iresult = reloadallimages_and_populateimagehandles();
				if (iresult) return iresult; //on error
				//random pick a new default image AND prepare output buffer 
				iresult = randompicknewdefaultimage_and_prepareoutputimagebuffer();
				if (iresult) return iresult; //on error
			}
			//2021oct31, spi, end

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
			//2021jan15, spi, begin
			if (global_INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES || global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES) //i.e. when input images are a serie of video frames
			{
				iprobability = 1; //force the usage of a new image
			}
			//2021jan15, spi, end

			for(int iframe=0; iframe<maxnumberofframeforthisbnop; iframe++)
			{
				framenumber_absolute = framenumber_start + iframe;
				//2021oct31, spi, begin
				if (iframe!=0 && (global_fractaltrace_outsidetype == OUTSIDE_TYPE_WRAP_WITHVARIABILITYMATRIX) && global_newzoomwindowforeachframe && ((global_maxnumberofimagestoload > 0) && (global_imagefilenames_backup.size() > global_maxnumberofimagestoload)))
				{
					//for each new video frame ...
					//if we want high variability AND we loaded less than all images
					//reload a new subset of images
					picknew_imagefilenamessubset_from_imagefilenamesbackup(global_imagefilenames);
					//for this new image filenames subset, reload all images and repopulate image handles
					int iresult = reloadallimages_and_populateimagehandles();
					if (iresult) return iresult; //on error
					//random pick a new default image AND prepare output buffer 
					iresult = randompicknewdefaultimage_and_prepareoutputimagebuffer();
					if (iresult) return iresult; //on error
					//random pick a new image
					imagehandleindex = RandomInt(0, global_imagehandles.size() - 1);
					p24bitDIB = global_imagehandles[imagehandleindex];
				}
				//2021oct31, spi, end

				//2021jan15, spi, begin
				if (global_INPUTIMAGES_ARE_VIDEOSEQUENCEFRAMES || global_INPUTIMAGES_ARE_TOBEUSEDASSEQUENCEOFFRAMES) //i.e. when input images are a serie of video frames
				{
					//pick new image in sequence (instead of picking new image randomly)
					imagehandleindex = (framenumber_absolute-1) % global_imagehandles.size();
					if (imagehandleindex < 0) imagehandleindex = 0;
					if (imagehandleindex > global_imagehandles.size() - 1) imagehandleindex = global_imagehandles.size() - 1;
					p24bitDIB = global_imagehandles[imagehandleindex];
				}
				//2021jan15, spi, end


				///////////////////////////////////////////////////////////////////////////
				//if forced to change default image every x sec AND time has arrived, do it
				///////////////////////////////////////////////////////////////////////////
				if(global_defaultimagechangeperiod_sec>0.0 && global_defaultimagechangeperiod_framenumber>0)
				{
					if( (framenumber_absolute % global_defaultimagechangeperiod_framenumber)==0 )
					{
						int imagehandleindex = RandomInt(0, global_imagehandles.size() - 1);
						global_p24bitDIBdefault = global_imagehandles[imagehandleindex];
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

				//spi, begin, generalization
				if (global_newzoomwindowforeachframe)
				{
					///////////////////////////////////////////////
					//for each frame, random pick a new zoom window
					///////////////////////////////////////////////
					if (global_zoomwindowsvector.size() > 0)
					{
						int zoomwindowindex = RandomInt(0, global_zoomwindowsfloatsvector.size() - 1);
						struct zoomwindow myzoomwindow = global_zoomwindowsfloatsvector[zoomwindowindex];
						frame_fractaltrace_xmin = myzoomwindow.xmin;
						frame_fractaltrace_xmax = myzoomwindow.xmax;
						frame_fractaltrace_ymin = myzoomwindow.ymin;
						frame_fractaltrace_ymax = myzoomwindow.ymax;
					}
				}
				//spi, end, generalization

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
				/*
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
				*/
				if (global_pNew24bitDIB) 
				{
					// clone succeeded!
//2021july20, spi, begin
//fm-org customer, for debugging, generating only bnop txt files and reading these txt files until matching customer specification
					//if (1) //1 normaly, 0 when debuging
					if(!global_debug) //if (1) //if (0) //1 normaly, 0 when debuging
//2021july20, spi, end
					{
						if (iprobability == 1)
						{
							//new image
							DoFractalTrace(p24bitDIB, global_pNew24bitDIB, frame_fractaltrace_xmin, frame_fractaltrace_xmax, frame_fractaltrace_ymin, frame_fractaltrace_ymax);
						}
						else
						{
							//default image
							DoFractalTrace(global_p24bitDIBdefault, global_pNew24bitDIB, frame_fractaltrace_xmin, frame_fractaltrace_xmax, frame_fractaltrace_ymin, frame_fractaltrace_ymax);
						}
						//save final image buffer
						cout << finalnewfilename << " (out of " << global_audiofileduration_framenumber << ")" << endl;
						FreeImage_Save(FIF_JPEG, global_pNew24bitDIB, finalnewfilename.c_str());
						//2021august18, spi, begin
						//FAA product template generalization
						//2021august18_fineartamerica_product-templates.txt
						if (global_OUTPUTIMAGES_ARE_TOBESCALEDTOFAAPRODUCTTEMPLATESIZE == 1)
						{
							FIBITMAP* pFAA24bitDIB = FreeImage_Clone(global_pNew24bitDIB);
							if (pFAA24bitDIB)
							{
								int nwidth = FreeImage_GetWidth(pFAA24bitDIB);
								int nheight = FreeImage_GetHeight(pFAA24bitDIB);
								int greatestdimension = nwidth;
								if (nheight > nwidth) greatestdimension = nheight;
								for (int ifaaproducttemplate = 1; ifaaproducttemplate < GLOBAL_FAA_PRODUCT_TEMPLATE_NUMBER; ifaaproducttemplate++)
								{
									//rescale to fit faa product template
									int faagreatestdimension = global_faa_product_template_nwidth[ifaaproducttemplate];
									int faanwidth = greatestdimension;
									int faanheight = faanwidth * (1.0* global_faa_product_template_nheight[ifaaproducttemplate])/(1.0*global_faa_product_template_nwidth[ifaaproducttemplate]);
									if (global_faa_product_template_nheight[ifaaproducttemplate] > global_faa_product_template_nwidth[ifaaproducttemplate]) faagreatestdimension = global_faa_product_template_nheight[ifaaproducttemplate];
									faanheight = greatestdimension;
									faanwidth = faanheight * (1.0* global_faa_product_template_nwidth[ifaaproducttemplate]) / (1.0*global_faa_product_template_nheight[ifaaproducttemplate]);
									FIBITMAP* pRescaledDIB = FreeImage_Rescale(pFAA24bitDIB, faanwidth, faanheight, FILTER_BICUBIC);
									if (pRescaledDIB == NULL)
									{
										if (myofstream.is_open()) myofstream << "error, output image could not be rescaled at " << global_faa_product_template_nwidth[ifaaproducttemplate] << "x" << global_faa_product_template_nheight[ifaaproducttemplate] << endl << endl;
										cout << "error, output image could not be rescaled at " << global_faa_product_template_nwidth[ifaaproducttemplate] << "x" << global_faa_product_template_nheight[ifaaproducttemplate] << endl << endl;
										Terminate();
										return 1;
									}
									else
									{
										FreeImage_Unload(pFAA24bitDIB);
										pFAA24bitDIB = pRescaledDIB;
									}
									//do fractal trace
									if (iprobability == 1)
									{
										//new image
										DoFractalTrace(p24bitDIB, pFAA24bitDIB, frame_fractaltrace_xmin, frame_fractaltrace_xmax, frame_fractaltrace_ymin, frame_fractaltrace_ymax);
									}
									else
									{
										//default image
										DoFractalTrace(global_p24bitDIBdefault, pFAA24bitDIB, frame_fractaltrace_xmin, frame_fractaltrace_xmax, frame_fractaltrace_ymin, frame_fractaltrace_ymax);
									}
									//save final image buffer
									cout << ifaaproducttemplate << " (out of " << GLOBAL_FAA_PRODUCT_TEMPLATE_NUMBER << ")" << endl;
									size_t npos = finalnewfilename.find_last_of(".");
									if (npos != string::npos)
									{
										string faafriendlyfilename = finalnewfilename.substr(0, npos); //kick out extension
										faafriendlyfilename += "_"; //add underscore
										faafriendlyfilename += global_name_faa_product_template[ifaaproducttemplate]; //add faa template name
										faafriendlyfilename += finalnewfilename.substr(npos); //add extension back
										FreeImage_Save(FIF_JPEG, pFAA24bitDIB, faafriendlyfilename.c_str());
										cout << "saved file " << faafriendlyfilename << endl;
									}

								}
								FreeImage_Unload(pFAA24bitDIB);
							}
						}
						//2021august18, spi, end
					}

					/*
					//mark filename for delete
					if(iframe!=0 && iframe!=middleframeindex && iframe!=endframeindex)
					{
						global_imagefilenames_tobedeleted.push_back(finalnewfilename);
					}
					*/
					//FreeImage_Unload(p24bitDIB);
					/*
					FreeImage_Unload(pNew24bitDIB);
					pNew24bitDIB=NULL;
					*/
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
		//FreeImage_Unload(p24bitDIBdefault); //no need to unload, default image belongs to the image list
		//p24bitDIBdefault=NULL;
		FreeImage_Unload(global_pNew24bitDIB);
		global_pNew24bitDIB = NULL;


		///////////////////
		//do the crossfades
		///////////////////
		if(global_maxnumberofframeforcrossfades>2)
		{
			if (myofstream.is_open())
			{
				myofstream << endl << "crossfading start ..." << endl;
			}

			framenumber_start = 1;
			framenumber_next;
			framenumber_absolute;
			//for (vector<int>::iterator iter = global_segaudiobnoptimes_framenumber.begin() ; iter != global_segaudiobnoptimes_framenumber.end(); ++iter)
			for (vector<int>::iterator iter = global_segaudiobnoptimes_framenumber.begin() ; iter < global_segaudiobnoptimes_framenumber.end(); ++iter)
			{  
				//*iter is an int
				framenumber_next = *iter;
				int maxnumberofframeforthisbnop = framenumber_next - framenumber_start;
				int maxnumberofframeforthefollowingbnop = 0;
				if ((iter+1)< global_segaudiobnoptimes_framenumber.end())
				{
					maxnumberofframeforthefollowingbnop = *(iter + 1) - framenumber_next;
				}
				else
				{
					if (myofstream.is_open())
					{
						myofstream << "crossfading segment done." << endl;
					}
					break;
				}
				//2021sept17, spi, begin
				//int numberofframeforthiscrossfade = min(global_maxnumberofframeforcrossfades, maxnumberofframeforthisbnop/2);
				//i.e. when 2 consecutive cuts are 60 frames and 30 frames, numberofframeforthiscrossfade could not be greater than half of the shortest cut, 15 frames in such case
				int numberofframeforthiscrossfade = min(maxnumberofframeforthisbnop/2, maxnumberofframeforthefollowingbnop/2);
				numberofframeforthiscrossfade = min(global_maxnumberofframeforcrossfades, numberofframeforthiscrossfade);
				numberofframeforthiscrossfade = std::round(numberofframeforthiscrossfade * 0.5f) * 2.0f; //pick lowest even number
				//if(numberofframeforthiscrossfade>2 && numberofframeforthiscrossfade<maxnumberofframeforthefollowingbnop)
				if (numberofframeforthiscrossfade > 3)
				{
					if (myofstream.is_open())
					{
						myofstream << endl << "crossfading segment ..." << endl;
					}
#if defined _M_IX86
					//for now, kick out the support for crossfading in the 32 bit version
					//todo spi, write it back
					int dummy = 1;
					/*
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
					*/

#elif defined _M_X64
					//before crossfading, the boundaries, between the two series of distinct frames, were
					//int framenumber_boundary1 = framenumber_next - numberofframeforthiscrossfade - 1;
					int framenumber_boundary1 = framenumber_next - 1; //end of previous cut
					int framenumber_boundary2 = framenumber_next; //start of following cut
					//2021oct31, spi, begin
					/*
					//load these two distinct cuts with opencv
					char buf[256];
					sprintf(buf, "%06d", framenumber_boundary1);
					string finalnewfilename_first = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
					sprintf(buf, "%06d", framenumber_boundary2);
					string finalnewfilename_second = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
					cv::Mat src1 = cv::imread(finalnewfilename_first.c_str());
					cv::Mat src2 = cv::imread(finalnewfilename_second.c_str());
					if (src1.empty())
					{
						//error
						cout << endl << "error in crossfading" << endl;
						cout << "src1.empty(), cannot load frame " << finalnewfilename_first << endl;
					}
					if (src2.empty())
					{
						//error
						cout << endl << "error in crossfading" << endl;
						cout << "src2.empty(), cannot load frame " << finalnewfilename_second << endl;
					}
					//crossfade the two series of frames
					float fpercent = 100.0f/(numberofframeforthiscrossfade+1.0f); 
					//frames to be overwritten will start roughly at
					framenumber_boundary1 = framenumber_boundary1 - numberofframeforthiscrossfade / 2;
					//frames to be overwritten will finish roughly at
					framenumber_boundary2 = framenumber_boundary2 + numberofframeforthiscrossfade / 2; //+1 extra frame when numberofframeforthiscrossfade is odd
					for(int index=0; index<numberofframeforthiscrossfade; index++)
					{
						//they are always the same two already loaded frames to blend with different alpha
						int framenumber_firstplusoffset = framenumber_boundary1 + (index + 1); //ascending serie, should the +1 be kicked out?
						float fpercent_slice = fpercent * (index + 1);
						float fpercent_first = 100.0f- fpercent_slice;
						float fpercent_second = fpercent_slice;
						char buf[256];
						sprintf(buf, "%06d", framenumber_firstplusoffset);
						string finalnewfilename_firstplusoffset = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;						
						float falpha_first = fpercent_first / 100.0f;
						float falpha_second = fpercent_second / 100.0f;
						cv::Mat dst;
						cv::addWeighted(src1, falpha_first, src2, falpha_second, 0.0, dst);
						cv::imwrite(finalnewfilename_firstplusoffset.c_str(), dst);
						cout << finalnewfilename_firstplusoffset << endl;
					}
					*/
					//int framenumber_cut1_start = framenumber_boundary2 - (numberofframeforthiscrossfade / 2);
					int framenumber_cut1_start = framenumber_boundary2 - numberofframeforthiscrossfade;
					int framenumber_cut2_start = framenumber_boundary2;
					int framenumber_cut2_end = framenumber_boundary2 + numberofframeforthiscrossfade;
					int framenumber_cutoutput_start = framenumber_cut1_start; //output cut will overlap both cut1 and cut2
					//const int nimg = numberofframeforthiscrossfade / 2;
					const int nimg = numberofframeforthiscrossfade;
					cv::Mat* p_src1 = (cv::Mat*) new cv::Mat[nimg];
					cv::Mat* p_src2 = (cv::Mat*) new cv::Mat[nimg];
					if (p_src1 != NULL && p_src2 != NULL)
					{
						for (int index = 0; index < nimg; index++)
						{
							//load these two distinct cuts in memory with opencv
							char buf[256];
							sprintf(buf, "%06d", framenumber_cut1_start + index);
							string finalnewfilename_first = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
							//sprintf(buf, "%06d", framenumber_cut2_start + index);
							sprintf(buf, "%06d", framenumber_cut2_start + index + 1); //+1 for not loading framenumber_cut2_start but the consecutive frames
							string finalnewfilename_second = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
							p_src1[index] = cv::imread(finalnewfilename_first.c_str());
							p_src2[index] = cv::imread(finalnewfilename_second.c_str());
							if (p_src1[index].empty())
							{
								//error
								cout << endl << "error in crossfading" << endl;
								cout << "src1.empty(), cannot load frame " << finalnewfilename_first << endl;
								if (myofstream.is_open())
								{
									myofstream << "error, during crossfade" << endl;
									myofstream << "cannot load frame " << finalnewfilename_first << endl;
									myofstream << "framenumber_next is " << framenumber_next << endl;
								}
							}
							if (p_src2[index].empty())
							{
								//error
								cout << endl << "error in crossfading" << endl;
								cout << "src2.empty(), cannot load frame " << finalnewfilename_second << endl;
								if (myofstream.is_open())
								{
									myofstream << "error, during crossfade" << endl;
									myofstream << "cannot load frame " << finalnewfilename_second << endl;
									myofstream << "framenumber_next is " << framenumber_next << endl;
								}
							}
						}
						//crossfade the two series of frames (output overwriting first serie of frames)
						//cut1 is crossfaded with cut2 taken in reverse over original cut1 serie of frames 
						float fpercent = 100.0f / (numberofframeforthiscrossfade + 1.0f);
						cv::Mat dst; //opencv output image array
						for (int index_o = 0; index_o < numberofframeforthiscrossfade; index_o++)
						{
							int framenumber_firstplusoffset = framenumber_cutoutput_start + index_o; //ascending serie
							float fpercent_slice = fpercent * (index_o + 1);
							float fpercent_first = 100.0f - fpercent_slice;
							float fpercent_second = fpercent_slice;
							char buf[256];
							sprintf(buf, "%06d", framenumber_firstplusoffset);
							string finalnewfilename_firstplusoffset = finalnewframefolder + "\\" + newframefilenameprefix + buf + global_imageextension;
							float falpha_first = fpercent_first / 100.0f;
							float falpha_second = fpercent_second / 100.0f;
							//cv::Mat dst;
							//int index = index_o / 2; //index for opencv input image arrays
							//cv::addWeighted(p_src1[index], falpha_first, p_src2[index], falpha_second, 0.0, dst);
							cv::addWeighted(p_src1[index_o], falpha_first, p_src2[numberofframeforthiscrossfade-1-index_o], falpha_second, 0.0, dst);
							cv::imwrite(finalnewfilename_firstplusoffset.c_str(), dst);
							cout << finalnewfilename_firstplusoffset << endl;
						}
					}
					else
					{
						if (myofstream.is_open())
						{
							myofstream << "error, not enough memory during crossfade." << endl;
							myofstream << "aborting crossfade" << endl << endl;
						}
						if (p_src1) delete[] p_src1;
						if (p_src2) delete[] p_src2;
						break;
					}
					if (p_src1) delete[] p_src1;
					if (p_src2) delete[] p_src2;
					//2021oct31, spi, end
#endif
					//2021sept17, spi, end
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
					if (myofstream.is_open())
					{
						myofstream << "crossfading segment done." << endl;
					}
				}
				//framenumber_start = framenumber_absolute+1;
				framenumber_start = framenumber_next;
			}
			if (myofstream.is_open())
			{
				myofstream << "crossfading done." << endl << endl;
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
		//if(0) //don't fetch a video
		if (global_output_video=="yes" && global_output_scale_k==2) //do fetch a video
		{
			systemcommand = quote + systemcommand + quote; //wrap systemcommand with quote again
			cout << endl;
			cout << systemcommand << endl;
			cout << endl;
			if (myofstream.is_open())
			{
				myofstream << endl << systemcommand << endl;
			}
			system(systemcommand.c_str());
		}
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

	//Terminate();
	unloadallimages();
	if (myofstream.is_open()) myofstream.close();
	return 0;
}

