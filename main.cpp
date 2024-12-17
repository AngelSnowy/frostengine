#include <X11/X.h>
#include <iomanip>
#define FILL_COLOR 0xAA22BB
#define BORDER_COLOR 0x000000
#define FILE_TO_IMPORT "keyboard.obj"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <vector>
#include "math.h"
#include <sstream>
#include <fstream>
#include <strstream>
#include <algorithm>
#include <chrono>
#include <thread>

#include "utility.h"

void multiply_matrix_vector(vec3d &i, vec3d &o, mat4x4 &m) {

  o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
  o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
  o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];

  float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];
  
  if (w != 0.0f) {

    o.x /= w; o.y /= w; o.z /= w;

  }

  return;
}

class XlibApp {
  
public:
  
  int sgn(int x)
  {
    if (x > 0) return +1;
    if (x < 0) return -1;
    return 0;
  }

  //this is a nightmare
  void draw_bresenham_line(unsigned int start_x, unsigned int start_y, unsigned int end_x, unsigned int end_y, unsigned long color) {

    XSetForeground(display,gc,color);
    
    int x, y, t, dx, dy, incx, incy, pdx, pdy, ddx, ddy, deltaslowdirection, deltafastdirection, err;
    
    dx = end_x - start_x;
    dy = end_y - start_y;
    
    incx = sgn(dx);
    incy = sgn(dy);

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    
    if (dx > dy) {
      
        pdx = incx; pdy = 0;
        ddx = incx; ddy = incy;
        deltaslowdirection = dy;
	deltafastdirection = dx;

    } else {

      pdx = 0;    pdy = incy;
      ddx = incx; ddy = incy;
      deltaslowdirection = dx;
      deltafastdirection = dy;   

    }
    
    x = start_x;
    y = start_y;
    err = deltafastdirection / 2;

    XDrawPoint(display,window,gc,x,y);
    
    for(t = 0; t < deltafastdirection; ++t)  {

      err -= deltaslowdirection;

      if(err < 0) {

	err += deltafastdirection;
	x += ddx;
	y += ddy;

      } else {

	x += pdx;
	y += pdy;

      }

      XDrawPoint(display,window,gc,x,y);

    }
  }

  // i mog nimma :(
  void draw_filled_in_triangle (unsigned int x1, unsigned int x2, unsigned int x3, unsigned int y1, unsigned int y2, unsigned int y3 ,unsigned long color) {

    float fx1 = x1;
    float fx2 = x2;
    float fx3 = x3;
    float fy1 = y1;
    float fy2 = y2;
    float fy3 = y3;
    
    std::cout << "drawing filled tri" << std::endl;
    
    unsigned int s_buf;
    
    if(fy1 > fy2) { s_buf = fy1; fy1 = fy2 ; fy2 = s_buf; }
    if(fy2 > fy3) { s_buf = fy2; fy2 = fy3 ; fy3 = s_buf; }
    
    if (fy2 == fy1)
      {
	std::cout << "bottom triangle! " << std::endl;
	
	fill_bottom_triangle(fx1,fy1,fx2,fy2,fx3,fy3,color);
      }
    
    if (fy2 == fy3)
      {
	std::cout << "top triangle! " << std::endl;
		
	fill_top_triangle(fx1,fy1,fx2,fy2,fx3,fy3,color);
      }
    
    float fx4,fy4;

    //    if(x1 == 0.0f || x2 == 0.0f || x3 == 0.0f || y1 == 0.0f || y2 == 0.0f || y3 == 0.0f) 
    
    fx4 = (fx1 + ((float)(fy2 - fy1) / (float)(fy3 - fy1)) * (fx3 - fx1));
    fy4 = fy2;

    fill_bottom_triangle(fx1,fy1,fx4,fy4,fx3,fy3,color);
    fill_top_triangle(fx1,fy1,fx4,fy4,fx3,fy3,color);
    
  }

