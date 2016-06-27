#include "ModeloJugador.h"


ModeloJugador::ModeloJugador(int id){
	this->id = id;
	this->puntajeAcumulado = 0;
}


ModeloJugador::~ModeloJugador(){

}

int ModeloJugador::getId(){
	return id;
}

void ModeloJugador::sumarPuntos(int cantidadDePuntos){
	this->puntajeAcumulado += cantidadDePuntos;
}

EstadoJugador ModeloJugador::getEstadoJugador(){
	return(EstadoJugador(id,puntajeAcumulado));
}