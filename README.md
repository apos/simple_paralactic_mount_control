
# Einfache Steuerung einer parallaktischen Montierung

* Arduino UNO 
* L298H Dual H Bridge Driver
* DS3231 AT24C32 Real Time Clock Module IIC 
* Stepper

## Status /Features

* Besonderheit: IRQ-Steuerung zur Verwendung einer Real Time Clock (Beta) korrigiert Ungenauigkeiten des Arduino
* manuelle Steuerung über Buttons (nur Rektaszenion - West / East) möglich. Geschwindigkeit über 3. Button (Stufen 1-4)


## Berechnung der Steps / Microsteps 

Mittlere Sternzeit: 86164,091 sec
An einem Tag wird in RA um 360° bewegt, in einer Stunde um 15°, in vier Minuten um 1°, in einer Minute um 0,25°, in einer Sekunde um 0,00417°

Eine volle Umdrehung	 360°		in		86164,091 sec
x 2,5    Schnecke [1]	 900°		in		86164,091 sec 
x 7,8125 Zahnriemen		7031,25°	in		86164,091 sec

=> am Motor müssen 7031,25° bewältigt werden, um an der RA-Achse eine volle Umdrehung zu bewerkstelligen.


## Pulleys and Belts

Zur Berechnung verwendet:
* https://www.bbman.com/belt-length-calculator/

### 180mm / 90T
Speed Ratio 0.32 ( 3,125:1 )
Pitch Diameter - Large Pulley (A) (mm)  31.8 (50 teeth)
Pitch Diameter - Small Pulley (B) (mm)  10.2 (16 teeth)
Next Available Belt Pitch Length (mm)   180
Next Available Number of Teeth - Belt   90

* Center Distance Using Next Available Belt (mm)    56.0

### 110mm / 55T
Speed Ratio 0.50 ( 2:1 )
Pitch Diameter - Large Pulley (A) (mm)  25.5 (40 teeth)
Pitch Diameter - Small Pulley (B) (mm)  12.7 (20 teeth)
Next Available Belt Pitch Length (mm)   110
Next Available Number of Teeth - Belt   55

* Center Distance Using Next Available Belt (mm)    24.2

### Umrechnung auf Grad pro Sekunde am Motor

Umrechnung für ein Grad am Motor pro Sekunde:

7031,25° [A] / 86164,091 sec [B]
__~= 0,081603019 °/sec [C]__ 


### Halfstep Modus (0,9° pro Step, 400 Steps pro Umdrehung, HALFSTEP)

Bei 0,9° für einen Microstep müssen für 1 Grad  jeweils entsprechend mehr Steps eingeplant werden:

### Umrechnung von °/sec in Steps/sec

0,081603019 °/sec  /  0.9 °/Step
__~= 0,09067002169 Steps /sec [D]__

Umrechnungsfaktor von [D] auf einen runden Wert:

E = 1 / D ~= 1,103 [E]

Umrechnung auf einen Microstep (zusätzlich Faktor 10)

0,09067002169 Steps / 1000 msec             | * ~1,103 [E]
=> 0,1  Microstep in     1102,900365 msec   |          / alle  1,1 s
__=> 1    Microstep in  11029,00365  msec   | * 10     / alle  11 s__
=> 3    Microstep in    33087,01094  msec   | * 30     / alle  33 s 
=> 10   Microsteps in  110290,0365   msec   | * 100    / alle 110 s
=> 100  Microsteps in 1102900,365    msec   | * 1000   / alle 18 min

18 Bogenminuten können als ausreichende Genauigkeit sowohl visuell als auch für die Astrofotografie angesehen werden. Die Abweichung beträgt in dieser Zeit nur 0,365 Microsecunden für 10 Steps.

- Steps mitzählen
- Millisekunden mitzählen
- Nach 110290 (100 Microsteps) und 110290 msecs (1000 Microsteps) kontrollieren und ggf. korrigieren.

if (actMillis >= 110290 ) {

}

# Legende

[1] 
*Die EQ-5 hat ein Zahnrad mit 144 Zähnen. Das entspricht*
*einer Untersetzung von 360/144 = 2,5. D.h. 10 Scheckenumdrehungen*
*entsprechen 25°.*

[A] - [D]
Variablen


