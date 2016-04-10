#include "MainServidor.h"
#include "asignadorDeUsuarios.h"
bool MainServidor::instanceFlag = false;
MainServidor* MainServidor::single = NULL;

struct IdYPunteroAlSocket {
  int id;
  SOCKET* punteroAlSocket;
};

struct StructDelEnviadorDeMensajes {
  IdYPunteroAlSocket idYPunteroAlSocket;
  bool* seCerroLaConexion;
};

MainServidor::MainServidor(){
	static int cantidadDeClientesMaxima = 2;
	usuarios = new AsignadorDeUsuarios(cantidadDeClientesMaxima);
	seDebeCerrarElServidor = false;
	int cantidadDeClientes = 0;
}

MainServidor::~MainServidor(){
	instanceFlag = false;
}

MainServidor* MainServidor::getInstance(){
	if(! instanceFlag){
		single = new MainServidor();
		instanceFlag = true;
		return single;
	}
	else{
		return single;
	}
}

SOCKET MainServidor::obtenerSocketInicializado(sockaddr_in &local){
	WSADATA wsa;
	SOCKET sock;
	//Inicializamos
	WSAStartup(MAKEWORD(2,0),&wsa);
	//Creamos el socket
	sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	//defnimos direcci�n por defecto, ipv4 y el puerto 9999
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(9999);
	//asociamos el socket al puerto
	if (bind(sock, (SOCKADDR*) &local, sizeof(local))==-1){
		printf("error en el bind\n");
	}
	return sock;
}

void MainServidor::ponerAEscuchar(SOCKET sock){
	if (listen(sock,2)==-1){
		printf("error en el listen\n");
	}
}
int MainServidor::fun_atenderCliente(void* punteroAlSocketRecibido){
	MainServidor * instan = MainServidor::getInstance();
	return instan->atenderCliente(punteroAlSocketRecibido);	
}

int MainServidor::fun_recibirConexiones(void* punteroAlSocketRecibido){
	MainServidor * instan = MainServidor::getInstance();
	return instan->recibirConexiones(punteroAlSocketRecibido);	
}

int MainServidor::revisarSiHayMensajesParaElClienteYEnviarlos(void* structPointer)
{
	StructDelEnviadorDeMensajes structRecibido = *((StructDelEnviadorDeMensajes*) structPointer);
	IdYPunteroAlSocket idYPunteroAlSocket = structRecibido.idYPunteroAlSocket;
	//idYPunteroAlSocket es igual a la direccion de memoria apuntada por el puntero recibido
	SOCKET socket = *idYPunteroAlSocket.punteroAlSocket;
	//el socket es igual a la direccion apuntada por el punteroAlSocket
	int id = idYPunteroAlSocket.id;
	std::queue<char*>* colaDeMensajesParaEnviar;
	char* mensaje;
	bool* seCerroLaConexionPointer = structRecibido.seCerroLaConexion;
	colaDeMensajesParaEnviar = usuarios->obtenerColaDeUsuario(id);
	printf("Se esta preparado para enviar mensajes al usuario: %i\n",id); 
	while(!(*seCerroLaConexionPointer)){
		if(!colaDeMensajesParaEnviar->empty()){
			mensaje = colaDeMensajesParaEnviar->front();
			SDL_mutexP(mut);
			colaDeMensajesParaEnviar->pop();
			SDL_mutexV(mut);
			send(socket, mensaje, strlen(mensaje), 0 );
			//Aca deber�a liberar la memoria del mensaje, pero si lo hago estalla.
			//Y efectivamente si mando muchos mensajes (Con un solo cliente abierto), la memoria aumenta, asi que 
			// la estamos perdiendo con los mensajes
		}
	}
    return 0;
}

int MainServidor::fun_revisarSiHayMensajesParaElClienteYEnviarlos(void* idYPunteroAlSocketRecibido){
	MainServidor * instan = MainServidor::getInstance();
	return instan->revisarSiHayMensajesParaElClienteYEnviarlos(idYPunteroAlSocketRecibido);	
}

int MainServidor::fun_consolaDelServidor(void* punteroAlSocketRecibido){
	MainServidor * instan = MainServidor::getInstance();
	return instan->consolaDelServidor(punteroAlSocketRecibido);	
}
/*
int MainServidor::atenderCliente(void* punteroAlSocketRecibido)
{
	int len;
	//	SOCKET socket;
	char Buffer[1024];


	SOCKET* punteroAlSocket = (SOCKET*)punteroAlSocketRecibido;
	len=sizeof(struct sockaddr);
	while (len!=0 && !seDebeCerrarElServidor){ //mientras estemos conectados con el otro pc
		len=recv(*punteroAlSocket,Buffer,1023,0); //recibimos los datos que envie
		//printf("BUG-000");
		if (len>0){
			//si seguimos conectados
			Buffer[len]=0; //le ponemos el final de cadena
			SDL_mutexP(mut);
			//productor
			colaDeMensaje.push(Buffer);
			printf("Texto recibido:%s\n",Buffer); //imprimimos la cadena recibida
			SDL_mutexV(mut);
		}
	}
	if(seDebeCerrarElServidor){
		closesocket(*punteroAlSocket);
		WSACleanup();
	}
	cantidadDeClientes--;
	printf("La cantidad de clientes conectados es: %i\n", cantidadDeClientes); 
	return 0;
}*/

