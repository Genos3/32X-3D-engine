#define _WIN32_WINNT 0x0600
#include <windows.h>
// #include <stdio.h>
#include "common.h"

HDC hdc;
HDC memdc;

// Function Declarations
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HBITMAP initscreen(HDC hdc, int w, int h, u16 **data);
//void toggleFullscreen(HWND hwnd);

int windows_width, windows_height;
static u8 view_lines_mode;
float delta_time_f;

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow) {
  WNDCLASS wc;
  HWND hwnd;
  MSG msg;
  //HDC hdc;
  //HDC memdc;
  HBITMAP bmp;
  BOOL quit = FALSE;
  
  // register window class
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "test";
  RegisterClass(&wc);
  
  int wf, hf;
  wf = PC_SCREEN_WIDTH * PC_SCREEN_SCALE_SIZE; //* 5
  hf = PC_SCREEN_HEIGHT * PC_SCREEN_SCALE_SIZE; //* 5
  windows_width = wf + 6; //wf
  windows_height = hf + 35; //hf
  //550,300,262,287
  SetProcessDPIAware();
  
  // create main window
  hwnd = CreateWindow(
    "test", "test",
    WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_MINIMIZEBOX,
    0, 0, windows_width, windows_height,
    NULL, NULL, hInstance, NULL);
  hdc = GetDC(hwnd);
  
  memdc = CreateCompatibleDC(hdc);
  bmp = initscreen(hdc, PC_SCREEN_WIDTH, PC_SCREEN_HEIGHT + 1, &screen); // account for offscreen rendering
  SelectObject(memdc, bmp);
  timeBeginPeriod(1);
  
  MoveWindow(hwnd, 600, 200, windows_width, windows_height, TRUE);

  //fullscreen=0;
  view_lines_mode = 0;
  frame_cycles = 0;
  delta_time_f = 0;
  fps = 60;
  redraw_scene = 1;
  key_curr = 0;
  dbg_show_poly_num = 0;
  dbg_show_grid_tile_num = 0;
  
  init_3d();
  //BitBlt(hdc, 0, 0, PC_SCREEN_WIDTH, PC_SCREEN_HEIGHT, memdc, 0, 0, SRCCOPY);
  StretchBlt(hdc, 0, 0, wf, hf, memdc, 0, 0, PC_SCREEN_WIDTH, PC_SCREEN_HEIGHT, SRCCOPY);
  
  float update_time = 1000.0 / 60.0; //20;
  float delta_time_t = update_time;
  LARGE_INTEGER t, rt;
  QueryPerformanceFrequency(&t);
  float PCFreq = t.QuadPart / 1000.0;
  
  // program main loop
  while (!quit) {
    key_prev = key_curr;
    
    // check for messages
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      // handle or dispatch messages
      if (msg.message == WM_QUIT) {
        quit = TRUE;
      } else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    
    QueryPerformanceCounter(&t);
    if (delta_time_t < update_time) {
      delta_time_t = update_time;
    }
    
    while (delta_time_t > 0) {
      //processInput(hwnd);
      float dt = min_c(delta_time_t, update_time);
      update_state(fix(dt / 1000));
      delta_time_t -= update_time;
    }
    
    if (redraw_scene) {
      draw();
      
      if (cfg.debug) {
        draw_dbg();
      }
      //BitBlt(hdc, 0, 0, PC_SCREEN_WIDTH, PC_SCREEN_HEIGHT, memdc, 0, 0, SRCCOPY);
      StretchBlt(hdc, 0, 0, wf, hf, memdc, 0, 0, PC_SCREEN_WIDTH, PC_SCREEN_HEIGHT, SRCCOPY);
      redraw_scene = 0;
    }

    QueryPerformanceCounter(&rt);
    delta_time_f = (rt.QuadPart - t.QuadPart) / PCFreq;
    delta_time_t = delta_time_f;
    fps = 1000.0 / delta_time_f;
    if (fps > 60) fps = 60;
    
    if (delta_time_f < update_time - 1) {
      Sleep(update_time - delta_time_f - 1);
    }
  }
  
  //free(z_buffer);
  timeEndPeriod(1);
  DeleteObject(bmp);
  DeleteDC(memdc);
  ReleaseDC(hwnd, hdc);
  DestroyWindow(hwnd);
  return msg.wParam;
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg){
    
    case WM_CLOSE:
      PostQuitMessage(0);
      break;
    
    case WM_KEYDOWN:
      if (!(lParam & 0x40000000)) {
        if (wParam == 'W') {
          key_curr |= KEY_UP;
        } else
        if (wParam == 'S') {
          key_curr |= KEY_DOWN;
        }
        
        if (wParam == 'A') {
          key_curr |= KEY_LEFT;
        } else
        if (wParam == 'D') {
          key_curr |= KEY_RIGHT;
        }
        
        if (wParam == VK_NUMPAD1) {
          key_curr |= KEY_A;
        }
        
        if (wParam == VK_NUMPAD2) {
          key_curr |= KEY_B;
        }
        
        if (wParam == VK_NUMPAD3) {
          key_curr |= KEY_C;
        }
        
        if (wParam == VK_NUMPAD4) {
          key_curr |= KEY_X;
        }
        
        if (wParam == VK_NUMPAD5) {
          key_curr |= KEY_Y;
        }
        
        if (wParam == VK_NUMPAD6) {
          key_curr |= KEY_Z;
        }
        
        if (wParam == VK_RETURN) {
          key_curr |= KEY_START;
        }
        
        if (wParam == VK_BACK) {
          key_curr |= KEY_MODE;
        }
        
        if (wParam == 'F') {
          cfg.debug ^= 1;
          redraw_scene = 1;
        }
        
        if (wParam == 'G') { //lines
          if (!view_lines_mode) {
            view_lines_mode = 1;
            cfg.draw_lines = 1;
          } else
          if (view_lines_mode == 1) {
            view_lines_mode = 2;
            cfg.draw_polys = 0;
          } else {
            view_lines_mode = 0;
            cfg.draw_lines = 0;
            cfg.draw_polys = 1;
          }
          
          redraw_scene = 1;
        }
        
        if (wParam == 'R') {
          cfg.tx_perspective_mapping_enabled ^= 1;
          redraw_scene = 1;
        }
        
        if (wParam == 'T') {
          cfg.draw_textures ^= 1;
          redraw_scene = 1;
        }
        
        if (wParam == 'V') {
          dbg_show_grid_tile_num ^= 1;
          redraw_scene = 1;
        }
        
        if (wParam == 'B') {
          dbg_show_poly_num ^= 1;
          redraw_scene = 1;
        }
        
        if (wParam == 'N') {
          if (dbg_show_poly_num) {
            if (GetKeyState(VK_SHIFT) & 0x8000) {
              dbg_num_poly_dsp -= 10;
              
              if (dbg_num_poly_dsp < 0) {
                dbg_num_poly_dsp = 0;
              }
            } else {
              dbg_num_poly_dsp--;
              
              if (dbg_num_poly_dsp < 0) {
                dbg_num_poly_dsp = g_model->num_faces;
              }
            }
          } else
          if (dbg_show_grid_tile_num) {
            if (GetKeyState(VK_SHIFT) & 0x8000) {
              dbg_num_grid_tile_dsp -= 10;
              
              if (dbg_num_grid_tile_dsp < 0) {
                dbg_num_grid_tile_dsp = 0;
              }
            } else {
              dbg_num_grid_tile_dsp--;
              
              if (dbg_num_grid_tile_dsp < 0) {
                dbg_num_grid_tile_dsp = dl.num_tiles;
              }
            }
          }
          
          redraw_scene = 1;
        }
        
        if (wParam == 'M') {
          if (dbg_show_poly_num) {
            if (GetKeyState(VK_SHIFT) & 0x8000) {
              dbg_num_poly_dsp += 10;
              
              if (dbg_num_poly_dsp > g_model->num_faces) {
                dbg_num_poly_dsp = g_model->num_faces;
              }
            } else {
              dbg_num_poly_dsp++;
              
              if (dbg_num_poly_dsp > g_model->num_faces) {
                dbg_num_poly_dsp = 0;
              }
            }
          } else
          if (dbg_show_grid_tile_num) {
            if (GetKeyState(VK_SHIFT) & 0x8000) {
              dbg_num_grid_tile_dsp += 10;
              
              if (dbg_num_grid_tile_dsp > dl.num_tiles) {
                dbg_num_grid_tile_dsp = dl.num_tiles;
              }
            } else {
              dbg_num_grid_tile_dsp++;
              
              if (dbg_num_grid_tile_dsp > dl.num_tiles) {
                dbg_num_grid_tile_dsp = 0;
              }
            }
          }
          
          redraw_scene = 1;
        }
        //printf("%d",(int)i_directionx_pressed);
        //printf(" %d\n",(int)i_directiony_pressed);
        if (wParam == VK_ESCAPE) {
          PostQuitMessage(0);
        }
        //if (wParam==VK_F11){toggleFullscreen(hwnd);}
        /* if (wParam==VK_RETURN){
          if (!animation_play){animation_play=1;} else {animation_play=0;}
        } */
      } //reset camera
      break;

    case WM_KEYUP:
      if (wParam == 'W') {
        key_curr ^= KEY_UP;
      }
      
      if (wParam == 'S') {
        key_curr ^= KEY_DOWN;
      }
      
      if (wParam == 'A') {
        key_curr ^= KEY_LEFT;
      }
      
      if (wParam == 'D') {
        key_curr ^= KEY_RIGHT;
      }
      
      if (wParam == VK_NUMPAD1) {
        key_curr ^= KEY_A;
      }
      
      if (wParam == VK_NUMPAD2) {
        key_curr ^= KEY_B;
      }
      
      if (wParam == VK_NUMPAD3) {
        key_curr ^= KEY_C;
      }
      
      if (wParam == VK_NUMPAD4) {
        key_curr ^= KEY_X;
      }
      
      if (wParam == VK_NUMPAD5) {
        key_curr ^= KEY_Y;
      }
      
      if (wParam == VK_NUMPAD6) {
        key_curr ^= KEY_Z;
      }
      
      if (wParam == VK_RETURN) {
        key_curr ^= KEY_START;
      }
      
      if (wParam == VK_BACK) {
        key_curr ^= KEY_MODE;
      }
      
      //printf("%d",(int)i_directionx_pressed);
      //printf(" %d\n",(int)i_directiony_pressed);
      break;
    
    /* case WM_SYSKEYDOWN:
    if (wParam==VK_RETURN && HIWORD(lParam)&KF_ALTDOWN){
      toggleFullscreen(hwnd);
      return 0;
    }
    break; */
    
    /* case WM_ACTIVATE:
    if (!wParam){
      if (mouse_capture){mouse_capture=0; setCursorPos(hwnd);}}
    break; */
    
    case WM_PAINT: {
      PAINTSTRUCT ps;
      BeginPaint(hwnd, &ps);
      BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right-ps.rcPaint.left, ps.rcPaint.bottom-ps.rcPaint.top, memdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
      EndPaint(hwnd, &ps);
      break;
    }
    
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

/* void toggleFullscreen(HWND hwnd) {
  if (!fullscreen) {
    fullscreen = 1;
    vp.screen_width = 1920;
    vp.screen_height = 1080;
    SetWindowLongPtr(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
    MoveWindow(hwnd, 0, 0, vp.screen_width, vp.screen_height, TRUE);
  } else {
    fullscreen = 0;
    vp.screen_width = 896;
    vp.screen_height = 768;
    SetWindowLongPtr(hwnd, GWL_STYLE, WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE | WS_MINIMIZEBOX);
    MoveWindow(hwnd, 500, 100, windows_width, windows_height, TRUE);
  }
  DeleteObject(bmp);
  bmp = initscreen(hdc, vp.screen_width, vp.screen_height,  &screen);
  SelectObject(memdc, bmp);
  initm();
  draw();
  BitBlt(hdc, 0, 0, vp.screen_width, vp.screen_height, memdc, 0, 0, SRCCOPY);
} */

HBITMAP initscreen(HDC hdc, int w, int h, u16 **data) {
  BITMAPINFO *bmi;
  int binfo_size = sizeof(*bmi);
  binfo_size += 3 * sizeof(DWORD); // For RGB masks
  bmi = calloc(1, binfo_size);
  
  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = w;
  bmi->bmiHeader.biHeight = -h;
  bmi->bmiHeader.biSizeImage = (w * 2) * h;
  bmi->bmiHeader.biPlanes = 1;
  bmi->bmiHeader.biBitCount = 16;
  bmi->bmiHeader.biCompression = BI_BITFIELDS; //BI_RGB;
  bmi->bmiHeader.biClrUsed = 0;
  ((DWORD *)bmi->bmiColors)[0] = 0x001F; // r
  ((DWORD *)bmi->bmiColors)[1] = 0x03E0; // g
  ((DWORD *)bmi->bmiColors)[2] = 0x7C00; // b
  
  HBITMAP bmp;
  bmp = CreateDIBSection(hdc, bmi, DIB_RGB_COLORS, (void**)data, NULL, 0);
  free(bmi);
  return bmp;
}