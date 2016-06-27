#include "Avion.h"

Avion::Avion(int ventanaAncho, int ventanaAlto, AvionView* avionView, BalaView* balaView) {
	//Las posicion inicial queda sin definir
	this->ventanaAncho = ventanaAncho;
	this->ventanaAlto = ventanaAlto;
	this->altoAvion = avionView->spriteXml->getAlto();
	this->anchoAvion = avionView->spriteXml->getAncho();
	this->velocidad = avionView->avionModel->velAvion;
    velocidadX = 0;
    velocidadY = 0;
	cantDeFotogramas = avionView->spriteXml->getCantidad();
	frame = 0;
	rollFlag = false;
	id = avionView->avionModel->id;
	this->balaView = balaView;
	centroProyectilMejorado = balaView->spriteXml->getAncho()/2;
	centroProyectilSinMejora = 3;
	superficieQueOcupo = SuperficieOcupada(0,0,anchoAvion,altoAvion);
	puntosDeVida = vidaMaximaAvion;
	this->jugadorAsociado = new ModeloJugador(id);
	soyInvulnerable = false;
	tengoElArmaMejorada = false;
}

Avion::~Avion() {
	std::list<Proyectil*>::iterator it;
	for (it = proyectiles.begin(); it != proyectiles.end(); it++) {
		delete (*it);
	}
	delete jugadorAsociado;
}

void Avion::setPosicion(Posicion pos) {
	superficieQueOcupo.moverAPosicion(pos);
}

SuperficieOcupada Avion::actualizarPosicionEnX(){
	SuperficieOcupada hitbox;
	hitbox = superficieQueOcupo.desplazarEnXObteniendoHitbox(velocidadX);

	int miPosicionEnX;
	miPosicionEnX = superficieQueOcupo.obtenerPosicion().getPosX();

	if( miPosicionEnX< 0){
		superficieQueOcupo.moverAX(0);
	}

	if( miPosicionEnX + anchoAvion > this->ventanaAncho ){
		superficieQueOcupo.moverAX(this->ventanaAncho - anchoAvion);
	}
	return hitbox;
}

SuperficieOcupada Avion::actualizarPosicionEnY(){
	SuperficieOcupada hitbox;
	superficieQueOcupo.desplazarEnYObteniendoHitbox(velocidadY);
	int miPosicionEnY;
	miPosicionEnY = superficieQueOcupo.obtenerPosicion().getPosY();

	if( miPosicionEnY < 0){
		superficieQueOcupo.moverAY(0);
	}

	if( miPosicionEnY + altoAvion > this->ventanaAlto ){
		superficieQueOcupo.moverAY(this->ventanaAlto - altoAvion);
	}
	return hitbox;
}

void Avion::continuarElRoll(){
	if ((frame / cantDeFotogramas) >= cantDeFotogramas - 1){
			frame = 0;
			rollFlag = false;
	}
	frame++;
}

void Avion::continuarMovimientoDelAvion(){
	SuperficieOcupada hitbox;
	if(!rollFlag){
		hitbox = actualizarPosicionEnX();
		actualizarPosicionEnY();
	}else{
		continuarElRoll();
	}
}

void Avion::revisoColisionesConEnemigos(SuperficieOcupada hitbox, list<FakeAvionEnemigo*> &avionesEnemigos,
							 list<PowerUp> &powerUps){
	std::list<FakeAvionEnemigo*>::iterator it;
	for (it = avionesEnemigos.begin(); it != avionesEnemigos.end(); it++) {
		if(hitbox.meSolapoCon((*it)->obtenerSuperficieOcupada()) && !(*it)->estaDestruido()){
		
			if(!soyInvulnerable)
				this->puntosDeVida--;
			(*it)->recibeUnImpacto(this->jugadorAsociado->getId());

			if((*it)->estaDestruido()){
				this->jugadorAsociado->sumarPuntos((*it)->getPuntosQueOtorgaAlSerDestruido());
				if((*it)->dejaUnPowerUpAlSerDestruido())
					powerUps.push_back((*it)->getPowerUpQueDejaAlSerDestruido());
			}
		}
	}
}
void Avion::destruirEnemigosEnPantalla(list<FakeAvionEnemigo*> &avionesEnemigos){
	std::list<FakeAvionEnemigo*>::iterator it;
	for (it = avionesEnemigos.begin(); it != avionesEnemigos.end(); it++) {
		if((*it)->estaEnPantalla()){
			(*it)->destruir();
			this->jugadorAsociado->sumarPuntos((*it)->getPuntosQueOtorgaAlSerDestruido());
		}
	}
}
void Avion::resolverColisionEntreElAvionYElPowerUp(PowerUp &powerUp, 
											       list<FakeAvionEnemigo*> &enemigos){
	//Si nunca fue usado (Los power ups tienen un solo uso)
	if(!powerUp.fueUsado()){
		powerUp.marcarComoUsado();
		if(powerUp.esDePuntos()){
			this->jugadorAsociado->sumarPuntos(powerUp.obtenerPuntosQueOtorga());
		}
		if(powerUp.esDeAmetralladora()){
			tengoElArmaMejorada = true;
		}
		if(powerUp.esDeMuerte()){
			destruirEnemigosEnPantalla(enemigos);
		}
		//Aca irian los otros casos de power ups.
	}
}

