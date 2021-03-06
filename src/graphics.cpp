//__________________________________________________________________________
//      _________________   ____    ________    _____                       |
//     /   _____/\______ \ |    |   \_____  \  /     \   ____               |
//     \_____  \  |    |  \|    |    /  ____/ /  \ /  \_/ __ \              |
//     /        \ |    `   \    |___/       \/    Y    \  ___/              |
//    /_______  //_______  /_______ \_______ \____|__  /\___ \              |
//            \/         \/        \/       \/       \/     \/              |
//__________________________________________________________________________|
//
// Copyright © 2013 Miguel Vera Belmonte AKA MikiLoz92.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "graphics.h"

Graphics *gGraphics;
Options *gOptions;

Graphics *S2M_CreateGraphics() {
	gOptions = new Options("config.cfg");
	gGraphics = new Graphics(320,240,"S2MTEST", gOptions);
	return gGraphics;
} // Una vez ejecutemos S2M_CreateGraphics, los objetos gGraphics y gOptions serán globales
// y podrán ser accedidos desde cualquier parte del código sin necesidad de pasar su puntero, 
// puesto que su nombre no cambiará nunca, y podríamos programar toda la librería utilizando
// estas dos instancias por su nombre de pila; sólo haría falta incluir "graphics.h".

Graphics *S2M_CreateGraphics(int w, int h, string title) {
	gOptions = new Options("config.cfg");
	gGraphics = new Graphics(w, h, title, gOptions);
	return gGraphics;
}

void S2M_UpdateGraphics() {
	if (gGraphics) gGraphics->update();
	else {
		cerr << "Error: Trying to update game graphics but no Graphics instance created. Please use S2M_CreateGraphics to start the graphics engine." << endl;
		exit(EXIT_FAILURE);
	}
}

/*
float easeInExpo (float t,float b , float c, float d) {
	return (t==0) ? b : c * pow(2, 10 * (t/d - 1)) + b;
}

float easeOutExpo (float t, float b , float c, float d) {
	return (t==d) ? b+c : c * (-pow(2, -10 * t/d) + 1) + b;
}*/

Graphics::Graphics(int logWidth, int logHeight, string winCaption, Options * options) {
	SDL_Init(SDL_INIT_EVERYTHING);
	if( !SDL_SetHint( SDL_HINT_RENDER_VSYNC, "1" ) ) {
		cout << "Warning: VSync not enabled!" << endl;
	}
	if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "0" ) ) {
		cout << "Warning: Linear texture filtering enabled!" << endl << "Sprites will be blurry!";
	}
	window = SDL_CreateWindow( winCaption.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, logWidth * options->getScale(), logHeight * options->getScale(), SDL_WINDOW_SHOWN );
	//window = SDL_CreateWindow( winCaption.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP );
	if( window == NULL ) {
		cout << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
	}
	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if( renderer == NULL )	{
		cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
	}
	SDL_RenderSetLogicalSize(renderer, logWidth, logHeight);
	SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0x00 );
	int imgFlags = IMG_INIT_PNG;
	if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
		cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
	}
	gamewidth = logWidth;
	gameheight = logHeight;
	buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gamewidth, gameheight);
}

Graphics::~Graphics() {
	/**/ cout << "Cleaning Graphics" << endl;
	for (int i = 0; i < sprites.size(); i++) {
		delete sprites[i];
	}
	SDL_DestroyRenderer( renderer );
	SDL_DestroyWindow( window );
	IMG_Quit();
	SDL_Quit();
}

SDL_Texture* Graphics::loadTexture(string filename, bool drawable) {
	SDL_Texture* newTexture = NULL;
	SDL_Texture* newerTexture = NULL;
	SDL_Surface* loadedSurface = IMG_Load( filename.c_str() );
	if( loadedSurface == NULL ) {
		printf( "Unable to load image %s! SDL_image Error: %s\n", filename.c_str(), IMG_GetError() );
	} else {
		if (drawable) {
			newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
	        newerTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, loadedSurface->w, loadedSurface->h);
			SDL_SetTextureBlendMode(newerTexture, SDL_BLENDMODE_BLEND);
			SDL_SetRenderTarget(renderer, newerTexture);
			SDL_RenderClear(renderer);
			SDL_Rect src = {0,0,loadedSurface->w,loadedSurface->h};
			SDL_Rect dest = {0,0,loadedSurface->w,loadedSurface->h};
			SDL_RenderCopy(renderer,newTexture,&src,&dest);
			SDL_SetRenderTarget(renderer, NULL); //NULL SETS TO DEFAULT
			SDL_DestroyTexture(newTexture);
			if( newerTexture == NULL ) {
				printf( "Unable to create texture from %s! SDL Error: %s\n", filename.c_str(), SDL_GetError() );
			}
			SDL_FreeSurface( loadedSurface );
			return newerTexture;
		} else {
			newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
			if( newTexture == NULL ) {
				printf( "Unable to create texture from %s! SDL Error: %s\n", filename.c_str(), SDL_GetError() );
			}
			SDL_FreeSurface( loadedSurface );
			return newTexture;
		}
	}
}

