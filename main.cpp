#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <thread>
#include <chrono>
#include <SFML/Network.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ostream>
#include <istream>
#include <string>
#include "httplib.h"
#include <thread>
#include <future>
#include <SFML/Window/Event.hpp>
#include <shtypes.h>
#include <shlobj.h>


using namespace sf;
using namespace std;
using namespace httplib;

Sprite buttonSprite;
string versionEnServidor;
string nombreImagenActual;
RenderWindow ventana; // Ventana global para la selección de la carpeta
string carpetaSeleccionada; // Variable para almacenar la carpeta seleccionada


// Función para eliminar espacios en blanco al principio y al final de una cadena
string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

// Función para verificar actualizaciones
bool verificarActualizacion() {
    Http servidor("http://webhook.ponchisao326.cf");
    Http::Request solicitud("/version.txt");
    Http::Response respuesta = servidor.sendRequest(solicitud);

    if (respuesta.getStatus() == sf::Http::Response::Ok) {
        // Obtener la primera línea del cuerpo de la respuesta
        std::istringstream iss(respuesta.getBody());
        std::string versionEnServidor;
        if (std::getline(iss, versionEnServidor)) {
            versionEnServidor = trim(versionEnServidor); // Aplicar trim a la primera línea
        }

        // Leer la versión local almacenada en un archivo
        std::ifstream archivoVersionLocal("update/version.txt");
        std::string versionLocal;
        if (archivoVersionLocal.is_open()) {
            std::getline(archivoVersionLocal, versionLocal);
            archivoVersionLocal.close();
        }

        if (!versionEnServidor.empty() && versionEnServidor != versionLocal) {
            return true; // Devolver true si las versiones son diferentes
        }
    }

    return false;
}

// Función para verificar el tipo de actualización
string verificarTipoActualizacion() {
    sf::Http servidor("http://webhook.ponchisao326.cf");
    sf::Http::Request solicitud("/type.txt");
    sf::Http::Response respuesta = servidor.sendRequest(solicitud);

    if (respuesta.getStatus() == sf::Http::Response::Ok) {
        string linea;
        istringstream iss(respuesta.getBody());
        while (getline(iss, linea)) {
            versionEnServidor = trim(linea); // Aplicar trim a cada línea
            if (versionEnServidor == "resourcepack") {
                return "resourcepack"; // Devolver "resourcepack" si se encuentra en alguna línea
            }
        }

        return "client"; // Devolver "client" si no se encuentra "resourcepack" en ninguna línea
    }

    cerr << "No ha sido posible conectarse al servidor tipo" << endl;
    return "error";
}

class Descargador {
public:
    Descargador() {}

    void Descargar() {

        Client cliente("webhook.ponchisao326.cf");
        auto respuesta = cliente.Get("/downloads/blanchiniiland.zip");

        if (respuesta && respuesta->status == 200) {
            ofstream archivo("blanchiniiland.zip", ios::binary);
            archivo.write(respuesta->body.c_str(), respuesta->body.length());
            archivo.close();
            cout << "Archivo descargado correctamente." << endl;
        } else {
            cerr << "Error al descargar el archivo." << endl;
        }

    }
};

class TextField : public Drawable {
public:
    TextField() : selected(false) {
        Font fuente;
        if (!fuente.loadFromFile("fonts/minecraft.otf")) {
            cerr << "No se pudo cargar la fuente." << endl;
        }
        text.setFont(fuente);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::Black);
    }

    void setFont(const sf::Font& font) {
        text.setFont(font);
        text.setFillColor(sf::Color::Black);
    }

    void setPosition(float x, float y) {
        text.setPosition(x, y);
        shape.setPosition(x, y);
    }

    void setSize(const sf::Vector2f& size) {
        shape.setSize(size);
    }

    void setCharacterSize(unsigned int size) {
        text.setCharacterSize(size);
    }

    void setFillColor(const sf::Color& color) {
        text.setFillColor(color);
    }

    void processEvent(const sf::Event& event) {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
            if (shape.getGlobalBounds().contains(mousePos)) {
                selected = true;
            } else {
                selected = false;
            }
        }

        if (selected && event.type == sf::Event::TextEntered) {
            if (event.text.unicode == 8 && !textString.empty()) { // Tecla retroceso
                textString.pop_back();
            } else if (event.text.unicode >= 32 && event.text.unicode <= 126) { // Caracteres imprimibles
                textString += static_cast<char>(event.text.unicode);
            }
            text.setString(textString);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(text);
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        // Dibujar el campo de texto utilizando la fuente y el texto configurados en tu clase TextField
        target.draw(text, states);

        // Puedes agregar más elementos de dibujo personalizado aquí si es necesario
    }

    const std::string& getText() const {
        return textString;
    }

