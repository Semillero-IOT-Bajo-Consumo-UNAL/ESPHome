# Instalacion en Linux

Para instalar el entorno de esphome

## 1. Se crea el entorno
```
python3 -m venv esp
```
## 2. Entramos al entorno
```
source esp/bin/activate
```

## 3. Instalamos las dependencias necesarias
```
pip install wheel
pip install esphome
```

## 4. finalmente para no tener que pedir permisos siempre

```
sudo usermod -a -G dialout [USUARIO]
```

en mi caso 

```
sudo usermod -a -G dialout savalderrama
```
y Reinicias tu PC. 

# Compilacion

Para compilar primero

1. Activamos el entorno 
```
source esp/bin/activate
```
2. Compilamos (ESPHome detecta los dispositivos conectados entonces no hay que especificar nada mas)
```
esphome run 1_uploadtest.yaml 
```