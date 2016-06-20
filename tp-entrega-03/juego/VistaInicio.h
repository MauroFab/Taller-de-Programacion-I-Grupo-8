#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include "TextField.h"
#include "Etiqueta.h"
#include "FondoInicio.h"

#define FUENTE_PATH "./sfd/DejaVuSans.ttf"
#define PATH_FONDO_IP "fondoMenuIP.bmp"
#define PATH_FONDO_PUERTO "fondoMenuPuerto.bmp"
#define PATH_FONDO_USUARIO "fondoMenuUsuario.bmp"

#define CANTIDAD_FONDOS 3
#define IP 0
#define PUERTO 1
#define USUARIO 2

#define WIDTH 480;
#define HEIGHT 640;

using namespace std;

class VistaInicio {

public:
	static VistaInicio* getInstance();
	virtual ~VistaInicio(void);

	bool inicializar();
	void close();
	void mostrar();
	void mostrarMensajeInformacion(string mensaje);
	string getPuerto();
	string getIP();
	string getUsuario();

private:
	VistaInicio();
	static bool instanceFlag;
	static VistaInicio* instance;

	void cargarFondos();

	int ventanaAncho;
	int ventanaAlto;
	SDL_Window* gWindow;
	SDL_Renderer* gRenderer;
	TTF_Font* gFont;

	TextField* textoDeEntrada;
	FondoInicio* fondos[CANTIDAD_FONDOS];
	string datos[CANTIDAD_FONDOS];
};