private:
    sf::Text text;
    sf::RectangleShape shape;
    std::string textString;
    bool selected;
};

class CarpetaDialog {
public:
    CarpetaDialog() {
        ventana.create(sf::VideoMode(400, 200), "Seleccionar Carpeta", sf::Style::Close);
        ventana.setFramerateLimit(30);

        if (!fuente.loadFromFile("fonts/minecraft.otf")) {
            cerr << "No se pudo cargar la fuente." << endl;
        }

        texto.setFont(fuente);
        texto.setCharacterSize(16);
        texto.setFillColor(sf::Color::Black);
        texto.setPosition(20, 20);
        texto.setString("Por favor, ingrese la ruta de la carpeta o selecciónela:");

        campoTexto.setFont(fuente); // Pasa la fuente al TextField
        campoTexto.setCharacterSize(16);
        campoTexto.setFillColor(sf::Color::Black);
        campoTexto.setPosition(20, 60);
        campoTexto.setSize(sf::Vector2f(280, 30)); // Tamaño del campo de entrada de texto

        ventana.setVisible(false);
    }

    void mostrarDialogoSeleccionCarpeta() {
        carpetaSeleccionada = "";
        sf::RenderWindow dialogo(sf::VideoMode(400, 200), "Seleccionar Carpeta", sf::Style::Close);
        dialogo.setFramerateLimit(30);

        while (dialogo.isOpen()) {
            sf::Event evento;
            while (dialogo.pollEvent(evento)) {
                if (evento.type == sf::Event::Closed) {
                    dialogo.close();
                }

                if (evento.type == sf::Event::KeyPressed) {
                    if (evento.key.code == sf::Keyboard::Return) {
                        carpetaSeleccionada = trim(campoTexto.getText());
                        if (!carpetaSeleccionada.empty()) {
                            dialogo.close();
                        }
                    }
                }

                campoTexto.processEvent(evento); // Procesar eventos para el campo de entrada de texto
            }

            dialogo.clear(sf::Color::White);
            dialogo.draw(texto);
            campoTexto.draw(dialogo); // Dibujar el campo de entrada de texto en la ventana
            dialogo.display();
        }
    }

private:
    sf::RenderWindow ventana;
    sf::Font fuente;
    sf::Text texto;
    TextField campoTexto; // Campo de entrada de texto
};

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

string mostrarDialogoSeleccionCarpeta(const string& direccionInicial = "") {
    string carpetaSeleccionada;

    char buffer[MAX_PATH];
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = "Seleccionar Carpeta";
    bi.pszDisplayName = buffer;
    if (!direccionInicial.empty()) {
        bi.lpfn = BrowseCallbackProc;
        bi.lParam = reinterpret_cast<LPARAM>(direccionInicial.c_str());
    }

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != 0) {
        SHGetPathFromIDList(pidl, buffer);
        carpetaSeleccionada = buffer;
        CoTaskMemFree(pidl);
    }

    return carpetaSeleccionada;
}

