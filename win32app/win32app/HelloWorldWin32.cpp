/*  Author:			Chris Grycki
	Date:			7/29/2013
	Description:	My first Visual C++ program */



// Basic Header Files
#include <Windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <tchar.h>
#include <random>
#include <sstream>
#include <string>
#include <iostream>


using namespace std;

// Globals

// The main window class name
static TCHAR szWindowClass[] = _T("win32app");

// The string that appears in the application's title bar
static TCHAR szTitle[] = _T("Win32 Paint");

// Holds whether the brush is down for drawing
bool brushDown = false;

// Same thing for the eraser
bool eraserDown = false;

// Brushes for painting and erasing
HBRUSH myBrush;
HBRUSH eraser = CreateSolidBrush(RGB(255,255,255)); // Set to white by default
HPEN myPen = CreatePen(NULL,NULL,NULL);

// Random Initial Brush Color
static COLORREF init_BrushColor;
int init_red, init_green, init_blue;

// Current Brush Color;
COLORREF brushColor;

// Remember whether we're increasing or decreasing each RGB value
bool inc_red,inc_green,inc_blue;

// Used to display RGB information on screen
RECT recText;

// Radius of the brush
int radius = 75;

// Saves DC state while window moves
int saveDC = 0;

// Maximum brush size
const short MAX_BRUSH_SIZE = 250;

// Time
const UINT IDT_TIMER1 = 0x1;
const UINT IDT_TIMER2 = 0x2;
const UINT FPS_TIMER = 0x3;
\

UINT framesCounter = 0;
UINT cur_fps = 0;
UINT max_fps = 0;
UINT min_fps = 0;
UINT avg_fps = 0;


static int loops = 100;
static int timer_set = 10;

static int min_red = 100;
static int max_red = 255;

static int min_green = 100;
static int max_green = 255;

static int min_blue = 100;
static int max_blue = 255;

static int bg_red = 40;
static int bg_green = 255;
static int bg_blue = 30;
static bool bg_on = false;

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
COLORREF changeBrushColor(COLORREF);
void rgbDisp(HDC,int,int,int);
void engageTool(HDC,HBRUSH,LPARAM);
void init_Timer(HWND,WPARAM);
void DelayedWindow(HWND);
void draw_bg(HWND, LPRECT);
void threedim_circle(HWND, int, int, int);
HBRUSH threedim_circle_brush(COLORREF, int);

// Main function
int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{


	srand( (unsigned int) time(NULL));
	init_red = (rand()%(max_red+1-min_red))+min_red;
	init_green = (rand()%(max_green+1-min_green))+min_green;
	init_blue = (rand()%(max_blue+1-min_blue))+min_blue;
	brushColor = init_BrushColor = RGB(init_red,init_green,init_blue);

	// Create an area in top left for RGB text
	SetRect(&recText, 0,0,600,500);



	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Win32 Guided Tour"),
			NULL);

		return 1;
	}

	hInst = hInstance; // Store instance handle in our global variable

	// The parameters to CreateWindow explained:
	// szWindowClass:				the name of the application
	// szTitle:						the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW:			the type of window to create
	// CW_USEDEFAULT, CWUSEDEFAULT: Initial position (x,y)
	// 500, 100:					initial size (width, length)
	// NULL:						the parent of this window
	// NULL:						this application does not have a menu bar
	// hInstance:					the first paramerter from WinMain
	// NULL:						not used in this application
	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 1024,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Win32 Guided Tour"),
			NULL);

		return 1;
	}



	init_Timer(hWnd,IDT_TIMER2);
	init_Timer(hWnd,FPS_TIMER);
\

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	return (int) msg.wParam;


}
void DelayedWindow(HWND hWnd)
{
	// The parameters to ShowWindow explained:
	//	hWnd:		the value returned from CreateWindow
	//	SW_MAXIMIZE: Maximize the window
	ShowWindow(hWnd,
		SW_MAXIMIZE);
	UpdateWindow(hWnd);
	KillTimer(hWnd,IDT_TIMER2);
}
void threedim_circle(HDC threehdc, int threeradius, int threex, int threey)
{
	
	SelectObject(threehdc, myPen);

	while (threeradius > 0)
	{
		SelectObject(threehdc,myBrush);
		Ellipse(threehdc,
			threex-threeradius,
			threey-threeradius,
			threex+threeradius,
			threey+threeradius);
		DeleteObject(myBrush);
		myBrush = threedim_circle_brush(brushColor,threeradius);
		threeradius-=2;
	}
}
HBRUSH threedim_circle_brush(COLORREF oldBrushColor, int threeradius)
{
	int red = GetRValue(oldBrushColor);
	
	int green = GetGValue(oldBrushColor);
	
	int blue = GetBValue(oldBrushColor);

	if( ( red-(red/32)-(threeradius/32) ) >= 100 )
		red -= (red/32)-(threeradius/32);
	if( ( green-(green/32)-(threeradius/32) ) >= 100 )
		green -= (green/32)-(threeradius/32);
	if( ( blue-(blue/32)-(threeradius/32) ) >= 100 )
		blue -= (blue/32)-(threeradius/32);

	

	brushColor = RGB(red,green,blue);
	
	return CreateSolidBrush(brushColor);
}
	


