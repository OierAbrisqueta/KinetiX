-- sqlite3 KinetiX Hay que ejecutar este comando antes de copiar el resto (Desde la carpeta Data, sino hay que poner data\)

PRAGMA foreign_keys = ON;

-- Tabla ESTACION
CREATE TABLE ESTACION (
                          id_estacion INTEGER PRIMARY KEY,
                          nombre VARCHAR(50) NOT NULL,
                          direccion VARCHAR(100),
                          capacidad_max INTEGER NOT NULL CHECK (capacidad_max > 0),
                          disponibilidad_actual INTEGER NOT NULL CHECK (disponibilidad_actual >= 0)
);

-- Tabla VEHICULO
CREATE TABLE VEHICULO (
                          id_vehiculo INTEGER PRIMARY KEY,
                          tipo VARCHAR(1) NOT NULL CHECK (tipo IN ('B', 'P')),
                          bateria FLOAT NOT NULL DEFAULT 100.0 CHECK (bateria >= 0 AND bateria <= 100),
                          estado VARCHAR(1) NOT NULL CHECK (estado IN ('D', 'R', 'M', 'B')),
                          id_estacion INTEGER NULL,
                          FOREIGN KEY (id_estacion) REFERENCES ESTACION(id_estacion) ON UPDATE CASCADE ON DELETE SET NULL
);

-- Tabla USUARIO
CREATE TABLE USUARIO (
                         id_usuario INTEGER PRIMARY KEY,
                         dni VARCHAR(9) NOT NULL UNIQUE,
                         nombre VARCHAR(50) NOT NULL,
                         contrasena VARCHAR(250) NOT NULL,
                         saldo DECIMAL(10,2) NOT NULL DEFAULT 0.00
);

-- Tabla ALQUILER
CREATE TABLE ALQUILER (
                          id_alquiler INTEGER PRIMARY KEY,
                          id_usuario INTEGER NOT NULL,
                          id_vehiculo INTEGER NOT NULL,
                          id_estacion_origen INTEGER NULL,
                          id_estacion_destino INTEGER NULL,
                          fecha_inicio DATETIME NOT NULL,
                          fecha_fin DATETIME NULL,
                          coste_total DECIMAL(10,2) NOT NULL DEFAULT 0.00 CHECK (coste_total >= 0.00),
                          FOREIGN KEY (id_usuario) REFERENCES USUARIO(id_usuario) ON UPDATE CASCADE ON DELETE RESTRICT,
                          FOREIGN KEY (id_vehiculo) REFERENCES VEHICULO(id_vehiculo) ON UPDATE CASCADE ON DELETE RESTRICT,
                          FOREIGN KEY (id_estacion_origen) REFERENCES ESTACION(id_estacion) ON UPDATE CASCADE ON DELETE SET NULL,
                          FOREIGN KEY (id_estacion_destino) REFERENCES ESTACION(id_estacion) ON UPDATE CASCADE ON DELETE SET NULL
);