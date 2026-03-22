-- Sentencias temporales de prueba generadas con IA
-- =============================================
-- DATOS DE PRUEBA
-- =============================================

-- ESTACIONES
INSERT INTO ESTACION (id_estacion, nombre, direccion, capacidad_max, disponibilidad_actual) VALUES
                                                                                                (1, 'Estación Centro',      'Calle Mayor 1, Madrid',          20, 14),
                                                                                                (2, 'Estación Norte',       'Av. de la Paz 45, Madrid',       15, 10),
                                                                                                (3, 'Estación Sur',         'Calle Granada 88, Madrid',       10,  6),
                                                                                                (4, 'Estación Este',        'Paseo del Prado 200, Madrid',    25, 20),
                                                                                                (5, 'Estación Aeropuerto',  'Terminal T4, Barajas, Madrid',   30,  5);

-- =============================================
-- USUARIOS
INSERT INTO USUARIO (id_usuario, dni, nombre, contrasena, saldo) VALUES
                                                                     (1, '12345678A', 'Ana García López',      '$2b$12$KIXab1hW1uB3V3mT4QkXCeYwB9Oz0L1', 50.00),
                                                                     (2, '87654321B', 'Carlos Martínez Ruiz',  '$2b$12$9mPqL2wT8xN7vR4sJ6KdOuZcF0Hn3M5', 120.50),
                                                                     (3, '11223344C', 'Laura Sánchez Pérez',   '$2b$12$3nQrM5yU9zP8wS2tK7LePvYdG1Ij4N6', 0.00),
                                                                     (4, '44332211D', 'Miguel Torres Díaz',    '$2b$12$7pRsN8aV0bQ9xT3uL4MfOwZeH2Jk5O7', 200.75),
                                                                     (5, '55667788E', 'Elena Romero Castro',   '$2b$12$1qStO9bW1cR0yU4vM5NgPxAeI3Kl6P8', 35.20);

-- =============================================
-- VEHÍCULOS
-- tipo: B=Bicicleta, P=Patinete
-- estado: D=Disponible, R=Rentado, M=Mantenimiento, B=Bloqueado
INSERT INTO VEHICULO (id_vehiculo, tipo, bateria, estado, id_estacion) VALUES
-- En Estación Centro (14 disponibles)
( 1, 'B', 95.0, 'D', 1),
( 2, 'B', 80.5, 'D', 1),
( 3, 'P', 100.0,'D', 1),
( 4, 'P', 60.0, 'D', 1),
-- En Estación Norte (10 disponibles)
( 5, 'B', 45.0, 'D', 2),
( 6, 'B', 90.0, 'D', 2),
( 7, 'P', 75.0, 'D', 2),
-- En Estación Sur (6 disponibles)
( 8, 'B', 30.0, 'D', 3),
( 9, 'P', 85.0, 'D', 3),
-- En Estación Este (20 disponibles)
(10, 'B', 100.0,'D', 4),
(11, 'P', 55.0, 'D', 4),
(12, 'B', 70.0, 'D', 4),
-- En Estación Aeropuerto (5 disponibles)
(13, 'P', 20.0, 'D', 5),
-- Vehículos en alquiler activo (sin estación)
(14, 'B', 65.0, 'R', NULL),
(15, 'P', 40.0, 'R', NULL),
-- Vehículos en mantenimiento o bloqueados
(16, 'B', 10.0, 'M', NULL),
(17, 'P',  0.0, 'B', NULL);

-- =============================================
-- ALQUILERES
-- Alquileres finalizados
INSERT INTO ALQUILER (id_alquiler, id_usuario, id_vehiculo, id_estacion_origen, id_estacion_destino, fecha_inicio, fecha_fin, coste_total) VALUES
                                                                                                                                               (1, 1,  3, 1, 2, '2025-03-10 08:00:00', '2025-03-10 08:45:00',  2.50),
                                                                                                                                               (2, 2,  7, 2, 4, '2025-03-11 09:15:00', '2025-03-11 10:00:00',  3.75),
                                                                                                                                               (3, 3,  1, 1, 3, '2025-03-12 07:30:00', '2025-03-12 08:10:00',  2.00),
                                                                                                                                               (4, 4, 10, 4, 1, '2025-03-13 18:00:00', '2025-03-13 18:30:00',  1.50),
                                                                                                                                               (5, 5,  9, 3, 5, '2025-03-14 12:00:00', '2025-03-14 13:00:00',  5.00),
                                                                                                                                               (6, 1,  6, 2, 2, '2025-03-15 10:00:00', '2025-03-15 10:20:00',  1.00),
-- Alquileres activos (sin fecha_fin ni estación destino)
                                                                                                                                               (7, 2, 14, 1, NULL, '2025-03-22 09:00:00', NULL, 0.00),
                                                                                                                                               (8, 4, 15, 3, NULL, '2025-03-22 10:30:00', NULL, 0.00);