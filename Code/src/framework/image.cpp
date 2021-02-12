#include "image.h"


Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

//copy constructor
Image::Image(const Image& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
}

//assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height*sizeof(Color)];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
	return *this;
}

Image::~Image()
{
	if(pixels) 
		delete pixels;
}



//change image size (the old one will remain in the top-left corner)
void Image::resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for(unsigned int x = 0; x < min_width; ++x)
		for(unsigned int y = 0; y < min_height; ++y)
			new_pixels[ y * width + x ] = getPixel(x,y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

//change image size and scale the content
void Image::scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = getPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::getArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			if( (x + start_x) < this->width && (y + start_y) < this->height) 
				result.setPixel( x, y, getPixel(x + start_x,y + start_y) );
		}
	return result;
}

void Image::flipX()
{
	for(unsigned int x = 0; x < width * 0.5; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			Color temp = getPixel(width - x - 1, y);
			setPixel( width - x - 1, y, getPixel(x,y));
			setPixel( x, y, temp );
		}
}

void Image::flipY()
{
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height * 0.5; ++y)
		{
			Color temp = getPixel(x, height - y - 1);
			setPixel( x, height - y - 1, getPixel(x,y) );
			setPixel( x, y, temp );
		}
}


//Loads an image from a TGA file
bool Image::loadTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int bytesPerPixel;
	unsigned int imageSize;

	FILE * file = fopen(filename, "rb");
   	if ( file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "File not found: " << filename << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;
    
	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];
    
	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		std::cerr << "TGA file seems to have errors or it is compressed, only uncompressed TGAs supported" << std::endl;
		fclose(file);
		delete tgainfo;
		return NULL;
	}
    
	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;
    
	tgainfo->data = new unsigned char[imageSize];
    
	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		if (tgainfo->data != NULL)
			delete tgainfo->data;
            
		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	//save info in image
	if(pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];

	//convert to float all pixels
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			this->setPixel(x , height - y - 1, Color( tgainfo->data[pos+2], tgainfo->data[pos+1], tgainfo->data[pos]) );
		}

	delete tgainfo->data;
	delete tgainfo;

	return true;
}

// Saves the image to a TGA file
bool Image::saveTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	FILE *file = fopen(filename, "wb");
	if ( file == NULL )
	{
		fclose(file);
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	//tgainfo->width = header[1] * 256 + header[0];
	//tgainfo->height = header[3] * 256 + header[2];

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	//convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width*height*3];
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[(height-y-1)*width+x];
			unsigned int pos = (y*width+x)*3;
			bytes[pos+2] = c.r;
			bytes[pos+1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height*3, file);
	fclose(file);
	return true;
}

#ifndef IGNORE_LAMBDAS

//you can apply and algorithm for two images and store the result in the first one
//forEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void forEachPixel(Image& img, const Image& img2, F f) {
	for(unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f( img.pixels[pos], img2.pixels[pos] );
}

void Image::drawRectangle(int x, int y, int h, int w, Color color,bool fill) {
	
	if (fill == false) {
		for (int position_x = x; position_x < (x + w); ++position_x) {

			setPixel(position_x, y, color);
			setPixel(position_x, y + h - 1, color);

		}for (int position_y = y + 1; position_y < (y + h - 1); ++position_y) {

			setPixel(x, position_y, color);
			setPixel(x + w - 1, position_y, color);

		}
	}
	else {
		for (int position_x = x; position_x < (x + w); ++position_x) {
			for (int position_y = y; position_y < (y + h); ++position_y) {

				setPixel(position_x, position_y, color);
			}
		}
	}
}