void init_Timer(HWND hWnd, WPARAM wParam)
{
	UINT_PTR timerResult = NULL;
	switch(wParam)
	{
		case IDT_TIMER1:
			// Timer with error handling
			// Timer that draws circles
			timerResult = SetTimer(hWnd,IDT_TIMER1,timer_set,NULL);
			break;
		
		case IDT_TIMER2:
			// Timer with error handling
			// Timer that delays show window
			timerResult = SetTimer(hWnd,IDT_TIMER2,1*1000,NULL);
			break;
		case FPS_TIMER:
			timerResult = SetTimer(hWnd,FPS_TIMER,1000,NULL);
			break;
		default:
			break;

	}
	if( timerResult == 0)
	{
		DWORD dwError = GetLastError();
		LPWSTR buf=L"";
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM,
			0,
			dwError,
			0,
			buf,
			sizeof(buf),
			0);
		MessageBox(hWnd,
			buf,
			L"Error",
			MB_OK);
	}
}

COLORREF changeBrushColor(COLORREF brushColor)
{
	// Get RGB values
	int red = GetRValue(brushColor);
	int green = GetGValue(brushColor);
	int blue = GetBValue(brushColor);

	// Return Variable
	COLORREF newBrushColor;

	// Randomize
	int color_to_change = (rand()%3);

	// Increment/Decrement to one RGB value
	switch(color_to_change)
	{
	case 0: // Red
		{
			if (red == max_red)
				inc_red = false;
			else if (red == min_red)
				inc_red = true;
			if (inc_red)
				red++;
			else
				red--;

			break;
		}
	case 1: // Green
		{
			if (green == max_green)
				inc_green = false;
			else if (green == min_green)
				inc_green = true;
			if (inc_green)
				green++;
			else
				green--;

			break;
		}
	case 2: // Blue
		{

			if (blue == max_blue)
				inc_blue = false;
			else if (blue == min_blue)
				inc_blue = true;
			if (inc_blue)
				blue++;
			else
				blue--;
			break;	
		}
	}
	
	newBrushColor = RGB(red,green,blue);	
	return newBrushColor;
}

