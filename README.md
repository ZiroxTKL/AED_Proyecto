pasos:

clona este repositorio
https://github.com/ZiroxTKL/AED_Proyecto.git


buscas en google Qt
https://www.qt.io/

te creas una cuenta educativa
[captura de pantalla nr1]

confirma tu creación de cuenta con el correo de confirmación de cuenta de Qt
automáticamente empezara la descarga del Qt.exe

abre el archivo exe y sigue los pasos de la instalación
[captura de pantalla nr2]

agrega la dirección de Qt en tu PATH

abre el código, ctrl + shift + P y agrega 
CMake: Edit User-Local CMake Kits.

abrirá un json y cambia todo a

[
  {
    "name": "Qt MinGW Manual",
    "compilers": {
      "C": "C:/Qt/Tools/mingw1310_64/bin/gcc.exe",
      "CXX": "C:/Qt/Tools/mingw1310_64/bin/g++.exe"
    },
    "preferredGenerator": {
      "name": "MinGW Makefiles"
    }
  }
]

guarda los cambios

luego ve a esta ruta:
C:\Qt\Tools\Ninja

copea la ruta y pegala en tu path

ahora ve a ctrl + , y busca cmake.configureEnvironment

luego agrega un nuevo item, el item llamado path y el value C:\Qt\Tools\mingw1120_64\bin;${env:PATH}



ahora ve a VSC y ve a ctrl + ,

busca CMake: Generator

escribe MinGW Makefiles en el cuadro en blanco