void Image::drawCircle(int l, int a, int r, Color color,bool fill) {
	
		for (double x = 0; x < 2 * PI; x = x + 0.01) {

			double position_x = r * cos(x);
			double position_y = r * sin(x);

	if (fill == false) {

			for (double j = 0; j < r; j = j + 0.01) {
				double R1 = position_x * (cos(j) + sin(j)) + l;
				double R2 = position_y * (cos(j) + sin(j)) + a;
				setPixel(R1, R2, color);

			}
	}
	else {
		for (double x = 0; x < 2 * PI; x = x + 0.01) {
			double position_x = r * cos(x) + l;
			double position_y = r * sin(x) + a;
			setPixel(position_x, position_y, color);
		}

	}
		
		}
}

void Image::drawLine(int x1, int y1, int x2, int y2, Color color) {

	float initial_x, initial_y, der_initial_x, der_initial_y, s;

	der_initial_x = x2 - x1;
	der_initial_y = y2 - y1;
	float m = der_initial_y / der_initial_x;

	//setPixel(x1, x2, Color(255,255,255));
	//setPixel(y1, y2, Color(255,255,255));

	//Creamos todas las posibles condiciones
	if (der_initial_x == 0) {
		if (x2 < y2) {
			for (initial_y = x2; initial_y < y2; initial_y++) {
				setPixel(x1, initial_y, color);
			}
		}
		else {
			for (initial_y = y2; initial_y < x2; initial_y++) {
				setPixel(x1, initial_y, color);
			}
		}
	}

	if (der_initial_x != 0) {

		if (x1 < x2) {
			for (initial_x = x2; initial_x < x2; initial_y++) {
				initial_y = y2 + m * (initial_x - x2);
				setPixel(initial_x, initial_y, color);
			}
		}

		else {
			for (initial_x = x2; initial_x < x1; initial_x++) {
				initial_y = y2 + m * (initial_x - x2);
				setPixel(initial_x, initial_y, color);
			}
		}
	}


	for (int initial_x = x1; initial_x < x2; initial_x++) {
		if (y1 < y2) {
			setPixel(initial_x, y1, color);
			y1++;
		}
	}
}

//FUNCIÓN QUE SIMULA UN GRADIENTE DE COLOR
void Image::ColorVariant(){
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			float f = x / (float)width;
			f = f * 255;
			setPixel(x, y, Color(f, 0, 0));
		}

	}
}

void Image::table() {
	int f;
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			f = fmod(x / 50 + y / 50, 2);
			f = f * 255;
			setPixel(x, y, Color(f, f, f));

		}
	}
}

//Hace zoom en una imagen
Image Image::scale(Image myimage){
	Image ImageScale = Image(myimage.width,myimage.height);
	Color pixel;
	
	for (int x = 0; x < myimage.width; x++) {
		for (int y = 0; y < myimage.height; y++) {
			pixel = myimage.getPixel(x*0.1, y*0.1);
			ImageScale.setPixel(x,y,pixel);
		}
	}
	return ImageScale;
}

Image Image::invert(Image myimage) { //Se encarga de invertir los colores de una imagen

	Image ImageInvert = Image(myimage.width, myimage.height);
	Color inv;

	//INVERT
	for (int x = 0; x < myimage.width; x++) {
		for (int y = 0; y < myimage.height; y++) {

			Color inv = myimage.getPixel(x, y);
			inv.r = 255 - inv.r;
			inv.g = 255 - inv.g;
			inv.b = 255 - inv.b;
			ImageInvert.setPixel(x, y, inv);
		}
	}
	return ImageInvert;
}


Image Image::grayScale(Image myimage) {//Se encarga de cambiar los colores a escala de grises

	Image ImageGray = Image(myimage.width, myimage.height);
	Color gray;

	//GRAYSCALE
	for (int x = 0; x < myimage.width; x++) {
		for (int y = 0; y < myimage.height; y++) {

			Color gray = myimage.getPixel(x, y);
			float valor = 0.2126 * gray.r + 0.7152 * gray.g + 0.0722 * gray.b;
			gray.r = valor;
			gray.g = valor;
			gray.b = valor;
			ImageGray.setPixel(x, y, gray);
		}
	}
	return ImageGray;
}

#endif