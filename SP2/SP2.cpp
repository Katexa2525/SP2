#include <Windows.h>
#include <strsafe.h>
#include <vector>
#include <string>
#include <fstream>
#include <locale>
#include <sstream>
#include <string>
#include <io.h>
#include <fcntl.h>

using namespace std;
HWND hWnd;
int fontSize = 20;
const int COL_NUMBER = 3;
const int ROW_NUMBER = 3;
const int CELL_NUMBER = COL_NUMBER * ROW_NUMBER;
string textPieces[CELL_NUMBER];

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitDC(HWND hWnd, int windowWidth, int windowHeight);

// Функция для преобразования UTF-8 в ANSI
string ConvertUTF8ToANSI(const string& utf8Str)
{
  int requiredSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
  if (requiredSize == 0)
  {
    // Обработка ошибки
    return string(); // Возвращаем пустую строку в случае ошибки
  }

  vector<wchar_t> wideBuffer(requiredSize);
  MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideBuffer[0], requiredSize);

  requiredSize = WideCharToMultiByte(CP_ACP, 0, &wideBuffer[0], -1, NULL, 0, NULL, NULL);
  if (requiredSize == 0)
  {
    // Обработка ошибки
    return string(); // Возвращаем пустую строку в случае ошибки
  }

  vector<char> ansiBuffer(requiredSize);
  WideCharToMultiByte(CP_ACP, 0, &wideBuffer[0], -1, &ansiBuffer[0], requiredSize, NULL, NULL);

  return string(&ansiBuffer[0]);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
  WNDCLASSEX wcex;
  MSG msg;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_VREDRAW | CS_HREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = NULL;
  wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
  HBRUSH hBkgrndBrush = CreateSolidBrush(RGB(255, 255, 255)); // White background
  wcex.hbrBackground = hBkgrndBrush;
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = L"Lab2";
  wcex.hIconSm = NULL;

  if (!RegisterClassEx(&wcex))
  {
    return 0;
  }

  hWnd = CreateWindowEx(0, L"Lab2", L"SP2", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

  ifstream in("C:/Users/Катя/Desktop/5 сем/СП/SP2/ff.txt");
  string line;
  int i = 0;
  while (getline(in, line))
  {
    istringstream iss(line);
    string word;
    int col = 0;
    while (iss >> word)
    {
      textPieces[i * COL_NUMBER + col] = word;
      col++;
    }
    i++;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  while (GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static int width, height;

  switch (uMsg)
  {
  case WM_SIZE:
    width = LOWORD(lParam);
    height = HIWORD(lParam);
    break;
  case WM_PAINT:
    InitDC(hWnd, width, height);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  case WM_KEYUP:
  {
    switch (wParam)
    {
    case VK_UP:
      fontSize++;
      break;
    case VK_DOWN:
      fontSize--;
      break;
    }
    InvalidateRect(hWnd, NULL, TRUE);
  }
  }
  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void DrawTable(HDC currDC, int windowWidth, int windowHeight)
{
  int columnWidth = windowWidth / COL_NUMBER;
  int rowHeight = windowHeight / ROW_NUMBER;
  int x, y = 0;

  // Устанавливаем цвет линий (черный)
  HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  HPEN hOldPen = (HPEN)SelectObject(currDC, hPen);

  for (int i = 0; i < ROW_NUMBER; i++)
  {
    x = 0;
    for (int j = 0; j < COL_NUMBER; j++)
    {
      RECT cellRect = { x, y, x + columnWidth, y + rowHeight };

      string ansiText = ConvertUTF8ToANSI(textPieces[i * COL_NUMBER + j]);
      DrawTextA(currDC, ansiText.c_str(), -1, &cellRect, DT_EDITCONTROL | DT_WORDBREAK);

      // Рисуем вертикальные линии между столбцами
      if (j < COL_NUMBER - 1)
      {
        MoveToEx(currDC, x + columnWidth, y, NULL);
        LineTo(currDC, x + columnWidth, y + rowHeight);
      }

      x += columnWidth;
    }

    // Рисуем горизонтальные линии между строками
    if (i < ROW_NUMBER - 1)
    {
      MoveToEx(currDC, 0, y + rowHeight, NULL);
      LineTo(currDC, windowWidth, y + rowHeight);
    }

    y += rowHeight;
  }

  // Восстанавливаем предыдущее перо
  SelectObject(currDC, hOldPen);
  DeleteObject(hPen);
}


void InitDC(HWND hWnd, int windowWidth, int windowHeight)
{
  RECT rect;
  HDC memDC;
  HBITMAP hBmp, hOldBmp;
  PAINTSTRUCT ps;
  HDC currDC = BeginPaint(hWnd, &ps);

  GetClientRect(hWnd, &rect);

  memDC = CreateCompatibleDC(currDC);
  hBmp = CreateCompatibleBitmap(currDC, rect.right - rect.left, rect.bottom - rect.top);
  hOldBmp = (HBITMAP)SelectObject(memDC, hBmp);

  HBRUSH hBkgrndBrush = CreateSolidBrush(RGB(255, 255, 255));
  FillRect(currDC, &rect, hBkgrndBrush);
  DeleteObject(hBkgrndBrush);

  SetBkMode(currDC, TRANSPARENT);
  HFONT hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD, false, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, NULL);
  SelectObject(currDC, hFont);

  DrawTable(currDC, rect.right - rect.left, rect.bottom - rect.top);

  DeleteDC(memDC);
  EndPaint(hWnd, &ps);
}
