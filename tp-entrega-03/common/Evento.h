#pragma once

#include "IGenericaVO.h"

const int noHayNingunEventoEspecial = 0;

//Los siguientes se usan unicamente para mensajes del cliente al servidor
const int apretadaLaTeclaDeMovimientoHaciaIzquierda = 1;
const int soltadaLaTeclaDeMovimientoHaciaIzquierda = 2;

const int apretadaLaTeclaDeMovimientoHaciaDerecha = 3;
const int soltadaLaTeclaDeMovimientoHaciaDerecha = 4;

const int apretadaLaTeclaDeMovimientoHaciaArriba = 5;
const int soltadaLaTeclaDeMovimientoHaciaArriba = 6;

const int apretadaLaTeclaDeMovimientoHaciaAbajo = 7;
const int soltadaLaTeclaDeMovimientoHaciaAbajo = 8;

const int apretadaLaTeclaDeDisparo= 9;
const int soltadaLaTeclaDeDisparo = 10;

const int apretadaLaTeclaDeRoll = 11;
const int soltadaLaTeclaDeRoll = 12;

const int apretadaLaTeclaDeReinicio= 13;
const int soltadaLaTeclaDeReinicio = 14;

const int seNecesitaLaPosicionDelMapa = 15;

//Los siguientes se usan solamente para mandar mensajes del servidor al cliente
//Se podria separar todo en 2 clases de eventos distintos
const int comienzaLaPartida = -1;

class Evento
{
public:
	//El evento se crea con una de las constantes puestas arriba, se podr�an redefinir por herencia cada uno.
	Evento(int numeroDeEvento);
	int getNumeroDeEvento();
	void setNumeroDeEvento(int numero);
	int getSizeBytes();
	void toString(TCadena1000 cadena);
	~Evento();
private:
	int numeroDeEvento;
};