void MainServidor::guardarElMensajeEnLaColaPrincipal(char* buffer, int id){

	SDL_mutexP(mut);
	MensajeConId* mensajeConId = new MensajeConId;
	mensajeConId->id = id;	
	mensajeConId->mensaje = buffer;
	colaDeMensaje.push(mensajeConId);
	SDL_mutexV(mut);
}

int MainServidor::atenderCliente(void* idYPunteroAlSocketRecibido)
{
	int len;
	char Buffer[1024];
	SDL_mutex *mut;
	SDL_Thread* threadDeEnvioDeMensajes;
	mut=SDL_CreateMutex();
	StructDelEnviadorDeMensajes* structParaEnviar = new StructDelEnviadorDeMensajes;
	bool* seCerroLaConexion = new bool;
	//Si bien seCerroLaConexion es algo que se accede de dos threads y puede ser bloqueado con un semaforo
	//No es relevante, una desincronizacion lleva a que se cicle un par de veces dem�s en el otro thread
	//se perdera mas tiempo bloqueandolo.
	*seCerroLaConexion = false;
	//idYPunteroAlSocket es igual a la direccion de memoria apuntada por el puntero recibido
	IdYPunteroAlSocket idYPunteroAlSocket = *((IdYPunteroAlSocket*) idYPunteroAlSocketRecibido);
	SOCKET socket = *idYPunteroAlSocket.punteroAlSocket;
	int id = idYPunteroAlSocket.id;
	len=sizeof(struct sockaddr);
	structParaEnviar->idYPunteroAlSocket = idYPunteroAlSocket;
	structParaEnviar->seCerroLaConexion = seCerroLaConexion;
	threadDeEnvioDeMensajes =
		SDL_CreateThread(MainServidor::fun_revisarSiHayMensajesParaElClienteYEnviarlos, "mensajesParaElCliente", (void*) structParaEnviar);
   	while (len != 0 && !seDebeCerrarElServidor){ //mientras estemos conectados con el otro pc
		len=recv(socket,Buffer,1023,0); //recibimos los datos que envie
		if (len>0){
		 //si seguimos conectados
			Buffer[len]=0; //Ponemos el fin de cadena 
			guardarElMensajeEnLaColaPrincipal(Buffer, id);
		}

	}
	*seCerroLaConexion = true;
	if(seDebeCerrarElServidor){
		closesocket(socket);
	    WSACleanup();
	}
	SDL_WaitThread(threadDeEnvioDeMensajes, NULL);
	delete seCerroLaConexion;
	usuarios->eliminarUsuario(id);
	printf("La cantidad de clientes conectados es: %i\n",usuarios->cantidadDeUsuarios()); 
    return 0;
}
//----------------------------------------------------------------------------
void freeSockets (SOCKET* s) {  // libero la memoria de los sockets
  free(s);
}
void waitThread (SDL_Thread* h) {  // wait para todos los threadsockets
	printf("wait al Thread \n");
  SDL_WaitThread(h, NULL);
}
//----------------------------------------------------------------------------
/*
int MainServidor::recibirConexiones(void*){
	struct sockaddr_in local;

	SOCKET* socketConexion;
	int len;
	len=sizeof(struct sockaddr);//Si no pongo esto no funciona, queda para futuras generaciones descubrir porque.

	socketDeEscucha = obtenerSocketInicializado(local);
	ponerAEscuchar(socketDeEscucha);

	printf("[Cuando se vaya recibiendo texto aparecera en pantalla]\n");
	do{
		//printf("BUG-001\n");
		if(cantidadDeClientes < cantidadDeClientesMaxima){ 
			socketConexion=(SOCKET*)malloc(sizeof(SOCKET)); // se usa malloc porque de otra forma siempre usas el mismo socket
			*socketConexion=accept(socketDeEscucha,(sockaddr*)&local,&len);
			//Problema hace un ingreso innecesario.
			cantidadDeClientes++;
			printf("La cantidad de clientes conectados es: %i\n", cantidadDeClientes); 
			void* punteroAlSocket = socketConexion;
			vectorHilos.push_back(SDL_CreateThread(MainServidor::fun_atenderCliente, "atenderAlCliente", punteroAlSocket));
			vectorSockets.push_back(socketConexion);
			// colaSockets.push(socketConexion);
			// algun contendor para los hilos que se crean			
		}
	}while(!seDebeCerrarElServidor);
	for_each (vectorHilos.begin(), vectorHilos.end(), waitThread);
	//liberar memoria de los sockets
	for_each (vectorSockets.begin(), vectorSockets.end(), freeSockets);
	return 0;
}*/
int MainServidor::recibirConexiones(void*){
	struct sockaddr_in local;

	SOCKET* socketConexion;
	int len;
	len=sizeof(struct sockaddr);//Si no pongo esto no funciona, queda para futuras generaciones descubrir porque.

	IdYPunteroAlSocket idYPunteroAlSocket;
	socketDeEscucha = obtenerSocketInicializado(local);
	ponerAEscuchar(socketDeEscucha);

	printf("[Cuando se vaya recibiendo texto aparecera en pantalla]\n");
	do{
		if(usuarios->puedoTenerMasUsuarios()){ 
			socketConexion=(SOCKET*)malloc(sizeof(SOCKET)); // se usa malloc porque de otra forma siempre usas el mismo socket
			printf("En espera de conexiones\n"); 
			*socketConexion=accept(socketDeEscucha,(sockaddr*)&local,&len);
			printf("Nueva conexion aceptada\n"); 
			// aca chequear los errores por si desconectamos el servidor, cerrando su conexion
			idYPunteroAlSocket.id = usuarios->crearUsuarioYObtenerId();
			printf("La cantidad de clientes conectados es: %i\n",usuarios->cantidadDeUsuarios()); 
			printf("La id del nuevo usuario es: %i\n",idYPunteroAlSocket.id); 
			if(usuarios->puedoTenerMasUsuarios()){
				printf("Todavia se pueden tener mas usuarios\n");
			}else{
				printf("Se ha alcanzado el limite de usuarios");
			}
			idYPunteroAlSocket.punteroAlSocket = socketConexion;
			vectorHilos.push_back(SDL_CreateThread(MainServidor::fun_atenderCliente, "atenderAlCliente", (void*) &idYPunteroAlSocket));
			vectorSockets.push_back(socketConexion);
			// colaSockets.push(socketConexion);
			// algun contendor para los hilos que se crean			
		}
	}while(!seDebeCerrarElServidor);
	for_each (vectorHilos.begin(), vectorHilos.end(), waitThread);
	//liberar memoria de los sockets
	for_each (vectorSockets.begin(), vectorSockets.end(), freeSockets);
	return 0;
}

