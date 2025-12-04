#!/bin/bash

# Script de instalación para el juego HEX

echo "=========================================="
echo "    INSTALADOR DEL JUEGO HEX"
echo "=========================================="
echo ""

# Detectar distribución
if [ -f /etc/fedora-release ]; then
   DISTRO="fedora"
   echo "Distribución detectada: Fedora"
elif [ -f /etc/lsb-release ] && grep -q "Ubuntu" /etc/lsb-release; then
   DISTRO="ubuntu"
   echo "Distribución detectada: Ubuntu"
else
   DISTRO="unknown"
   echo "Distribución no reconocida"
fi

# Verificar dependencias
echo ""
echo "Verificando dependencias..."

# GCC
if command -v gcc &> /dev/null; then
   echo "✓ GCC instalado: $(gcc --version | head -n1)"
else
   echo "✗ GCC no encontrado"
   NEED_INSTALL=1
fi

# Make
if command -v make &> /dev/null; then
   echo "✓ Make instalado: $(make --version | head -n1)"
else
   echo "✗ Make no encontrado"
   NEED_INSTALL=1
fi

# pthread (parte de glibc, siempre debería estar)
echo "✓ pthread (parte de glibc)"

# Raylib
if pkg-config --exists raylib 2>/dev/null; then
   echo "✓ Raylib instalado: $(pkg-config --modversion raylib)"
else
   echo "✗ Raylib no encontrado"
   NEED_INSTALL=1
fi

# Si necesitamos instalar algo
if [ -n "$NEED_INSTALL" ]; then
   echo ""
   echo "Se necesitan instalar dependencias."
   echo "¿Deseas instalarlas ahora? (requiere sudo) [y/N]"
   read -r response
   
   if [[ "$response" =~ ^[Yy]$ ]]; then
      if [ "$DISTRO" = "fedora" ]; then
         echo "Instalando en Fedora..."
         sudo dnf install -y gcc make raylib-devel
      elif [ "$DISTRO" = "ubuntu" ]; then
         echo "Instalando en Ubuntu..."
         sudo apt update
         sudo apt install -y gcc make libraylib-dev
      else
         echo "No se pudo instalar automáticamente."
         echo "Por favor instala manualmente:"
         echo "  - gcc"
         echo "  - make"
         echo "  - raylib-devel (Fedora) o libraylib-dev (Ubuntu)"
         exit 1
      fi
   else
      echo "Instalación cancelada."
      exit 0
   fi
fi

# Compilar el proyecto
echo ""
echo "=========================================="
echo "Compilando el proyecto..."
echo "=========================================="

if make clean && make; then
   echo ""
   echo "=========================================="
   echo "  ✓ COMPILACIÓN EXITOSA"
   echo "=========================================="
   echo ""
   echo "Ejecutables generados:"
   echo "  ./hex          - Versión con interfaz gráfica"
   echo "  ./hex_console  - Versión de consola"
   echo ""
   echo "Prueba rápida (modo consola):"
   echo "  ./hex_console -t 2 -s 100"
   echo ""
   echo "Jugar (modo gráfico):"
   echo "  ./hex -t 4 -s 1000"
   echo ""
   echo "Ver todas las opciones:"
   echo "  make help"
   echo ""
else
   echo ""
   echo "=========================================="
   echo "  ✗ ERROR EN LA COMPILACIÓN"
   echo "=========================================="
   echo "Revisa los mensajes de error anteriores."
   exit 1
fi