void Avion::revisoColisionesConPowerUps(SuperficieOcupada hitbox, list<PowerUp> &powerUps, 
							 list<FakeAvionEnemigo*>& avionesEnemigos){
	std::list<PowerUp>::iterator it;
	for (it = powerUps.begin(); it != powerUps.end(); it++) {
		//Si toco al power up
		if(hitbox.meSolapoCon((*it).obtenerSuperficieOcupada()) && !(*it).fueUsado()){
			//Revis con los power ups, los avionesEnemigos son por si es de muerte
			resolverColisionEntreElAvionYElPowerUp(*it, avionesEnemigos);
		}
	}
}


void Avion::continuarMovimientoDelAvion(list<FakeAvionEnemigo*> &avionesEnemigos,
										list<PowerUp> &powerUps){
	//Los movimientos se hacen unidimensionalmente
	//Primero en X y luego en Y
	SuperficieOcupada hitbox;
	
	if(!rollFlag){
		hitbox = actualizarPosicionEnX();
		//Los power ups son solo para agregar si se destruye un avion que libera uno
		//Reviso colisiones con power ups
		revisoColisionesConEnemigos(hitbox,avionesEnemigos, powerUps);
		//Reviso colisiones con aviones enemigos
		revisoColisionesConPowerUps(hitbox,powerUps, avionesEnemigos);
		hitbox = actualizarPosicionEnY();
		revisoColisionesConEnemigos(hitbox,avionesEnemigos, powerUps);
		revisoColisionesConPowerUps(hitbox,powerUps, avionesEnemigos);
	//Si estoy haciendo un roll, no colisiono tampoco
	}else{
		continuarElRoll();
	}
}
void Avion::continuarMovimientoDeLosProyectiles(std::list<FakeAvionEnemigo*> &avionesEnemigos,
												std::list<PowerUp> &powerUps){
	std::list<Proyectil*>::iterator it;
	for (it = proyectiles.begin(); it != proyectiles.end(); it++) {
		if ((*it)->estaEnPantalla()) {
			(*it)->mover(avionesEnemigos, jugadorAsociado, powerUps);
		}
	}
}

void Avion::eliminarLosProyectilesQueSalieronDeLaPantalla(){
		Proyectil* ultimoProyectil = proyectiles.front();
		if(!ultimoProyectil->estaEnPantalla()){
			proyectiles.pop_front();
			delete ultimoProyectil;
		}
}


void Avion::mover(list<FakeAvionEnemigo*> &avionesEnemigos, list<PowerUp> &powerUps) {
	if(!estoyDestruido()){
		continuarMovimientoDelAvion(avionesEnemigos, powerUps);
		//Avanzo los proyectiles
		continuarMovimientoDeLosProyectiles(avionesEnemigos, powerUps);
		//Si hay proyectiles
		if(!proyectiles.empty())
			eliminarLosProyectilesQueSalieronDeLaPantalla();
	}
}

