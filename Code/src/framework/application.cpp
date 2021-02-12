#include "application.h"
#include "utils.h"
#include "image.h"

//Variables creadas para el Laboratorio
Image ImgA; //Variable de tipo imagen donde almaceno alguna imagen que me haga falta 
Image imagenB;//Variable de tipo imagen donde almaceno la imagen para el filtro
Image imagenA;//Variable de tipo imagen donde almaceno la imagen para el zoom
Image imagenC;//Variable de tipo imagen donde almacenola ciudad para el efecto de nieve
Image imgAuxiliar;//Variable de tipo imagen donde almaceno el regreso al menú
Vector2 vector[500]; //guardar posicion
Vector2 velocidad[500]; //encargado de la velocidad;
int estado = 0; //Variable de tipo int que me sirve para regresar al RENDER
int copos = 50;


Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w,h;
	SDL_GetWindowSize(window,&w,&h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);

	framebuffer.resize(w, h);
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;

	//here add your init stuff
	ImgA.loadTGA("MenuInterfaz.tga");//INTERFAZ QUE SE MOSTRARÁ POR PANTALLA
	imagenA.loadTGA("fruta.tga");//IMAGEN PARA EL ZOOM
	imagenB.loadTGA("foto-en-sepia-de-un-parque.tga");//IMAGEN PARA LOS FILTROS
	imagenC.loadTGA("snow.tga");//IMAGEN DE FONDO PARA LA CIUDAD NEVADA
	imgAuxiliar.loadTGA("MenuInterfaz.tga");
	

	//con esto generamos los copos de nieve
	for (int x = 0; x < copos; x++) {
		//le doy valores aleatorios para tratar de conseguir el efecto de caida de nieve
		vector[x].x = (randomValue() * 794/*<-- valores elegido para que abarque la ventana de la framebuffer*/)+5;
		vector[x].y = (randomValue()* 590/*<-- valores elegido para que abarque la ventana de la framebuffer*/)+5;
		velocidad[x].x = (randomValue() * 10) + 1; //el tamaño
		velocidad[x].y = (randomValue() * 40/*<--Con este valor acelero la caída de nieve*/) + 0.5 ; //y la velocidad
	}
}

//render one frame
void Application::render( Image& framebuffer )
{
	//clear framebuffer if we want to start from scratch
	framebuffer = ImgA;
	
	
	//here you can add your code to fill the framebuffer
	
	if (estado == 1) {
		for (int i = 0; i < copos; i++) {
			framebuffer.drawCircle(vector[i].x, vector[i].y, (velocidad[i].x) / 4, Color::WHITE, false);
			vector[i].y = vector[i].y - velocidad[i].y;

			if (vector[i].y < 4) {
				vector[i].y = 590;
			}
		}
	}
}

//called after render
void Application::update(double seconds_elapsed)
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	if (keystate[SDL_SCANCODE_0]){
		//REGRESAR AL MENÚ PRINCIPAL
		estado = 0;
		ImgA = imgAuxiliar;
	}
	
	//--------------------------IDEA 1--------------------------
	if (keystate[SDL_SCANCODE_1]) //if key space is pressed
	{
		estado = 0;
		framebuffer.fill(Color::BLACK);
		framebuffer.drawRectangle(100, 100, 100, 100, Color::CYAN, true);
		framebuffer.drawCircle(300, 300, 50, Color::GREEN, true); //
		framebuffer.drawLine(500, 200, 150, 170, Color::RED);//drawLine(x1,y1,x2,y2,color)
		ImgA = framebuffer;	
	}
	if (keystate[SDL_SCANCODE_2]) {
			estado = 0;
			framebuffer.fill(Color::BLACK);
			ImgA.fill(Color::BLACK);
			framebuffer.drawRectangle(100, 100, 100, 100, Color::CYAN,false);
			framebuffer.drawCircle(300, 300, 50, Color::GREEN,false); //
			ImgA = framebuffer;
	}
	//----------------------------------------------------------------

	//------------------------IDEA 2---------------------------------
	if (keystate[SDL_SCANCODE_3]) {
		//IDEA 2
		estado = 0;
		ImgA.fill(Color::BLACK);
		ImgA.ColorVariant();
	}

	if (keystate[SDL_SCANCODE_4]) {
		//IDEA 2
		estado = 0;
		ImgA.fill(Color::BLACK);
		ImgA.table();
	}
	//----------------------------------------------------------------

	//-----------------------IDEA 3----------------------------------
	if (keystate[SDL_SCANCODE_6]) {
		//IDEA 3
		estado = 0;
		ImgA = framebuffer.invert(imagenB);
	}
	if (keystate[SDL_SCANCODE_5]) {
		estado = 0;
		ImgA = framebuffer.grayScale(imagenB);
	}

	//---------------------------------------------------------------

	//----------------------IDEA 4------------------------------------
	if (keystate[SDL_SCANCODE_7]) {
		estado = 0;
		ImgA = ImgA.scale(imagenA); 
		//POSIBLE MEJORAS:
		//Poder elegir donde quieres hacer Zoom tomando en cuenta
		//la posición del mouse. Podriamos pasarle los valores de la posición del mouse y cuando dé click
		//en un área de la imagen que haga Zoom en ese punto.
	}
	//----------------------IDEA 5---------------------------------
	if (keystate[SDL_SCANCODE_8]) {
		ImgA = imagenC;
		estado = 1;
	}
	//----------------------------------------------------------
	
}


	//to read mouse position use mouse_position

//keyboard press event 
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	//to see all the keycodes: https://wiki.libsdl.org/SDL_Keycode
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE:
			exit(0); 
			break; //ESC key, kill the app

	}
}

//keyboard key up event 
void Application::onKeyUp(SDL_KeyboardEvent event)
{
	//...
}

//mouse button event
void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{
		//if you read mouse position from the event, careful, Y is reversed, use mouse_position instead
	}
}

void Application::onMouseButtonUp( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{

	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}
