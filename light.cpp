#include<iostream>
#include<cstdio>
#include"SDL2/SDL.h"
#include"SDL2/SDL_image.h"
#include"cleanup.h"
#include<cmath>
using namespace std;

//Screen Size
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

/*
 * Log an SDL error with some error message
 * @param os The output stream to write the message too
 * @param msg The error message to write, format will be msg error: SDL_GetError()
 */
void logSDLError(ostream &os, const string &msg){
	os << msg << "Error: " << SDL_GetError() << endl;
}

/*
 * Log an SDL_image error with some error message
 * @param os The output stream to write the message too
 * @param msg The error message to write, format will be msg error: IMG_GetError()
 */
void logImageError(ostream &os, const string &msg){
	os << msg << "Error: " << IMG_GetError() << endl;
}
/*
 * Loads an image into a texture on the rendering device
 * @param file The image file to load
 * @param ren The renderer to load the texture
 * @return the loaded texture, or nullptr if something went wrong.
 */
SDL_Texture* loadTexture(const string &filename, SDL_Renderer *ren){
	SDL_Texture *texture = IMG_LoadTexture(ren, filename.c_str());
	if(texture == NULL){
		logImageError(cerr, "loadTexture");
	}
	return texture;
}
/*
 * Loads an image as a surface
 * @param file The image file to load
 * @return the loaded surface, or nullptr if something went wrong.
 */
SDL_Surface* loadSurface(const string &filename){
	SDL_Surface *sur = IMG_Load(filename.c_str());
	if(sur == NULL){
		logImageError(cerr, "loadsurface");
	}
	return sur;
}

/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, with desired
 * width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 * @param w The width of the texture to draw
 * @param h The height of the texture to draw
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
	//Setup the destination rectangle to be at the position we want	
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = w;
	dest.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dest);
}
/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
 * the texture's width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x,int y){
	int w,h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	renderTexture(tex, ren, x, y, w, h);
}

