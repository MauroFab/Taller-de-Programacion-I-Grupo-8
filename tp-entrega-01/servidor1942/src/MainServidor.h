#ifndef _MAINSERVIDOR_H_
#define _MAINSERVIDOR_H_

#include <iostream>
#include <string>
#include <queue>     
#include <vector>
#include <algorithm>
#include <WinSock2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#include "asignadorDeUsuarios.h"

class MainServidor
{
private:
    static bool instanceFlag;
    static MainServidor *single;
	AsignadorDeUsuarios *usuarios;
    MainServidor();

public:
	bool seDebeCerrarElServidor;
	std::queue<char*> colaDeMensaje;
	std::vector<SDL_Thread*> vectorHilos;
	std::vector<SOCKET*> vectorSockets;
	SOCKET socketDeEscucha;
	SDL_mutex *mut; // el mutex para proteger la cola de mensajes

public:

	virtual ~MainServidor();
	static MainServidor* getInstance();
public:
	SOCKET obtenerSocketInicializado(sockaddr_in &local);
	void ponerAEscuchar(SOCKET sock);
	static int fun_atenderCliente(void* punteroAlSocketRecibido);
	static int fun_recibirConexiones(void*);
	static int fun_consolaDelServidor(void*);
	static int fun_revisarSiHayMensajesParaElClienteYEnviarlos(void* idYPunteroAlSocketRecibido);

	int revisarSiHayMensajesParaElClienteYEnviarlos(void* idYPunteroAlSocketRecibido);
	int atenderCliente(void* punteroAlSocketRecibido);
	int recibirConexiones(void*);
	int consolaDelServidor(void*);
	int mainPrincipal();
};

#endif //_MAINSERVIDOR_H_