// Handles displaying the rgb information on screen
void rgbDisp(HDC hdc,int red, int green, int blue)
{
	// The stream that RGB information is written to
	std::wstringstream ostr;

	// Stream the rgb data
	ostr << "R: ";
	ostr << red;
	ostr << " G: ";
	ostr << green;
	ostr << " B: ";
	ostr << blue;
	ostr << "    ";

	// Temporary string to hold stream data
	const std::wstring tmp = ostr.str();

	
	
	DrawText(hdc,
		tmp.c_str(),			// String to display
		tmp.length(),	// Length of string
		&recText,				// Area to display in
		32);					// Something about the text format

	// Clear the stream
	ostr.clear();	

}
void engageTool(HDC hdc, HBRUSH tool, LPARAM lParam)
{
	

	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	

	// Old Code to draw a square
			/*
	RECT brushShape;
	brushShape.left = x - radius;
	brushShape.right = x + radius;
	brushShape.top = y - radius;
	brushShape.bottom = y + radius;
	FillRect(hdc,
		&brushShape,
		tool);*/


	// Create a pen to draw the border of the ellipse
	HPEN myPen = CreatePen(PS_NULL,NULL,NULL);

	// Select the proper pen and brush for the job
	HGDIOBJ oldBrush = SelectObject(hdc, tool);
	HGDIOBJ oldPen = SelectObject(hdc, myPen);

	Ellipse(hdc,
		x-radius,
		y-radius,
		x+radius,
		y+radius);
	
	// Return to the previous pen and brush
	SelectObject(hdc,oldBrush);
	SelectObject(hdc,oldPen);
	DeleteObject(myBrush);
	DeleteObject(myPen);
	if (brushDown)
		// Change global brushColor
		brushColor = changeBrushColor(brushColor);
}
void draw_bg(HDC hdc, LPRECT windowDimension)
{
	HBRUSH bg_brush = CreateSolidBrush(RGB(bg_red,bg_green,bg_blue));
	SelectObject(hdc,bg_brush);
	FillRect(hdc,windowDimension, myBrush);
	DeleteObject(bg_brush);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	HDC hdc = GetDC(hWnd);
	PAINTSTRUCT ps;
	short wheelPos;
	RECT windowDimension;
	GetWindowRect(hWnd, &windowDimension);

	wistringstream fps_imsg;
	string fps_msg_str;
	/*
	HBITMAP brushCursorBMap;
	brushCursorBMap = CreateCompatibleBitmap(hdc, radius*2, radius*2);
	HBITMAP brushDIB;
	
	// Bitmap Information Header
	BITMAPINFOHEADER bmh;
	bmh.biSize = sizeof(myBrush);
	bmh.biWidth = radius*2;
	bmh.biHeight = radius*2;
	bmh.biPlanes = 1;
	bmh.biBitCount = 24;
	bmh.biCompression = BI_RGB;
	bmh.biSizeImage = 0;
	bmh.biXPelsPerMeter = NULL;
	bmh.biYPelsPerMeter = NULL;
	bmh.biClrUsed = 0;
	bmh.biClrImportant = 0;

	tagRGBQUAD rgbq;
	rgbq.rgbRed = 1;
	rgbq.rgbGreen = 1;
	rgbq.rgbBlue = 1;
	rgbq.rgbReserved = 0;






	_ICONINFO cur;
	cur.fIcon = false;
	cur.xHotspot = radius;
	cur.yHotspot = radius;
	cur.hbmColor = brushCursorBMap;
	cur.hbmMask = brushCursorBMap;

								
	HCURSOR brushCursor = CreateIconIndirect(&cur);
	
	SetCursor(brushCursor);
	*/
	int y = (rand()%(windowDimension.bottom+1))+windowDimension.top;
	int x = (rand()%(windowDimension.right+1))+windowDimension.left;
	int radius = rand()%41+10;
	int width;
	int height;

	double elapsed_seconds;
	

	std::wostringstream fps_msg;
	
	HDC backbuffDC;
	HDC backbuffDC2;
	HBITMAP backBuffer;
	HBITMAP backBuffer2;
	
	std::wstring tmp;
	switch (message)
	{
	case WM_CREATE:
		GetWindowRect(hWnd, &windowDimension);
		init_Timer(hWnd,IDT_TIMER1);
		
		return 0;

	case WM_PAINT:
		
		
		BeginPaint(hWnd,&ps);

		// Here your application is laid out.

		//rgbDisp(hdc,init_red,init_green,init_blue);
		/*
		width = windowDimension.right;
		height = windowDimension.bottom;
		backbuffDC = CreateCompatibleDC(hdc);

		backBuffer = CreateCompatibleBitmap( hdc, width, height );
		SelectObject( backbuffDC, backBuffer );
		//hBrush = CreateSolidBrush(RGB(255,255,255));
		//FillRect(backbuffDC,&windowDimension,hBrush);
		BitBlt(hdc,0,0,width,height,backbuffDC,0,0,SRCCOPY);
		DeleteObject(hBrush);
		*/
		
		if (bg_on)
		{
		//	draw_bg(hdc, &windowDimension);
		}

		// End application-specific layout section.

		EndPaint(hWnd, &ps);
		

		break;
	case WM_WINDOWPOSCHANGED:
		GetWindowRect(hWnd, &windowDimension);
		break;
	case WM_TIMER:
		switch(wParam)
		{
		case(IDT_TIMER1):
			width = windowDimension.right;
			height = windowDimension.bottom;

			backbuffDC2 = CreateCompatibleDC(hdc);
			
			backBuffer2 = CreateCompatibleBitmap( hdc, width, height );

			// Copy hdc into buffer2
			SelectObject( backbuffDC2, backBuffer2 );
			BitBlt(backbuffDC2,0,0,width,height,hdc,0,0,SRCCOPY);
			
			// Use borderless pen
			myPen = CreatePen(PS_NULL,NULL,NULL);
			SelectObject(backbuffDC2,myPen);
			brushColor = changeBrushColor(brushColor);
			
			if (bg_on)
			{
				draw_bg(backbuffDC2, &windowDimension);
			}

			for(int i = 0;i<loops;i++)
			{
				y = (rand()%(windowDimension.bottom+1))+windowDimension.top;
				x = (rand()%(windowDimension.right+1))+windowDimension.left;
				radius = rand()%41+10;
				
				brushColor = changeBrushColor(brushColor);
				myBrush = CreateSolidBrush(brushColor);
				SelectObject(backbuffDC2,myBrush);		
				/*
				Ellipse(backbuffDC2,
					x-radius,
					y-radius,
					x+radius,
					y+radius);
				*/
				threedim_circle(backbuffDC2,radius,x,y);
				brushColor = init_BrushColor;
				DeleteObject(myBrush);			
			}

			DeleteObject(myPen);
			
			// Create intermediary buffer
			backbuffDC = CreateCompatibleDC(backbuffDC2);
			backBuffer = CreateCompatibleBitmap( backbuffDC2, width, height );

			// Copy from Buffer2 to Buffer1
			SelectObject(backbuffDC,backBuffer );
			BitBlt(backbuffDC,0,0,width,height,backbuffDC2,0,0,SRCCOPY);
			
			// Cleanup
			DeleteDC(backbuffDC2);
			DeleteObject(backBuffer2);

			//Copy from Buffer1 to hdc
			BitBlt(hdc,0,0,width,height,backbuffDC,0,0,SRCCOPY);

			// Count the frame after drawing it
			framesCounter++;

			// Cleanup
			DeleteDC(backbuffDC);
			DeleteObject(backBuffer);

			break;
		case(IDT_TIMER2):
			DelayedWindow(hWnd);
			break;
		case(FPS_TIMER):
			cur_fps = framesCounter;
			framesCounter = 0;
			if (max_fps < cur_fps)
				max_fps = cur_fps;
			if ((min_fps == 0) || (min_fps > cur_fps))
				min_fps = cur_fps;
			if (avg_fps != 0)
				avg_fps = (avg_fps+cur_fps)/2;
			else
				avg_fps = cur_fps;
			cur_fps = 0;
			break;
		}
		
		break;
	case WM_MOUSEWHEEL:
		wheelPos = GET_WHEEL_DELTA_WPARAM(wParam)/10;
		if ((radius + wheelPos)>MAX_BRUSH_SIZE)
			radius = MAX_BRUSH_SIZE;
		else if ((radius + wheelPos)<1)
			radius = 1;
		else
			radius += wheelPos;
		break;
	case WM_LBUTTONDOWN:
		// Left mouse button down turns brush on, engages tool once
		brushDown = true;
		// Capture cursor movement that goes outside window
		SetCapture(hWnd);
		myBrush = CreateSolidBrush(brushColor);		
		 
		ShowCursor(false);

		engageTool(hdc,myBrush,lParam);

		break;
	case WM_LBUTTONUP:
		// Left mouse button up turns brush off
		brushDown = false;

		// Destroy brush
		DeleteObject(myBrush);

		// Release the cursor capture
		ReleaseCapture();
		ShowCursor(true);
		break;
	case WM_RBUTTONDOWN:
		// Right mouse button down turns eraser on, engages tool once
		eraser = CreateSolidBrush(GetBkColor(hdc));
		eraserDown = true;
		// Capture cursor movement that goes outside window
		SetCapture(hWnd);
		engageTool(hdc,eraser,lParam);

		break;
	case WM_RBUTTONUP:
		eraserDown = false;
		// Release the cursor capture
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:

		// if the brush is down, then paint
		if (brushDown)
		{		
			// Initialize Brush
			myBrush = CreateSolidBrush(brushColor);
			
			// Get RGB values and display them on screen
			int red = GetRValue(brushColor);
			int green = GetGValue(brushColor);
			int blue = GetBValue(brushColor);

			// Call function to display rgb information on screen
			rgbDisp(hdc, red, green, blue);
			
			// Engage the tool
			engageTool(hdc,myBrush,lParam);	
			
		}
		else if (eraserDown)
		{
			//Begin drawing routine

			// Keep RGB information on screen
			// Get RGB values and display them on screen
			int red = GetRValue(brushColor);
			int green = GetGValue(brushColor);
			int blue = GetBValue(brushColor);

			// Call function to display rgb information on screen
			rgbDisp(hdc, red, green, blue);
			
			// Call drawing function
			engageTool(hdc,eraser,lParam);

			// End of drawing routine

		}
			
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_DESTROY:
		fps_msg << "Frames per Second Statistics\n";
		fps_msg << "\nAvg. FPS: ";
		fps_msg << std::to_wstring(avg_fps);
		fps_msg << "\nMin. FPS: ";
		fps_msg << std::to_wstring(min_fps);
		fps_msg << "\nMax. FPS: ";
		fps_msg << std::to_wstring(max_fps);
		//char tmp[100];
		//sprintf(tmp,"%d",fps_msg.str());
		
		
		MessageBox( hWnd,
					fps_msg.str().c_str(),
					_T("FPS STATS"),
					32);
					

		PostQuitMessage(0);
		break;
	default:
		ReleaseDC(hWnd, hdc);
		DeleteObject(myBrush);
//		DeleteObject(backBuffer);
//		DeleteDC(backbuffDC);
		//DestroyIcon(brushCursor);
		
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	//DestroyIcon(brushCursor);
	ReleaseDC(hWnd, hdc);
	DeleteObject(myPen);
	DeleteObject(myBrush);
//	DeleteObject(backBuffer);
//	DeleteDC(backbuffDC);
	return 0;

}





