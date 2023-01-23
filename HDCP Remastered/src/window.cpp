#include <windows.h>
#include <thread>
#include <chrono>

#define Color unsigned int
#define blueThreshold 120

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void* mem;

BITMAPINFO bmi = { 0 };
HDC hdc;
int screenX;
int screenY;

int screen(HBITMAP& outbitmap) {
	HDC hScreenDC = GetDC(nullptr);

	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int width = screenX;//GetDeviceCaps(hScreenDC, HORZRES);
	int height = screenY;
	outbitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, outbitmap));
	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
	outbitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));
	
	DeleteDC(hMemoryDC);
	DeleteDC(hScreenDC);

	return 0;
}

Color dGetPixel(int xloc, int yloc, int screenX, int screenY) {
    int* pixel = (int*)mem;
    pixel += ((screenY - yloc - 1) * screenX) + xloc;
    return *pixel;
}

void dSetPixel(int locx, int locy, Color color, int screenX, int screenY) {
    Color getB = dGetPixel(locx, locy, screenX, screenY);
    int gR = (getB >> 16) & 0xff;
    int gG = (getB >> 8) & 0xff;
    int gB = getB & 0xff;

    int cR = (color >> 16) & 0xff;
    int cG = (color >> 8) & 0xff;
    int cB = color & 0xff;

    int zR = (gR + cR) / 2;
    int zG = (gG + cG) / 2;
    int zB = (gB + cB) / 2;

    if (locx < screenX && locy < screenY && locx >= 0 && locy >= 0) {
        int* pixel = (int*)mem;
        pixel += ((screenY - locy - 1) * screenX) + locx;
        *pixel = RGB(zB, zG, zR);
    }
}

int offset = 8;

void RenderOnBitmap(int width, int height, bool isRight, int pixx, int pixy){
    
    Color color = 0xFF3C3C;
    bool yes = false;

    if (pixx < width && pixx > 0) {
        if (pixy < height) {
            yes = true;
            Color z = dGetPixel(pixx, pixy, width, height);
            int g = (z >> 8) & 0xff;

            if (g > blueThreshold) {
                
                color = 0x3CDCFF;
            }

        }
    }

    if (isRight)
    {

        for (int y = 0; y < offset; y++)
        {
            for (int x = 0; x < offset; x++)
            {
                int xx = pixx + x;
                int yy = pixy + y;

                if (xx < width && xx > 0)
                {
                    if (yy < height)
                    {
                        if ((xx % 8) != 0) {
                            dSetPixel(xx, yy, color, width, height);
                        }
                    }
                }
            }
        }

        for (int y = 0; y < offset; y++)
        {
            for (int x = 0; x < offset; x++)
            {
                int xx = pixx + x + offset;
                int yy = pixy + y + offset;

                if (xx < width && xx > 0)
                {
                    if (yy < height)
                    {
                        if ((xx%8)!=0) {
                            dSetPixel(xx, yy, color, width, height);
                        }
                    }
                }
            }
        }
    }
    else
    {


        for (int y = 0; y < offset; y++)
        {
            for (int x = 0; x < offset; x++)
            {
                int xx = pixx + x + offset;
                int yy = pixy + y;

                if (xx < width && xx > 0)
                {
                    if (yy < height)
                    {
                        if ((xx % 8) != 0) {
                            dSetPixel(xx, yy, color, width, height);
                        }
                    }
                }

            }
        }

        for (int y = 0; y < offset; y++)
        {
            for (int x = 0; x < offset; x++)
            {
                int xx = pixx + x;
                int yy = pixy + y + offset;

                if (xx < width && xx > 0)
                {
                    if (yy < height)
                    {
                        if ((xx % 8) != 0) {
                            dSetPixel(xx, yy, color, width, height);
                        }
                    }
                }
            }
        }
    }


}


void th() {

    std::this_thread::sleep_for(std::chrono::seconds(10));

    int* loc = (int*)mem;
    for (int i = 0; i < screenX * screenY; i++) {
        *loc++ = 0;
    }
    SetDIBitsToDevice(hdc, 0, 0, screenX, screenY, 0, 0, 0, screenY, mem, &bmi, 0);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    exit(0);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {


    screenX = GetSystemMetrics(SM_CXMAXTRACK);
    screenY = GetSystemMetrics(SM_CYMAXTRACK);

    HBITMAP hbitmap;
    screen(hbitmap);

	const char* CLASS_NAME = "MainWindowClass";
	const char* WINDOW_NAME = "MainWindow";

	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc = WndProc;
	RegisterClassEx(&wc);




	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST, CLASS_NAME, WINDOW_NAME, WS_POPUP, 0, 0, screenX, screenY, NULL, NULL, NULL, NULL);
	


	size_t memorySize = (size_t)(screenX * screenY * 4);

	mem = malloc(memorySize);
	

	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = screenX;
	bmi.bmiHeader.biHeight = screenY;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;


	BITMAP bitmap;
	GetObject(hbitmap, sizeof(bitmap), (LPVOID)&bitmap);

	hdc = GetDC(hwnd);

	if (!mem) {
		return 0;
	}


	HDC hMemDC = CreateCompatibleDC(NULL);
	(HBITMAP)SelectObject(hMemDC, hbitmap);

	GetDIBits(hdc, hbitmap, 0, screenY, mem, &bmi, 0);

	ShowWindow(hwnd, SW_SHOW);


    int stepsV = screenY/4;

    int stepsH = screenX/4;

    for (int v = 0; v < stepsH; v++)
    {
        for (int i = 0; i < stepsV; i++)
        {
            int offset2 = offset * 2;

            int thing = i * offset2;
            int thing2 = v * (offset2 + 50);
            int thing3 = -screenY;

            RenderOnBitmap(screenX, screenY, i % 2 == 1, thing + thing2 + thing3, thing);
        }
    }

    SetDIBitsToDevice(hdc, 0, 0, screenX, screenY, 0, 0, 0, screenY, mem, &bmi, 0);
    
    ShowCursor(0);

    RECT r = { 0,0,0,0 };
    ClipCursor(&r);

    std::thread ok{ th };

    
	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
    
    ok.detach();
    

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_CREATE: {



		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	default: {
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	}
	return 0;
}