int MainServidor::consolaDelServidor(void*){
	char entradaTeclado[20];
	do{
		scanf("%s", entradaTeclado);
	}while(strcmp(entradaTeclado,"terminar"));
	seDebeCerrarElServidor = true;
	//cuando cierro la conexion del socketDeEscucha, se crea igual un hilo, no controlo eso.
	closesocket(socketDeEscucha);
	return 0;
}

int MainServidor::mainPrincipal(){
	mut=SDL_CreateMutex();
	MensajeConId* mensajeConId;
	printf("Escriba terminar si desea cerrar el servidor\n", usuarios->cantidadDeUsuarios()); 

	SDL_Thread* receptor=SDL_CreateThread(MainServidor::fun_recibirConexiones, "recibirConexiones", NULL);
	SDL_Thread* consola=SDL_CreateThread(MainServidor::fun_consolaDelServidor, "recibirConexiones", NULL);

	while(!seDebeCerrarElServidor){
		//printf("BUG-002");
		SDL_mutexP(mut);
		if(!colaDeMensaje.empty()){
			//consumidor
			std::queue<char*>* colaDeMensajesDelUsuario;
			mensajeConId = colaDeMensaje.front();
			colaDeMensaje.pop();
			printf("Recibido del usuario:%i", mensajeConId->id);
			printf(" el mensaje:%s\n",mensajeConId->mensaje);
			colaDeMensajesDelUsuario = usuarios->obtenerColaDeUsuario(mensajeConId->id);
			char* mensajeDeRespuesta = new char;
			mensajeDeRespuesta = "Llego todo bien";
			SDL_mutexP(mut);
			colaDeMensajesDelUsuario->push(mensajeDeRespuesta);
			SDL_mutexV(mut);
			delete mensajeConId;
		}
		SDL_mutexV(mut);
		SDL_Delay(100);//No quiero tener permanentemente bloqueada la cola para revisar si llego algo.
	}
	SDL_WaitThread(receptor, NULL);
	SDL_WaitThread(consola, NULL);
	SDL_DestroyMutex(mut);
	SDL_Delay(20000);
	return 0;
}
