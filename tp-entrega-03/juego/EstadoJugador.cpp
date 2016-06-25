#include "EstadoJugador.h"


EstadoJugador::EstadoJugador(int id, int puntajeAcumulado){
	this->id = id;
	this->puntajeAcumulado = puntajeAcumulado;
}
EstadoJugador::EstadoJugador(){
	this->id = -1;
	this->puntajeAcumulado = -1;
}

int EstadoJugador::getid(){
	return id;
}

int EstadoJugador::getPuntajeAcumulado(){
	return puntajeAcumulado;
}

EstadoJugador::~EstadoJugador(void){
}