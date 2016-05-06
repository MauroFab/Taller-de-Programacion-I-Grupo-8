#include "ElementoDelMapa.h"


ElementoDelMapa::ElementoDelMapa(int xInicial, int yInicial, SDL_Renderer* gRendererRecibido, Textura* texturaRecibida)
{
	x = xInicial;
	y = yInicial;
	offset = 0;
	gRenderer = gRendererRecibido;
	textura = texturaRecibida;
}


ElementoDelMapa::~ElementoDelMapa(void){
}

void ElementoDelMapa::reiniciar(){
		offset = 0;
}

bool ElementoDelMapa::estaEnPantalla(int yMaxPantalla,int  yMinPantalla){
	bool estaEnPantalla;
	return ((y - textura->getHeight() < yMaxPantalla)
			&& (y + offset > yMinPantalla));
}

bool ElementoDelMapa::apareceDesdeAfuera(int yMaxPantalla, int yMinPantalla){
	int tamanioVertical;
	tamanioVertical = (yMaxPantalla - yMinPantalla);
	return (y > tamanioVertical);
}

void ElementoDelMapa::graficarseSiEstaEnPantalla(int yMaxPantalla,int  yMinPantalla){

	if(estaEnPantalla(yMaxPantalla, yMinPantalla)){
		if(apareceDesdeAfuera(yMaxPantalla, yMinPantalla)){
			//Para el render y = 0 es el tope de la pantalla
			//La imagen comienza a verse no en 0, si no cuando solo una parte de la imagen ya entra, y desde arriba
			textura->render(x, offset - textura->getHeight() ,gRenderer);
			offset++;
		}else{
			int tamanioVertical;
			tamanioVertical = (yMaxPantalla - yMinPantalla);
			int posicionInicial = tamanioVertical - y;
			textura->render(x, offset - textura->getHeight() + posicionInicial ,gRenderer);
			offset++;
		}
	}
}