  void draw_horizontal_line(unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2, unsigned long color) {

    XSetForeground(display,gc,color);
    
    XDrawLine(display,window,gc,x1,y1,x2,y2);

  }

  mesh import_obj_mesh(std::string file_path) {

    mesh output;
    
    std::ifstream file(file_path);
    if(!file.is_open())
      std::cout << "imort mesh not found!!!" << std::endl;

    std::vector<vec3d> vertices;
    
    while(!file.eof()) {

      char line[1024];
      file.getline(line,1024);

      std::strstream s;

      s << line;

      char junk;

      if(line[0] == 'v') {

	vec3d vec;

	s >> junk >> vec.x >> vec.y >> vec.z;

        vertices.push_back(vec);

      }
      
      if(line[0] == 'f') {

	int f[3];

	s >> junk >> f[0] >> f[1] >> f[2];
	
	output.tris.push_back({ vertices[f[0] - 1], vertices[f[1] - 1], vertices[f[2] - 1] , 0x000000 });

      }
      
    }
    
    return output;

  }

  void try_to_render_screen() {    

    try_to_draw = true;

  }

  void window_runtime_helper() {

    printf("runtime helper start! \n");
    
    while(!shutdown) {      
      
      if (try_move_foreward) {
	
	vec3d cam_foreward = vector_Mul(look_dir, 0.02f * z_pos);
	
	camera = vector_Add(camera, cam_foreward);	
      }
      
      if (try_move_backward) {
	
	vec3d cam_foreward = vector_Mul(look_dir, 0.02f * z_pos);
	
	camera = vector_Sub(camera, cam_foreward);	
      }

      if (try_move_right) {
	
	vec3d cam_foreward_step = vector_Mul(look_dir, 0.04f);
	
	vec3d cam_foreward = vector_Mul(cam_foreward_step,x_pos);
	
	vec3d cam_sideward = cam_foreward;
	cam_sideward.y = 0.0f;
	
	vec3d cam_modified;
	
	cam_modified.x = cam_sideward.z;
	cam_modified.z = cam_sideward.x * -1.0f;
	cam_modified.y = cam_foreward.y;
	
	camera = vector_Sub(camera, cam_modified);
	
	x_pos -= 0.01f;	 	
      }

      if (try_move_left) {
	
	vec3d cam_foreward = vector_Mul(look_dir, 0.04 *  x_pos);
	
	vec3d cam_sideward = cam_foreward;
	cam_sideward.y = 0.0f;
	
	vec3d cam_modified;
	
	cam_modified.x = cam_sideward.z * -1.0f;
	cam_modified.z = cam_sideward.x;
	cam_modified.y = cam_foreward.y;
	
	camera = vector_Sub(camera, cam_modified);
	
	x_pos += 0.01f;	 	  	
      }
      
      if (try_move_down) 	
	camera.y -= 0.05f;	 
	      
      if (try_move_up) 	
	camera.y += 0.05f;	 
	     
      if (try_rotate_right) 	
	cam_yaw += change_rate * 0.05f;	 
	
      if (try_rotate_left) 	
	cam_yaw -= change_rate * 0.05f;	 
        
      if(draw_cooldown) {
	
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
	draw_cooldown = false;
	printf("no \n");
	
      }

      render_screen();
      
      if(!draw_cooldown && try_to_draw) {

        draw_cooldown = true;
	try_to_draw = false;
	printf("yes \n");
	
	render_screen();
	
      } else {

	printf("rendering attempt started, but rendering on cooldown! \n");
	
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      
    }

    return;

  }
  
  void tmp_draw_filled_tri(unsigned int v1x, unsigned int v2x, unsigned int v3x, unsigned int v1y, unsigned int v2y, unsigned int v3y, unsigned long color) {

     XSetForeground(display,gc,color);
    
    XPoint triangle[3];
    triangle[0].x = v1x; triangle[0].y = v1y;
    triangle[1].x = v2x; triangle[1].y = v2y;
    triangle[2].x = v3x; triangle[2].y = v3y;
    
    XFillPolygon(display,window,gc, triangle, 3, 2, 0);

  }
  
  void fill_top_triangle(unsigned int v1x, unsigned int v1y, unsigned int v2x, unsigned int v2y, unsigned int v3x, unsigned int v3y,unsigned int color) {

    float fv1x = (float) v1x;
    float fv1y = (float) v1y;
    float fv2x = (float) v2x;
    float fv2y = (float) v2y;    
    float fv3x = (float) v3x;
    float fv3y = (float) v3y;
    
    float invslope1,invslope2;
    
    if (fv3y != fv1y) {
      invslope1 = (fv3x - fv1x) / (fv3y - fv1y);
    } else {
      fv1y = fv1y + 0.1f;
      invslope1 = (fv3x - fv1x) / (fv3y - fv1y);
    }
    
    if (fv3y != fv2y) {
      invslope2 = (fv3x - fv2x) / (fv3y - fv2y);
    } else {
      fv1y = fv1y + 0.1f;
      invslope2 = (fv3x - fv2x) / (fv3y - fv2y);
    }
        
    float curx1 = fv3x;
    float curx2 = fv3x;
    
    for (int scanlineY = fv3y; scanlineY > fv1y; scanlineY--)
      {
	
	draw_horizontal_line((unsigned int)curx1, (unsigned int)scanlineY, (unsigned int)curx2, (unsigned int)scanlineY, color);
	curx1 -= invslope1;
	curx2 -= invslope2;
	
      }
  }
  
  void fill_bottom_triangle(unsigned int v1x,unsigned int v1y,unsigned int v2x,unsigned int v2y,unsigned int v3x,unsigned int v3y,unsigned int color) {

    float fv1x = (float) v1x;
    float fv1y = (float) v1y;
    float fv2x = (float) v2x;
    float fv2y = (float) v2y;    
    float fv3x = (float) v3x;
    float fv3y = (float) v3y;

    float invslope1,invslope2;
    if(fv2y != fv1y) {
      invslope1 = (fv2x - fv1x) / (fv2y - fv1y);
    } else {
      fv1y = fv1y + 0.1f;
      invslope1 = (fv2x - fv1x) / (fv2y - fv1y);
    }

    if(fv3y !=fv1y) {
      invslope2 = (fv3x - fv1x) / (fv3y - fv1y);
    } else {
      fv1y = fv1y + 0.1f;
      invslope2 = (fv3x - fv1x) / (fv3y - fv1y);
    }
    
    float curx1 = fv1x;
    float curx2 = fv1x;
    
    for (int scanlineY = fv1y; scanlineY <= fv2y; scanlineY++)
      {	
	draw_horizontal_line((unsigned int)curx1, (unsigned int)scanlineY, (unsigned int)curx2, (unsigned int)scanlineY, color);
	curx1 += invslope1;
	curx2 += invslope2;
      }
  }
  
  XlibApp(int width, int height) : width(width), height(height) {
    display = XOpenDisplay(nullptr);
    if (display == nullptr) {
      std::cerr << "Cannot open display" << std::endl;
      exit(1);
    }
    
    root = DefaultRootWindow(display);
    
    screen = DefaultScreen(display);

    window = XCreateSimpleWindow(display,
				 root,
				 10,
				 10,
				 width,
				 height,
				 1,				 
				 BlackPixel(display, screen),
				 WhitePixel(display, screen)
				 );
    
    XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask);
    XMapWindow(display, window);
    
    gc = XCreateGC(display, window, 0, nullptr);

    loaded_mesh = import_obj_mesh(FILE_TO_IMPORT);
    
  }
  
