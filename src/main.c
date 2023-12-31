
#define FRAME_IMPLEMENTATION
#include "frame.h"

// For decoding

#define PNM_IMPLEMENTATION
#include "pnm.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define QOI_IMPLEMENTATION
#include "qoi.h"

#define PADDING 48
#define BORDER_PADDING 4

static Frame frame;
float zoom = 1.f;
float initial_zoom = 1.f;
bool show_border = false;

float y_off = 0.f;
float y_start = 0.f;
bool y_drag = false;

float x_off = 0.f;
float x_start = 0.f;
bool x_drag = false;

unsigned int tex;
const char *last_path = NULL;
int img_width, img_height;

void load_file(const char *path) {

  qoi_desc desc;	
  unsigned char *data = NULL;
  
  data = qoi_read(path, &desc, 4);
  img_width = desc.width;
  img_height = desc.height;
  if(!data) {
    data = pnm_load(path, &img_width, &img_height, NULL, 4);
  }
  if(!data) {
    data = stbi_load(path, &img_width, &img_height, 0, 4);
  }  
  if(!data) {
    fprintf(stderr, "ERROR: Can not open '%s'\n", path); fflush(stderr);
    return; 
  }
  
  last_path = path;
  frame_set_title(&frame, path);
  
  frame_renderer.images_count = 0;
  frame_renderer_push_texture(img_width, img_height, data, false, &tex);

  free(data); // all libs use 'free'

  if(img_width > img_height) {
    zoom = ((float) frame.width - 2 * PADDING) / (float) img_width;
  } else {
    zoom = ((float) frame.height - 2 * PADDING) / (float) img_height;
  }
  y_off = 0.f;
  x_off = 0.f;
  initial_zoom = zoom;
}

int main(int argc, char **argv) {
 
  if(!frame_init(&frame, 800, 800, argv[0], FRAME_DRAG_N_DROP)) {
    return 1;
  }

  if(argc > 1) {
    load_file(argv[1]);
  }

  ///////////////////////////////////////////////////////////////////////////
  

  float DT = 1000.f / 60.f;
  float last_click = 0.f;
  Frame_Event event;
  while(frame.running) {
    Vec2f mouse;
    frame_get_mouse_position(&frame, &mouse.x, &mouse.y);
      
    while(frame_peek(&frame, &event)) {

      switch(event.type) {

      case FRAME_EVENT_MOUSEWHEEL: {
	zoom += 0.025f * (float) event.as.amount;
      } break;

      case FRAME_EVENT_MOUSEPRESS: {
	if(last_click > 0.0f) {
	  frame_toggle_fullscreen(&frame);
	  last_click = 0.f;
	} else {
	  last_click = FRAME_DOUBLE_CLICK_TIME_MS;
	  y_start = mouse.y - y_off;
	  y_drag = true;
	  x_start = mouse.x - x_off;
	  x_drag = true;
	}
      } break;

      case FRAME_EVENT_MOUSERELEASE: {
	y_drag = false;
	x_drag = false;
      } break;
      case FRAME_EVENT_KEYPRESS: {

	switch(event.as.key) {
	case 'b': {
	  show_border = !show_border;
	} break;
	case 'r': {
	  zoom = initial_zoom;
	  x_off = 0.f;
	  y_off = 0.f;
	} break;
	case 'q': {
	  frame.running = false;
	} break;
	case 'f':{
	  frame_toggle_fullscreen(&frame);
	} break;
	case FRAME_ESCAPE: {
	  if(frame.running & FRAME_FULLSCREEN) {
	    frame_toggle_fullscreen(&frame);
	  }
	} break;
	}
	
      }break;
	
      case FRAME_EVENT_FILEDROP: {

	Frame_Dragged_Files files;
	if(frame_dragged_files_init(&files, &event)) {
	  char *path;
	  if(frame_dragged_files_next(&files, &path)) {
	    load_file(path);
	  }

	  frame_dragged_files_free(&files);
	}

      } break;
      default: {	
      } break;
      }
      
    }

    if(y_drag) {
      y_off = mouse.y - y_start;
    }
    if(x_drag) {
      x_off = mouse.x - x_start;
    }

    float target_width = (float) img_width * zoom;
    float target_height = (float) img_height * zoom;    

    if(last_path != NULL) {
      float ratio =  (float) img_width / (float) img_height;
      (void) ratio;

      Vec2f pos = vec2f((float) frame.width/2 - target_width/2 + x_off,
			(float) frame.height/2 - target_height/2 + y_off);
      Vec2f size = vec2f(target_width, target_height);

      if(show_border) {
	frame_renderer_solid_rect(vec2f(pos.x - BORDER_PADDING,
					pos.y - BORDER_PADDING),
				  vec2f(size.x + 2 * BORDER_PADDING,
					size.y + 2 * BORDER_PADDING),
				  WHITE);     	
      }
      
      frame_renderer_texture(tex, pos, size, vec2f(0, 0), vec2f(1, 1));   
    }

    last_click -= DT;
    frame_swap_buffers(&frame);    
  }

  frame_free(&frame);
  
  return 0;
}