void Graphics::blitTexture(SDL_Texture* tdest, SDL_Texture* tsrc, SDL_Rect* src, SDL_Rect* dest, bool clear) {
	SDL_SetRenderTarget(renderer, tdest);
	if (clear) SDL_RenderClear(renderer);
	SDL_SetTextureBlendMode(tdest, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(renderer,tsrc,src,dest);
	SDL_SetRenderTarget(renderer, NULL);
}

void Graphics::blitTexture(SDL_Texture* tdest, SDL_Texture* tsrc, int x, int y, bool clear) {
	int w, h;
	SDL_QueryTexture(tsrc, NULL, NULL, &w, &h);
	SDL_Rect src = {0,0,w,h};
	SDL_Rect dest = {x,y,w,h};
	blitTexture(tdest,tsrc,&src,&dest,clear);
}

void Graphics::blitTexture(SDL_Texture* tdest, SDL_Texture* tsrc, int x, int y, int w, int h, bool clear) {
	SDL_Rect src = {0,0,w,h};
	SDL_Rect dest = {x,y,w,h};
	blitTexture(tdest,tsrc,&src,&dest,clear);
}

void Graphics::drawTexture(SDL_Texture* source, int x, int y, int w, int h) {
	SDL_Rect dest = {x,y,w,h};
	SDL_RenderCopy(renderer, source, NULL, &dest);
}

void Graphics::update() {

	SDL_RenderClear(renderer);

	// Draw the Room elements
	gRoom->draw();

	// Draw GUI
	for (int i = 0; i < guielements.size(); i++) {
		guielements[i]->draw();
	}

	// Draw Transition (if occurring)
	if (trans) {
		if (trans->update()) {
			if (trans->getMode() == S2M_TRANSITION_CLOSE) {
				SDL_Delay(trans->getDelay());
				trans->changeMode();
				trans->update();
				gRoom = gRoom->nextRoom;
				//gRoom->setCamera(new Camera(0,0));
			} else {
				delete trans;
				trans = nullptr;
			}
		}
	}

	SDL_RenderPresent(renderer);
}

void Graphics::addSprite(Sprite *sprite) {
	sprites.push_back(sprite);
}

void Graphics::addGUIElement(GUIElement *g) {
	guielements.push_back(g);
}

int Graphics::getGameWidth() {
	return gamewidth;
}

int Graphics::getGameHeight() {
	return gameheight;
}

Transition *Graphics::getTransition() {
	return trans;
}

void Graphics::setTransition(Transition *t) {
	if (!trans) trans = t;
	else delete t;
}

SDL_Renderer *Graphics::getRenderer() {
	return renderer;
}

Sprite::Sprite(string filename, int w, int h) {
	// Creamos la textura del spritesheet
	//texture = SDL_CreateTextureFromSurface(gGraphics->renderer, IMG_Load(filename.c_str()));
	texture = gGraphics->loadTexture(filename, true);
	if (texture == NULL) cout << "Error de textura" << endl;
	SDL_QueryTexture(texture, NULL, NULL, &wtexture, &htexture);
	// Obtenemos las dimensiones tanto del sprite como del spritesheet
	width = w;
	height = h;
	// Añadimos el Sprite al vector global de Sprites actuales de gGraphics
	gGraphics->addSprite(this);
	//DEBUG: Creamos el rect a mano
	/**/rect = {3,2,6,14};
}

Sprite::~Sprite() {
	SDL_DestroyTexture(texture);
}

int Sprite::getWidth() {
	return width;
}

int Sprite::getHeight() {
	return height;
}

void Sprite::setCenter(int xc, int yc) {
	xcenter = xc;
	ycenter = yc;
}

int Sprite::getXCenter() {
	return xcenter;
}

int Sprite::getYCenter() {
	return ycenter;
}

SDL_Rect *Sprite::getRect(int frame) {
	div_t divresult;
	divresult = div(frame,wtexture/width);
	int y = divresult.quot;
	int x = divresult.rem;
	SDL_Rect *src = new SDL_Rect {x*width,y*height,width,height};
	return src;
}

void Sprite::addAnimation(vector<int> animation) {
	animations.push_back(animation);
}

vector<int> Sprite::retAnimation(int i) {
	if (i < int(animations.size()) && i >= 0) {
		return animations[i];
	} else cout << "Trying to retrieve animation of index " << i << " when it doesn't even exist... (size=" << animations.size() << ")" << endl;
}

int Sprite::getAnimationsSize() {
	return animations.size();
}

void Sprite::update() {
	// TODO: Update SDL_Rect rect according to current frame.
}

Transition::Transition(bool m, bool s) {
	mode = m;
	single = s;
}

Transition::~Transition() {
	SDL_DestroyTexture(texture);
}

bool Transition::getMode() {
	return mode;
}

int Transition::getDelay() {
	return delay;
}

void Transition::changeMode() {
	done = false;
	mode = !mode;
	cout << "Changing mode..." << endl;
}

bool Transition::isSingle() {
	return single;
}

bool Transition::update() {

}