# README

Projet en classe de ROB3 - 2021-2022 - ALONSO Benito et SHLYKOVA Olga

---

Afin de faire fonctionner les programmes de suivis de couleur ou de detection de visages et filtres vous aurez besoin des librairies/logiciels suivants:

# INSTALLATION LES LOGICIELS ET LIBRAIRIES

# 📂OpenCv

Bibliothèque pour le traitement des images en temps réel et la détection des couleurs

Installation: Commande à saisir dans le terminal : 

`sudo apt install libopencv4-dev` ou alors `sudo apt install libopencv-dev`

Vous aurez peut-être besoin de faire avant: `sudo apt-get update -y`

Pour tout information complémentaire, merci de se référer à :

[OpenCV: Introduction to OpenCV](https://docs.opencv.org/4.x/df/d65/tutorial_table_of_content_introduction.html)

Installation depuis la source (en cas de problèmes de compilation GTK, notamment un conflit entre GTK+ 2.x et GTK+ 3) :

![Untitled](README%205f1eb2ce099d4c98ae2f2346046871eb/Untitled.png)

nb: cette erreur n’apparait pas forcément sur toutes les machines, mais si elle appraît, un moyen qui normalement fonctonne est le suivant:

Installer les prérequis

```cpp
sudo apt install -y cmake g++ wget unzip
```

Sur le Bureau, créer un nouveau dossier et ouvez y un nouveau terminal pour y télécharger les sources:

```cpp
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.x.zip
unzip opencv.zip
```

Mettez vous dans le dossier extrait opencv-4.x. Compiler et installer avec les commandes suivantes:

```cpp
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
	-D CMAKE_INSTALL_PREFIX=/usr/local \
	-D WITH_GTK_2_X=ON \
	-D WITH_GTK3=OFF ..
sudo make -j$(nproc)
sudo make install
```

# 📂Dlib

La librairie dlib est utilisée pour la figure libre et détécter les visages. Dans le dossier suivis_visage, tous les fichiers nécessaires sont déjà présents pour le bon fonctionnement.

Pour tout autre information complémentaire, merci de se référer à :

[dlib C++ Library](http://dlib.net/compile.html)

# 📂GTK2

Pour le projet libre, nous utilisons la librairie GTK2 afin de créer un interface avec des boutons cliquables qui permettent d’activer/désactiver les différents filtres

Pour installer cette dernière sur Linux, dans le terminal:

`>> sudo apt-get install gtk2.0-examples`

`>> sudo apt-get install libgtk2.0-dev`

# 🔨Mbed Studio

- Application à installer pour commander les servomoteurs et envoyer le code servo.cpp sur la carte ArchPro

Installation: Télécharger l’application sur le site Mbed Studio 

[Mbed Studio](https://os.mbed.com/studio/)

Choisir son système d’exploitation et cliquer sur télécharger. Cela télécharge un fichier d’extension .sh

Dans le terminal saisir la commande `bash "MbedStudio-{version}.sh" -y`

Pour tout information complémentaire ou besoin d’aide pour débuter, merci de se référer à : 

[Mbed OS Blinky tutorial](https://os.mbed.com/docs/mbed-studio/current/getting-started/index.html)

Contrôle en mode manuel: en bas de la fenêtre se trouve un terminal directement connecté à la carte, celui si vous servira à déplacer les servos moteurs manuellement si vous le souhaiter.

# 🔨v4l-utils - Connaître les périphériques vidéo connectés

Pour lancer les fonctions, il vous sera demandé d’indiquer un index correspondant à la webcam voulu. Le logiciel ira alors chercher ce flux vidéo directement grâce à l’index fournis et la fonction 

`cv::VideoCapture capture(val_index_cam);`

Afin de connaître l’index des périphériques vidéo connectés, depuis le terminal Linux:

`>> sudo apt-get install v4l-utils`

`>> sudo v4l2-ctl --list-devices`

# 🔨Putty - Alternative

Sans passer par Mbeb studio, il est possible d’utiliser le compilateur en ligne de Mbed. Il faudra ensuite exporter le code en .bin et mettre le fichier .bin directement dans la carte (la carte est reconnue comme périphérique usb)

Pour directement contrôler les servos en mode manuel, il faut alors utiliser le logiciel putty. 

Pour l’installation sur Linux:

`>> sudo apt install putty`

Puis

`>> sudo putty /dev/ttyACM0 -serial -serfg 9600,8,n,1,N`

Pour la démo du mode manuel, merci de regarder la vidéo intitulée: “servo-mode-manuel-demo.mkv”

# 🔨Cmake

Afin de faciliter la compilation, nous utilisons un fichier appeler CmakeList.txt. Pour pouvoir compiler ce dernier qui générera les librairies et les fichiers nécessaires il vous faut donc CMake.

Certaines machines linux possèdent Cmake par défaut mais si ce n’est pas le cas, merci de télécharger depuis le lien suivant le fichier correspondant à votre version d’OS.

[Download | CMake](https://cmake.org/download/)

Dans le terminal, saisir la commande `bash "(nomdufichiertéléchargé".sh" -y`

# 🔨Git

Afin de télécharger git sur votre machine linux, il vous suffit de saisir la commande suivante dans le terminal: `sudo apt install git`

Pour tout autre information complémentaire, merci de se référer à :

[git [Wiki ubuntu-fr]](https://doc.ubuntu-fr.org/git)

# LANCEMENT DES PROGRAMMES

# ⚙️Téléchargement du dossier

Afin de télécharger les fichiers depuis le git:

- Créez un nouveau dossier cible sur votre ordinateur “Projet”
- Dans ce dossier ouvrez un nouveau terminal et entrez la ligne de commande suivante : `git clone https://gitlabsu.sorbonne-universite.fr/polol-et-batoau/projet-machine-c.git`

# 🎨 📷 Figure imposée - suivis de couleur

## Code servomoteur

---

Le code à exporter dans la carte se nomme servo.cpp

Vous pouvez directement exporter le code dans la carte depuis l’application MbedStudio ou alors passer par le compilateur en ligne pour ensuite déplacer le fichier en .bin dans la carte.

Une fois l’export réalisé le programme est alors fonctionnel et servira pour suivre la couleur détectée.

Pour contrôler les servos en Mode Manuel, directement depuis le terminal de Mbed Stuido ou depuis le terminal Putty (cf. explications plus haut) il y a 5 touches possibles

- “r” [appuis 1 fois] pour reset les servos à leur position de départ
- “s” [appuis continu] pour descendre la caméra
- “w” [appuis continu] pour remonter la caméra
- “d” [appuis continu] pour tourner la caméra vers la droite
- “a” [appuis continu] pour tourner la caméra vers la gauche

Pour la démo du mode manuel, merci de regarder la vidéo intitulée: “servo-mode-manuel-demo.mkv”

## Suivis et détection de couleur

Lien d’accès: Projet/projet-machine-c//suivis_couleurs

Construction de l’éxécutable, ouvrir un nouveau terminal depuis le dossier et saisir:

`>> cmake .` 

`>> make`

### Lancement du programme

Pour lancer l’éxécutable il suffit alors de faire `>> ./main` 

Il vous sera alors demandé de rentrer l’index de la caméra voulue et de saisir 1 ou 0 en fonction de si la carte ArchPro est connectée ou non.

Toutes ces informations vous seront rappelées lors de l’exécution

Le programme affiche 3 fenêtres:

- Fenêtre de contrôle avec des sliders pour manuellement selectionner la couleur que vous souhaiter détecter
- Fenêtre d’affichage de la webcam
- Fenêtre Thresold qui isole la couleur détectée

### Utilisation du programme - Choix de la couleur détéctée

Mode manuel → en réglant les sliders manuellement. Les sliders sont définis en mode [HSV](https://fr.wikipedia.org/wiki/Teinte_Saturation_Valeur) pour Teinte, Saturation et Valeur en français.

![Untitled](README%205f1eb2ce099d4c98ae2f2346046871eb/Untitled%201.png)

En cliquant sur “t” un screen de la vidéo est alors pris et s’affiche dans la fenêtre de contrôle. Il suffit alors de cliquer sur la couleur voulue quelque part dans l’image. Automatiquement, l’image disparait et laisse place à un onglet de vérification qui montre en fond la couleur sélectionner et le code correspond en HSV et RGB.

![Example avec couleur vert/jaune et mise à jour des sliders](README%205f1eb2ce099d4c98ae2f2346046871eb/Untitled%202.png)

Example avec couleur vert/jaune et mise à jour des sliders

Automatiquement les sliders sont mis à jour avec une certaine tolérance afin d’isoler la couleur.

Sur la fenêtre de la webcam devrait alors apparaître un cercle localisant l’objet de la couleur sélectionnée ainsi qu’un point central qui servira pour faire bouger les servomoteur. 

Sur la fenêtre Treshold apparaît la forme de l’objet de la couleur isolée.

Si vous avez mal cliqué, vous pouvez à tout moment appuyer sur “t” pour reselectionner la couleur ou sur “e” pour reset tous les sliders à 0

Avec les servos moteurs, si à n’importe quel moment vous cliquez sur “r”, ces derniers sont remis à leur position de départ.

### Utilisation du programme - Suivis de couleur

Si vous voulez activer le suivis de couleur, vous avez normalement du écrire 1 sur le terminal au lancement du programme.

Automatiquement lors de la sélection de la couleur, le programme va alors récupérer la position du centre l’objet. En fonction de cette dernière, il enverra alors des informations au servos pour aller à droite/gauche ou en haut/bas et remettre l’objet au centre.

Suivis de couleurVous pouvez vérifier tout cela en temps réel sur la fenêtre de la webcam avec les rectangles qui s’afficheront en rouge si l’objet dépasse du carré central [affichage des rectangles rouges uniquement en mode 1 (carte connectée) afin de ne pas surcharger l’image dans le mode 0 qui lui l’utilise pas les servos moteurs].

![Untitled](README%205f1eb2ce099d4c98ae2f2346046871eb/Untitled%203.png)

![Untitled](README%205f1eb2ce099d4c98ae2f2346046871eb/Untitled%204.png)

Disclaimer: merci de vous assurez que la couleur détectée soit bien isolée du reste et d’être dans une pièce où la lumière est homogène, sinon, il risque d’y avoir des interférences, notamment provoquées lorsque les servos bougent. Si la lumière change alors, le programme n’isolera pas correctement la couleur et les servos pourraient bouger à une position nous voulue.

Si cela arrive, cliquer sur “e” pour reset les sliders, puis sur “r” pour remettre les servos à leurs position de départ puis recommencez la sélection de couleur.

### Quitter le programme

Pour quitter le programme, il vous suffit d’être sur une des fenêtre affichées puis d’appuyer sur la touche “q”. 

Depuis le terminal ctrl + C permet aussi de mettre fin au programme

# 👪 📷 Figure libre - détection de visage et création de filtres variés et ludiques

---

## Compilation et lancement

Lien d’accès au fichier: `projet-machine-c/suivis_visage/dlib-projet/projet_polo_benito` 

Dans ce dossier vous trouverez le fichier source `visageandfiltres.cpp` ainsi que le fichier `CMakeLists.txt`

Afin de compiler le programme, veuillez vous rendre dans le dossier `build` et de la créer si celui ci n’existe pas.

Ouvrir un terminal depuis le dossier build et saisir:

`>> cmake ..` 

`>> cmake --build . --config Release` nb: la première compilation prendra du temps car elle doit reconstruire toute la librairie Dlib.

Pour lancer le programme il suffit alors après compilation de saisir:

`>> ./visageandfiltres`

Pour toute autre information merci de vous référer à [http://dlib.net/compile.html](http://dlib.net/compile.html)

## Utilisation du programme

Ce programme est beaucoup plus intuitif car tout se fait avec la fenêtre de control de Gtk.

Au démarage il y a 2 fenêtres qui s’affichent, une avec le feed en temps réel de la webcam et l’affichage du contour des visages et une autre de control pour sélectionner l’effet/le filtre voulu.

![Untitled](README%205f1eb2ce099d4c98ae2f2346046871eb/Untitled%205.png)

[DISCLAIMER] Le logiciel permet de détecter plusieurs visages à la fois mais en mode filtre, merci de vous assurez qu’il y ai UN SEUL visage devant la webcam sinon, le logiciel ne saura sur lequel appliquer les transformations 

[DISCLAIMER] Les filtres demandent assez de ressources à votre PC, nous ne pouvons que vous conseiller de ne pas les utiliser en même temps

[DISCLAIMER] Pour activer le bouton suivis_visage merci de bien vérifier que la carte est bien connectée sinon il y aura une erreur de segmentation

Afin de lancer à filtre, il suffit de cliquer sur le Bouton correspondant, un label apparait alors sur l’img_originale pour vous indiquer que le filtre est actif et une fenêtre avec le filtre est créé et s’affiche.

Pour désactiver le filtre il suffit de recliquer sur ce même bouton.

Example avec [Filtre lunettes]

![Untitled](README%205f1eb2ce099d4c98ae2f2346046871eb/Untitled%206.png)

Pour le Label, il suffit d’écrire le texte que vous souhaitez afficher à l’écran et de cliquer sur la ton entrer.

Pour désactiver le Label, il vous suffit de vous remettre dans le carré où vous avez écris et de recliquer sur la touche entrer 

Pour quitter le programme, il suffit d’appuyer sur la touche [Quitter]

VOIR LA VIDEO DEMO FILTRE + DEMO SUIVIS_VISAGE POUR PLUS DE PRECISIONS
