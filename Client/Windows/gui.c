#include <windows.h>
#include <stdio.h>
#include "Resource.h"

#define MAX_LOADSTRING 100

// Variables globales :
HINSTANCE hInst;                                // instance actuelle
WCHAR szTitle[MAX_LOADSTRING];                  // Le texte de la barre de titre
WCHAR szWindowClass[MAX_LOADSTRING];            // le nom de la classe de fenêtre principale
HWND mainWnd, hwndLabel;
static const char *messageText;

// Pré-déclarations des fonctions incluses dans ce module de code :
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY guiMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    MyRegisterClass(hInstance);

    // Effectue l'initialisation de l'application :
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Boucle de messages principale :
    while (GetMessage(&msg, NULL, 0, 0))
    {
	DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}


//
//  FONCTION : MyRegisterClass()
//
//  BUT : inscrit la classe de fenêtre.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREENCAST));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = L"PROMYS";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FONCTION : InitInstance(HINSTANCE, int)
//
//   BUT : enregistre le handle de l'instance et crée une fenêtre principale
//
//   COMMENTAIRES :
//
//        Dans cette fonction, nous enregistrons le handle de l'instance dans une variable globale, puis
//        créons et affichons la fenêtre principale du programme.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Stocke le handle d'instance dans la variable globale

   HWND hWnd = CreateWindow(TEXT("PROMYS"), TEXT("PROMYS"), WS_POPUP|WS_BORDER|WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, 350, 80, NULL, NULL, hInstance, NULL);

   mainWnd = hWnd;

   if (!hWnd)
   {
      return FALSE;
   }

   hwndLabel = CreateWindow(
	   TEXT("STATIC"),                   /*The name of the static control's class*/
	   TEXT("Starting..."),                  /*Label's Text*/
	   WS_CHILD | WS_VISIBLE | SS_CENTER,  /*Styles (continued)*/
	   10,                                /*X co-ordinates*/
	   30,                                /*Y co-ordinates*/
	   350,                               /*Width*/
	   30,                               /*Height*/
	   hWnd,                             /*Parent HWND*/
	   (HMENU)NULL,              /*The Label's ID*/
	   hInstance,                        /*The HINSTANCE of your program*/
	   NULL);                            /*Parameters for main window*/

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FONCTION : WndProc(HWND, UINT, WPARAM, LPARAM)
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_APP+1:
	SendMessage(hwndLabel, WM_SETTEXT, 0, (LPARAM)messageText);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void showMessage(const char *message) {
    messageText = message;
    PostMessage(mainWnd, WM_APP+1, 0, 0);
}

void hideWindow() {
    ShowWindow(mainWnd, SW_MINIMIZE);
}