  ~XlibApp() {
    shutdown = true;
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
  }

  
  unsigned long construct_rgb_value(unsigned long r, unsigned long g, unsigned long b) {
    return (
	    static_cast<unsigned int>(r) << 16) | 
      (static_cast<unsigned int>(g) << 8) | 
      (static_cast<unsigned int>(b));
  }
  
  unsigned long float_to_rgb_grayscale(float input) {

    unsigned long output;

    unsigned long tmp;
    
    tmp = ((255) / 1.0f) * (input);
      
    output = construct_rgb_value(tmp,tmp,tmp);
   
    return output;

  }
  
  void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    
    XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
    
    draw_bresenham_line(x1, y1, x2, y2, BORDER_COLOR);
    draw_bresenham_line(x2, y2, x3, y3, BORDER_COLOR); 
    draw_bresenham_line(x3, y3, x1, y1, BORDER_COLOR);
    XFlush(display);
  }
  
  void draw_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    
    Colormap colormap = DefaultColormap(display, screen);
    XColor color;
    color.red = (r << 8);
    color.green = (g << 8);
    color.blue = (b << 8);
    XAllocColor(display, colormap, &color);
    
    XSetForeground(display, gc, color.pixel);
    XDrawPoint(display, window, gc, x, y);
    XFlush(display);
    
  }
  
  void render_screen() {

    draw_cooldown = true;
    
    int x, y; 
    unsigned int border_width, depth;
    
    if (XGetGeometry(display, window, &root, &x, &y, &width, &height, &border_width, &depth)) {
      std::cout << "Window size: " << width << "x" << height << std::endl;
    } else {
      std::cerr << "Failed to get window geometry" << std::endl;
    }
    
    XSetForeground(display, gc , 0x000000);
    XFillRectangle(display, window, gc , 0, 0, width, height);

    std::vector<triangle> v_want_to_draw;

    matRotZ = matrix_make_rotate_z(fThetaZ);
    matRotX = matrix_make_rotate_x(fThetaX);
    matRotY = matrix_make_rotate_y(fThetaY);

    matProj = matrix_make_projection(fFov, (float)height / (float)width, 0.1f, 100.0f);

    //tjmat
    
    //x y z
    matTrans = matrix_make_translate(5.0f,3.0f,10.0f);

    //matrix allocation
    matWorld = matrix_make_static_identity();
    matWorld = matrix_mul_matrix(matWorld,matRotX);
    matWorld = matrix_mul_matrix(matWorld,matRotY);
    matWorld = matrix_mul_matrix(matWorld,matRotZ);
    matWorld = matrix_mul_matrix(matWorld,matTrans);

    vec3d up = {0,1,0};
    vec3d target = {0,0,1};
    mat4x4 cam_rot = matrix_make_rotate_y(cam_yaw);
    look_dir = matrix_multiply_vector(cam_rot, target);
    target = vector_Add(camera, look_dir);
    mat4x4 camera_mat = matrix_PointAt(camera, target, up);

    mat4x4 view_mat = matrix_QuickInverse(camera_mat);
    
    for(auto tri : loaded_mesh.tris) {
      
      triangle triProjected;
      triangle triTransformed;
      triangle triViewed;

      triTransformed.p[0] = matrix_multiply_vector(matWorld,tri.p[0]);
      triTransformed.p[1] = matrix_multiply_vector(matWorld,tri.p[1]);
      triTransformed.p[2] = matrix_multiply_vector(matWorld,tri.p[2]);
      
      vec3d normal, vec1, vec2;
 
      //vecs that span surface
      vec1 = vector_Sub(triTransformed.p[1], triTransformed.p[0]);
      vec2 = vector_Sub(triTransformed.p[2], triTransformed.p[0]);

      // normal of surface
      normal = vector_CrossProduct(vec1,vec2);
      normal = vector_Normalise(normal);

      vec3d camera_ray = vector_Sub(triTransformed.p[0], camera);
	
	if( vector_DotProduct(normal, camera_ray) < 0.0f ) {
	  
	  vec3d light_source = { 0.0f, 0.0f, -1.0f };
	  light_source = vector_Normalise(light_source);
	  float dp_light = normal.x * light_source.x + normal.y * light_source.y + normal.z * light_source.z;

	  triTransformed.col_rgb  = float_to_rgb_grayscale( dp_light );

	  // world -> view
	  triViewed.p[0] = matrix_multiply_vector(view_mat, triTransformed.p[0]);
	  triViewed.p[1] = matrix_multiply_vector(view_mat, triTransformed.p[1]);
	  triViewed.p[2] = matrix_multiply_vector(view_mat, triTransformed.p[2]);

	  // proj to 2d
	  triProjected.p[0] = matrix_multiply_vector(matProj, triViewed.p[0]);
	  triProjected.p[1] = matrix_multiply_vector(matProj, triViewed.p[1]);
	  triProjected.p[2] = matrix_multiply_vector(matProj, triViewed.p[2]);

	  triProjected.col_rgb = triTransformed.col_rgb;
	  // TJtrans
	  triProjected.p[0] = vector_Div(triProjected.p[0], triProjected.p[0].w);
	  triProjected.p[1] = vector_Div(triProjected.p[1], triProjected.p[1].w);
	  triProjected.p[2] = vector_Div(triProjected.p[2], triProjected.p[2].w);

	  offset_view = {1,1,0};
	  triProjected.p[0] = vector_Add(triProjected.p[0], offset_view);
	  triProjected.p[1] = vector_Add(triProjected.p[1], offset_view);
	  triProjected.p[2] = vector_Add(triProjected.p[2], offset_view);
	  
	  triProjected.p[0].x *= 0.5f * (float)width;
	  triProjected.p[0].y *= 0.5f * (float)height;
	  
	  triProjected.p[1].x *= 0.5f * (float)width;
	  triProjected.p[1].y *= 0.5f * (float)height;
	  
	  triProjected.p[2].x *= 0.5f * (float)width;
	  triProjected.p[2].y *= 0.5f * (float)height;

	  v_want_to_draw.push_back(triProjected);
	  
	}      
    }

    //shoutout to stdlib
    sort(v_want_to_draw.begin(), v_want_to_draw.end(), [](triangle &t1, triangle &t2) {

      float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
      float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;

      return z1 > z2;

    });
    
    for(auto &triProjected : v_want_to_draw) {

      tmp_draw_filled_tri((unsigned int)triProjected.p[0].x,
			  (unsigned int)triProjected.p[1].x,
			  (unsigned int)triProjected.p[2].x,
			  (unsigned int)triProjected.p[0].y,
			  (unsigned int)triProjected.p[1].y,
			  (unsigned int)triProjected.p[2].y,
			  triProjected.col_rgb);
      
      if(wireframe_active){
	draw_triangle(triProjected.p[0].x, triProjected.p[0].y,
		      triProjected.p[1].x, triProjected.p[1].y,
		      triProjected.p[2].x, triProjected.p[2].y);
      }      
      
    }
    
    
    XFlush(display);

    draw_cooldown = false;
    
  }    
  
  void run() {

    printf("launching draw helper! \n");
    
    std::thread t(&XlibApp::window_runtime_helper,this);
    
    camera.x = 0.0f;
    camera.y = 0.0f;
    camera.z = 0.0f;
    
    XEvent event;
    while (true) {

      XNextEvent(display, &event);

      if (event.type == Expose) {
	
	int x, y; 
	unsigned int border_width, depth;
	
	if (XGetGeometry(display, window, &root, &x, &y, &width, &height, &border_width, &depth)) {
	  std::cout << "Window size: " << width << "x" << height << std::endl;
	} else {
	  std::cerr << "Failed to get window geometry" << std::endl;
	}
	
        try_to_render_screen();
	
      }
      
      if (event.type == KeyPress) {
	KeySym key = XLookupKeysym(&event.xkey, 0);

	// world rotation
	if (key == XK_h) {
	  
	  fThetaX += change_rate;     
	  
	} else if (key == XK_k) {
	  
	  fThetaY -= change_rate;	  
	  
	} else if (key == XK_l) {
	  
	  fThetaX -= change_rate;	  
	  
	} else if (key == XK_j) {
	  
	  fThetaY += change_rate;	  
	  
	} else if (key == XK_u) {
	  
	  fThetaZ += change_rate;	  
	  
	} else if (key == XK_i) {
	  
	  fThetaZ -= change_rate;	 
	  
	} else if (key == XK_f) {
	  
	  wireframe_active = !wireframe_active;
	  
	}
	
	// camera navigation input

	if (key == XK_w)
	  try_move_foreward = true;

	if (key == XK_s)
	  try_move_backward = true;

	if (key == XK_a)
	  try_move_right = true;

	if (key == XK_d)
	  try_move_left = true;

	if (key == XK_c)
	  try_rotate_right = true;

	if (key == XK_z)
	  try_rotate_left = true;

	if (key == XK_e)
	  try_move_up = true;

	if (key == XK_q)
	  try_move_down = true;

	//extras

	if (key == XK_t) {
	  
	  fFov += 5.0f;
	  //fFovRad = 1.0f / tanf( fFov * 0.5f / 180.0f * 3.14159f );
	  fFovRad += 0.1f;
	}
	
	if (key == XK_g) {
	  
	  fFov -= 5.0f;
	  //fFovRad = 1.0f / tanf( fFov * 0.5f / 180.0f * 3.14159f );
	  fFovRad -= 0.1f;
	}
	
	//quit
	if (key == XK_x) {
	  break; 
	}
	
        try_to_render_screen();
	
      }

      if(event.type == KeyRelease) {
	
	KeySym key = XLookupKeysym(&event.xkey, 0);

	if (key == XK_w)
	  try_move_foreward = false;
	if (key == XK_s)
	  try_move_backward = false;
	if (key == XK_a)
	  try_move_left = false;
	if (key == XK_d)
	  try_move_right = false;
	if (key == XK_c)
	  try_rotate_right = false;
	if (key == XK_z)
	  try_rotate_left = false;
	if (key == XK_e)
	  try_move_up = false;
	if (key == XK_q)
	  try_move_down = false;


      }
				      
      
    }
    
  }
  
  