// Definición de la función BrowseCallbackProc
int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if (uMsg == BFFM_INITIALIZED) {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

int main() {

    RenderWindow ventana(VideoMode::getFullscreenModes()[0], "Blanchiniiland Launcher", Style::Fullscreen);
    ventana.setFramerateLimit(60);

    Image icono;
    if (icono.loadFromFile("images/icon.png")) {
        ventana.setIcon(icono.getSize().x, icono.getSize().y, icono.getPixelsPtr());
    }

    Vector2u ventanaSize = ventana.getSize();
    vector<Texture> texturas;

    // Pantalla de carga
    Texture cargaTextura;
    if (cargaTextura.loadFromFile("images/Loading/loading.png")) {
        Sprite cargaSprite(cargaTextura);
        cargaSprite.setPosition((ventanaSize.x - cargaTextura.getSize().x) / 2,
                                (ventanaSize.y - cargaTextura.getSize().y) / 2);
        ventana.draw(cargaSprite);
        ventana.display();
    }

    for (int i = 1; i <= 156; ++i) {
        Texture textura;
        if (textura.loadFromFile("images/Animation/frame (" + to_string(i) + ").jpg")) {
            texturas.push_back(textura);
        }
    }

    Sprite sprite;
    int fotogramaActual = 0;
    float tiempoPorFotograma = 0.05;
    Clock reloj;

    Music musica;
    if (!musica.openFromFile("background/background.ogg")) {
        return 1;
    }

    // Ocultar la pantalla de carga
    ventana.clear();
    ventana.display();

    musica.setLoop(true);
    musica.setVolume(25);

    ventana.clear();
    musica.play();

    // Imagen personalizada para el botón "Exit"
    Texture exitTexture;
    if (!exitTexture.loadFromFile("images/frontend/cross.png")) {
        return 1;
    }

    Texture loginTexture;
    if (!loginTexture.loadFromFile("images/frontend/login.png")) {
        return 1;
    }

    Sprite loginButton;
    loginButton.setTexture(loginTexture);
    loginButton.setPosition(10, 10);

    Sprite exitButton;
    exitButton.setTexture(exitTexture);
    exitButton.setPosition(ventanaSize.x - exitButton.getGlobalBounds().width - 10, 10);

    // Fuente para el texto "Launcher by Ponchisao"
    Font font;
    if (!font.loadFromFile("fonts/minecraft.otf")) {
        return 1;
    }

    Text ponchisaoText("Launcher by Ponchisao326", font, 26);
    ponchisaoText.setFillColor(Color::White);
    ponchisaoText.setPosition(10, ventanaSize.y - 50);

    // Cargar cursores personalizados (flecha y mano)
    Texture arrowCursorTexture;
    if (!arrowCursorTexture.loadFromFile("images/cursors/normal.png")) {
        return 1;
    }

    Cursor arrowCursor;
    arrowCursor.loadFromPixels(arrowCursorTexture.copyToImage().getPixelsPtr(), {32, 32}, {0, 0});

    Texture handCursorTexture;
    if (!handCursorTexture.loadFromFile("images/cursors/link.png")) {
        return 1;
    }
    Cursor handCursor;
    handCursor.loadFromPixels(handCursorTexture.copyToImage().getPixelsPtr(), {32, 32}, {0, 0});

    // Crear un botón para "Launch" o "Update" con sprites
    Texture launchTexture;
    if (!launchTexture.loadFromFile("images/frontend/launch.png")) {
        return 1;
    }

    Texture updateTexture;
    if (!updateTexture.loadFromFile("images/frontend/update.png")) {
        return 1;
    }

    Texture errorTexture;
    if (!errorTexture.loadFromFile("images/frontend/error.png")) {
        return 1;
    }

    if (!verificarActualizacion()) {
        buttonSprite.setTexture(launchTexture);
        nombreImagenActual = "launch.png";
    } else {
        buttonSprite.setTexture(updateTexture);
        nombreImagenActual = "update.png";
    }

    buttonSprite.setPosition((ventanaSize.x - buttonSprite.getGlobalBounds().width) / 2, ventanaSize.y - 400);

    Texture imagenSuperiorTexture;
    if (!imagenSuperiorTexture.loadFromFile("images/frontend/blanchiniiland.png")) {
        return 1;
    }

    Sprite imagenSuperiorSprite(imagenSuperiorTexture);

    Vector2u imagenSize = imagenSuperiorTexture.getSize();

    imagenSuperiorSprite.setPosition((ventanaSize.x - imagenSize.x) / 2 + 40, 0);

    ifstream minecraftDir1("minecraft.txt");
    if (minecraftDir1.is_open()) {
        string linea;
        getline(minecraftDir1, linea);


        if (!linea.empty()) {
            carpetaSeleccionada = linea;
            minecraftDir1.close();
        } else {
            do {

                string carpetaInicial = "C:\\Program Files (x86)\\Minecraft Launcher";
                CarpetaDialog carpetaDialog;
                carpetaSeleccionada = mostrarDialogoSeleccionCarpeta(
                        carpetaInicial); // Llamada a tu función para seleccionar carpeta

                ofstream minecraftDir("minecraft.txt");
                if (!carpetaSeleccionada.empty()) {
                    if (minecraftDir.is_open()) {
                        minecraftDir << carpetaSeleccionada << endl;
                        minecraftDir.close();
                        break; // Sal del bucle si se eligió una carpeta
                    } else {
                        cout << "No se puede abrir el archivo." << endl;
                    }
                }
            } while (true);
        }
    }

    while (ventana.isOpen()) {
        Event evento;
        while (ventana.pollEvent(evento)) {
            if (evento.type == Event::Closed)
                ventana.close();

            if (evento.type == Event::MouseButtonPressed) {
                if (evento.mouseButton.button == Mouse::Left) {
                    Vector2i mousePos = Mouse::getPosition(ventana);
                    if (loginButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                               static_cast<float>(mousePos.y))) {
                    } else if (ponchisaoText.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                                        static_cast<float>(mousePos.y))) {
                        system("start https://ponchisao326.cf");
                        system("start https://instagram.com/ponchisao326_");
                    } else if (buttonSprite.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                                       static_cast<float>(mousePos.y))) {
                        if (nombreImagenActual == "update.png") {
                            string type = verificarTipoActualizacion();
                            if (type == "resourcepack") {
                                // Descargar el archivo de versión desde el servidor
                                Http servidor("http://webhook.ponchisao326.cf");
                                Http::Request versionRequest("/version.txt");
                                Http::Response versionResponse = servidor.sendRequest(versionRequest);

                                if (versionResponse.getStatus() == sf::Http::Response::Ok) {
                                    // Guardar la versión del servidor en el archivo local
                                    std::ofstream localVersionFile("update/version.txt");
                                    if (localVersionFile.is_open()) {
                                        localVersionFile << versionResponse.getBody();
                                        localVersionFile.close();
                                    } else {
                                        cerr << "No se pudo abrir el archivo de versión local." << endl;
                                    }

                                    // Ejecutar acciones para solo actualizar el resource pack en un hilo separado
                                    Descargador descargador;
                                    std::future<void> result = std::async(std::launch::async, [&descargador]() {
                                        descargador.Descargar();
                                    });

                                    // Esperar a que la descarga termine
                                    result.wait();

                                    // Cambiar nuevamente el estado de la descarga y mostrar el botón "Download"
                                    buttonSprite.setTexture(launchTexture);
                                    nombreImagenActual = "launch.png";
                                } else if (type == "client") {
                                    // Cosas que hacer cuando ponga cliente
                                } else {
                                    buttonSprite.setTexture(errorTexture);
                                    buttonSprite.setPosition((ventanaSize.x - buttonSprite.getGlobalBounds().width) / 2,
                                                             ventanaSize.y - 600);
                                }
                            }
                        } else if (nombreImagenActual == "launch.png") {
                            ventana.close();
                            string launcherPath = carpetaSeleccionada + "\\MinecraftLauncher.exe";
                            string comando = "\"" + launcherPath + "\"";

                            int resultado = system(comando.c_str());

                            if (resultado == 0) {
                                cout << "Launcher de Minecraft iniciado con éxito." << endl;
                            } else {
                                cerr << "Error al iniciar el launcher de Minecraft." << endl;
                            }
                        }
                    } else if (exitButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                                     static_cast<float>(mousePos.y))) {
                        ventana.close();
                    }
                }

                if (evento.type == Event::MouseMoved) {
                    Vector2i mousePos = Mouse::getPosition(ventana);
                    if (exitButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                              static_cast<float>(mousePos.y))) {
                        ventana.setMouseCursor(handCursor);
                    } else if (ponchisaoText.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                                        static_cast<float>(mousePos.y))) {
                        ventana.setMouseCursor(handCursor);
                    } else if (buttonSprite.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                                       static_cast<float>(mousePos.y))) {
                        ventana.setMouseCursor(handCursor);
                    } else if (loginButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                                                      static_cast<float>(mousePos.y))) {
                        ventana.setMouseCursor(handCursor);
                    } else {
                        ventana.setMouseCursor(arrowCursor);
                    }
                }
            }

            if (reloj.getElapsedTime().asSeconds() >= tiempoPorFotograma) {
                sprite.setTexture(texturas[fotogramaActual]);
                fotogramaActual = (fotogramaActual + 1) % texturas.size();
                reloj.restart();
            }

            ventana.draw(sprite);
            ventana.draw(exitButton);
            ventana.draw(ponchisaoText);
            ventana.draw(buttonSprite);
            ventana.draw(loginButton);
            ventana.draw(imagenSuperiorSprite);
            ventana.display();
        }
    }

    return 0;
}