bool inRect(int x, int y, int width, int height, int point_x, int point_y){
	return point_x >= x && point_y >= y && point_x <= x+width && point_y <= y + height;
}
int main(int argc, char** argv){
	SDL_Window *window = NULL;
	SDL_Renderer *ren = NULL;
	SDL_Texture *switch_on, *switch_off, *switch_back_on, *switch_back_off;
	SDL_Texture *background_on, *background_off;
	switch_off = switch_on = switch_back_off = switch_back_on = background_on = background_off = NULL;
	SDL_Surface *mouse_surface = NULL;
	SDL_Cursor *mouse_cursor_new = NULL;
	SDL_Cursor *mouse_cursor_system = NULL;
	do{
		//Init the Video subsystem
		if(SDL_Init(SDL_INIT_VIDEO) != 0){
			logSDLError(cerr, "SDL_Init");
			break;
		}
		//Apply a window
		window = SDL_CreateWindow("Light", SDL_WINDOWPOS_CENTERED,
						SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if(window == NULL){
			logSDLError(cerr, "CreateWindow");
			break;
		}
		//Apply a render
		ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if(ren == NULL){
			logSDLError(cerr, "CreateRenderer");
			break;
		}
		//load images
		switch_on = loadTexture("image/switch_on.png", ren);
		switch_off = loadTexture("image/switch_off.png", ren);
		switch_back_on = loadTexture("image/switch_back_on.png", ren);
		switch_back_off = loadTexture("image/switch_back_off.png", ren);
		background_on = loadTexture("image/background_on.png", ren);
		background_off = loadTexture("image/background_off.png", ren);
		mouse_surface = loadSurface("image/mouse.png");
		mouse_cursor_new = SDL_CreateColorCursor(mouse_surface, 15, 0);
		mouse_cursor_system= SDL_GetCursor();
		if(mouse_cursor_system == NULL || mouse_surface == NULL || mouse_cursor_new == NULL || switch_on == NULL ||switch_off == NULL || switch_back_on == NULL || switch_back_off == NULL || background_on == NULL || background_off == NULL){
			break;
		}
		//Query the size of light then caculate the appropriate position of it
		int switch_back_width, switch_back_height, switch_back_x, switch_back_y;
		//Query the size of the background of switch then caculate the appropriate position of it
		SDL_QueryTexture(switch_back_off, NULL, NULL, &switch_back_width, &switch_back_height);
		switch_back_x = SCREEN_WIDTH / 2 - switch_back_width / 2;
		switch_back_y = SCREEN_HEIGHT * 0.618 - switch_back_height / 2 + 60;
		int switch_width, switch_height, switch_x, switch_y;
		//Query the size of the switch
		SDL_QueryTexture(switch_on, NULL, NULL, &switch_width, &switch_height);
		switch_x = switch_back_x;
		switch_y = switch_back_y;
		//Render the picture to the window
		SDL_RenderClear(ren);
		renderTexture(background_off, ren, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		renderTexture(switch_back_off, ren, switch_back_x, switch_back_y, switch_back_width, switch_back_height);
		renderTexture(switch_off, ren, switch_x, switch_y, switch_width, switch_height);
		
		SDL_RenderPresent(ren);
		int light_state = -1;//Init the light is off
		bool mouse_motion = false;//Move the mouse or not
		SDL_Event e;
		int mouse_x, mouse_y, mouse_xrel, mouse_state;
		bool quit = false;
		//Read any events that occured
		while(!quit){
			while(SDL_PollEvent(&e)){
				if (e.type == SDL_QUIT){
					quit = true;
				}
				int cur_mouse_x, cur_mouse_y;
				SDL_GetMouseState(&cur_mouse_x, &cur_mouse_y);
				if(inRect(switch_x, switch_y, switch_width, switch_height, cur_mouse_x, cur_mouse_y)){
					SDL_SetCursor(mouse_cursor_new);
				}else{
					SDL_SetCursor(mouse_cursor_system);
				}
				if ( e.type == SDL_MOUSEMOTION){
					//Judge the Mouse is drag the switch or not
					if(SDL_BUTTON(SDL_BUTTON_LEFT) & SDL_GetMouseState(NULL, NULL)){
						mouse_x = e.motion.x;
						mouse_y = e.motion.y;
						mouse_xrel = e.motion.xrel;
						mouse_motion = true;
						break;
					}
				}
			}
			//Mouse is drag the switch
			if(mouse_motion && inRect(switch_x, switch_y, switch_width, switch_height, mouse_x,mouse_y)){
				switch_x += mouse_xrel;
				if(switch_x < switch_back_x){
					switch_x = switch_back_x;
				}
				if(switch_x > (switch_back_x + switch_back_width - switch_width)){
					switch_x = switch_back_x + switch_back_width - switch_width;	
				}
				//Init for next circle
				mouse_motion = false;
				//Mouse is not point the switch then move the switch to the longer side
			}else if(!(SDL_BUTTON(SDL_BUTTON_LEFT) & SDL_GetMouseState(NULL, NULL))) {
				int change_x = (switch_x + switch_width / 2 - (switch_back_x + switch_back_width / 2));
				//break the balance
				if(change_x == 0){
					switch_x += mouse_xrel;
				}else{
					switch_x += change_x;
				}
				if(switch_x < switch_back_x){
					switch_x = switch_back_x;
				}
				if(switch_x > (switch_back_x + switch_back_width - switch_width)){
					switch_x = switch_back_x + switch_back_width - switch_width;	
				}
			}
			//Judge the status of light
			if(fabs(switch_x + switch_width - switch_back_x - switch_back_width) <= 1e-6 ){
				//light is on
				light_state = 1;
			}else if(fabs(switch_x - switch_back_x) <= 1e-6){
				//light is off
				light_state =-1;
			}
			SDL_RenderClear(ren);
			//ReRender the images
			if(light_state == -1){
				renderTexture(background_off, ren, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				renderTexture(switch_back_off, ren, switch_back_x, switch_back_y, switch_back_width, switch_back_height);
				renderTexture(switch_off, ren, switch_x, switch_y, switch_width, switch_height);
			}else{
				renderTexture(background_on, ren, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				renderTexture(switch_back_on, ren, switch_back_x, switch_back_y, switch_back_width, switch_back_height);
				renderTexture(switch_on, ren, switch_x, switch_y, switch_width, switch_height);
			}
			SDL_RenderPresent(ren);
		}
		if(mouse_cursor_system != NULL){
			SDL_FreeCursor(mouse_cursor_system);
		}
		if(mouse_surface != NULL){
			SDL_FreeSurface(mouse_surface);
		}
		if(mouse_cursor_new != NULL){
			SDL_FreeCursor(mouse_cursor_system);
		}
		cleanup(background_on, background_off, switch_on, switch_off, switch_back_on, switch_back_off, ren, window);
		return 0;

	}while(0);
	if(mouse_cursor_system != NULL){
		SDL_FreeCursor(mouse_cursor_system);
	}
	if(mouse_surface != NULL){
		SDL_FreeSurface(mouse_surface);
	}
	if(mouse_cursor_new != NULL){
		SDL_FreeCursor(mouse_cursor_system);
	}
	cleanup(background_on, background_off, switch_on, switch_off, switch_back_on, switch_back_off, ren, window);
	return 1;
}