private:

  float change_rate = 0.15f;
  
  bool wireframe_active = true;

  //X11
  Display* display;
  Window window;
  GC gc;
  Window root;
  int screen;
  
  unsigned int width;
  unsigned int height;

  //Renderer Data
  float fThetaX = 0.0f;
  float fThetaY = 0.0f;
  float fThetaZ = 0.0f;

  float fPosition_X;
  float fPosition_Y;
  float fPosition_Z;
  
  mesh loaded_mesh;

  float z_pos = 1.0f;
  float x_pos = 1.0f;
  
  float fNear = 0.1f;
  float fFar = 1000.0f;
  float fFov = 100.0f;

  vec3d camera;
  vec3d look_dir;
  vec3d offset_view;
  float cam_yaw;
  
  mat4x4 matProj;
  mat4x4 matRotZ;
  mat4x4 matRotX;
  mat4x4 matRotY;

  mat4x4 matTrans;
  mat4x4 matWorld;
  
  float fAspectRatio = (float)height / (float)width;
  float fFovRad = 1.72123f;

  //Optimization 
  bool try_to_draw;
  bool draw_cooldown = false;
  bool shutdown = false;

  //Movement helpers (i know its shit impl but its x11 so theres no better way )

  bool try_move_foreward = false;
  bool try_move_backward = false;
  bool try_move_left = false;
  bool try_move_right = false;
  bool try_move_up = false;
  bool try_move_down = false;
  bool try_rotate_left = false;
  bool try_rotate_right = false;
  
};

int main() {
    XlibApp app(800, 600);
    app.run();
    return 0;
}