EstadoAvion* Avion::getEstado() {
	//Paso una cantidad de puntos de vida cualquiera hasta que lo programe
	int miPosicionEnY;
	miPosicionEnY = superficieQueOcupo.obtenerPosicion().getPosY();
	int miPosicionEnX;
	miPosicionEnX = superficieQueOcupo.obtenerPosicion().getPosX();
	EstadoAvion*  estado =  new EstadoAvion(id, frame, puntosDeVida, 
											miPosicionEnX, miPosicionEnY);
	std::list<EstadoProyectil*> lista;
	std::list<Proyectil*>::iterator it;
	for (it = proyectiles.begin(); it != proyectiles.end(); it++) {
		estado->agregarEstadoProyectil((*it)->createEstado());
	}
	return estado;
}

std::list<EstadoProyectil*> Avion::getEstadoProyectiles() {
	std::list<EstadoProyectil*> lista;
	std::list<Proyectil*>::iterator it;
	for (it = proyectiles.begin(); it != proyectiles.end(); it++) {
		lista.push_back((*it)->createEstado());
	}
	return lista;
}

void Avion::realizarAccionEnBaseA(Evento* evento){
	switch(evento->getNumeroDeEvento()){
    case apretadaLaTeclaDeMovimientoHaciaIzquierda  :
		this->darVelocidadHaciaLaIzquierda();
       break;
    case soltadaLaTeclaDeMovimientoHaciaIzquierda   :
		this->darVelocidadHaciaLaDerecha();
       break;
	case apretadaLaTeclaDeMovimientoHaciaDerecha   :
		this->darVelocidadHaciaLaDerecha();
       break;
	case soltadaLaTeclaDeMovimientoHaciaDerecha    :
		this->darVelocidadHaciaLaIzquierda();
       break;
	case apretadaLaTeclaDeMovimientoHaciaArriba   :
		this->darVelocidadHaciaArriba();
       break;
	case soltadaLaTeclaDeMovimientoHaciaArriba  :
		this->darVelocidadHaciaAbajo();
       break;
	case apretadaLaTeclaDeMovimientoHaciaAbajo  :
		this->darVelocidadHaciaAbajo();
       break;
	case soltadaLaTeclaDeMovimientoHaciaAbajo  :
		this->darVelocidadHaciaArriba();
       break;
	case apretadaLaTeclaDeDisparo :
		this->disparar();
       break;
	case apretadaLaTeclaDeRoll  :
		this->hacerUnRoll();
       break;
	}
}
//BUG-000 aca hay un error pues plantea que se suma la misma velocidad en ambas coordenadas
//deltaY <-- (+vel)
void Avion::darVelocidadHaciaArriba(){
	velocidadY += velocidad;
}
//deltaY <-- (-vel)
void Avion::darVelocidadHaciaAbajo(){
	velocidadY -= velocidad;
}
//deltaX <-- (+vel)
void Avion::darVelocidadHaciaLaDerecha(){
	velocidadX += velocidad;
}
//deltaX <-- (-vel)
void Avion::darVelocidadHaciaLaIzquierda(){
	velocidadX -= velocidad;
}

void Avion::disparar(){
	int miPosicionEnY;
	miPosicionEnY = superficieQueOcupo.obtenerPosicion().getPosY();
	int miPosicionEnX;
	miPosicionEnX = superficieQueOcupo.obtenerPosicion().getPosX();

	if(!rollFlag){
		Proyectil* proyectil = new Proyectil(this->balaView, tengoElArmaMejorada);
		if(tengoElArmaMejorada){
		
			proyectil->setCoordenasDeComienzo(miPosicionEnX + (anchoAvion / 2) - centroProyectilMejorado, 
												(miPosicionEnY + altoAvion));
		
		}else{

			proyectil->setCoordenasDeComienzo(miPosicionEnX + (anchoAvion / 2) - centroProyectilSinMejora, 
												(miPosicionEnY + altoAvion));
		}
		proyectiles.push_back(proyectil);
	}
}

void Avion::hacerUnRoll(){
	rollFlag = true;
}

EstadoJugador Avion::getEstadoJugadorAsociado(){
	return(jugadorAsociado->getEstadoJugador());
}

void Avion::sumarPuntosAlJugadorAsociado(int puntos){
	this->jugadorAsociado->sumarPuntos(puntos);
}

void Avion::volverseInvulnerable(){
	this->soyInvulnerable = true;
}
void Avion::dejarDeSerInvulnerable(){
	this->soyInvulnerable = false;
}

bool Avion::estoyDestruido(){
	return (puntosDeVida <= 0);
}