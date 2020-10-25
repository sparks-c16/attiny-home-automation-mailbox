# attiny-mailbox-automation
Boîte à lettres domotique autour d'un ATtiny85

On going

## Fonctionnalités

- Détecteur d'ouverture portes avant et arrière, et fenêtre
- Détecteur d'intrusion (ouverture du boitier contenant le circuit)
- Mesure de la tension de la batterie
- Emission de signaux RF 433MHz dans un protocole reconnu par les box domotiques
- La box domotique signalant le passage du facteur par un SMS


## Liste des composants

- 1 microcontroleur ATtiny85
- 1 résistance de 
- 1 condensateur de 100 nF   
- 3 capteurs Reed (ouvert au repos)
- 3 petits aimants Neodyne
- 1 convertisseur DC-DC Boost 5V en sortie
- 1 module radio émetteur 433 MHz
- 1 micro switch 3 bornes
- 1 accumulateur LiPo 18650
- 1 support de batterie
- 1 boitier étanche
- connecteurs
- fils

## Prototypage


<img src="./resources/attiny-mailbox-automation.svg">



## Schéma

...


## Principes

- Pour économiser la batterie, le microcontrôleur est mis en veille avec un watchdog de 8s (maximum)
- Entre deux mises en veille, mesure de la tension de la baterie, zéro volt indiquant l'ouverture du boitier à cause du changement d'état du micro switch
- Réveil sur changement d'état (interruption) sur les entrées reliées aux capteurs Reed (niveau bas indiquant l'ouverture d'une porte ou de la fenêtre, les pins du µC  sont configurés en entrée avec PULLUP)



## Réalisation

...


## Pistes d'amélioration

- Économie d'énergie en utilisant la dernière broche du microcontrôleur pour piloter la mise en veille (ou l'alimentation de l'émetteur avec un mosfet)
- Sortir l'antenne du boitier pour une meilleur réception côté box domotique
- Rechargement de la batterie via un panneau solaire
