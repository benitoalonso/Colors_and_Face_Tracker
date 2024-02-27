/*
ALONSO Benito  & SHLYKOVA Polo

Programme pour suivis de couleur directement grâce à la webcam.
Fonctionnement:
- 	utilise 3 fenetres: celle de la webcam, celle avec les seuils de couleurs et la fenetre de control
- 	pour selectionner une couleur a suivre 2 options: DANS LA FENETRE CONTROL
		-	tout faire manuellement en controlant les trackbars
		-	cliquer sur T pour "take", cela fait alors apparaite une copie de la webcam à l'instant du click
			cliquer alors sur avec le clic gauche de la souris sur la couleur à suivre
			une fois le click, l'image disparait et a la place une petite image avec le fond de couleur choisi + texte HSV et RGB de la couleur s'affiche
			automatiquement // alors les trackbars s'updatent et la couleur est desormais reconnue et suivie
-	pour verifier que le suivis de couleur est bien efficace, dans la fenetre ImgThrasholded, les pixels blancs correspondent à la couleur reconnue et isolée
- 	dans la fenetre Original, un cercle rouge apparait et permet trace un cercle autour de l'objet suivis/selectionne

Fontion suivis:
-	directement en récupérant la position du centre de lobjet trouvé, si ce dernier sors du carré central alors le programme écris directement sur le fichier qui correspond
	au port série de la carte. Cette dernière récupérer les informations envoyées ( a s d ou w) et bouge alors les servos en conséquence.
*/

#include <iostream>
#include <string>
#include <math.h>
#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


// Linux headers
/*
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
*/

#define COLOR_LIGNES 80
#define COLOR_COLONNES 250

using namespace cv;
using namespace std;

//struct termios tty;


void on_mouse_click(int event, int x, int y, int flags, void* ptr) {
	if (event == cv::EVENT_LBUTTONDOWN) { 
	
		//si le clique gauche souris est détécté (dans la fenetre snapshot)
		cv::Mat* snapshot = (cv::Mat*)ptr;

		cv::Mat image=(*snapshot).clone();
 		//Vec3b rgb=image.at<Vec3b>(y,x);

		//recupere les valeurs du pixel clique en rgb directement
		cv::Vec3b pixel = snapshot->at<cv::Vec3b>(cv::Point (x, y));
		int b, g, r;
		b = pixel[0]; 	
		g = pixel[1];	
		r = pixel[2];	

		// determine couleur police a partir de la couleur en background soit blanc soit noir
		// reference: stackoverflow : determine-font-color-based-on-background-color
		float luminance = 1 - (0.299*r + 0.587*g + 0.114*b) / 255;
		cv::Scalar textColor;
		if (luminance < 0.5) {
			textColor = cv::Scalar(0,0,0);
		} else {
			textColor = cv::Scalar(255,255,255);
		}
		
		//texte a afficher pour le rgb
		std::string rgbText = "RGB[" + std::to_string(r) + "," + std::to_string(g)+ "," + std::to_string(b) + "]";

		//couleur de fond pour la fenêtre
		cv::Mat colorArray;
		colorArray = cv::Mat(COLOR_LIGNES, COLOR_COLONNES, CV_8UC3, cv::Scalar(b,g,r));

		//affichage
		cv::putText(colorArray, 
					rgbText, 
					cv::Point2d(10, COLOR_LIGNES - 10), //position du texte
					cv::FONT_HERSHEY_SIMPLEX, 
					0.7, 
					textColor); 

		//conversion de l'image en HSV
		cv::Mat HSV;
		cv::Mat RGB=image(cv::Rect(x,y,1,1)); //permet de convertir la zone cliquée uniquement=moins de ressources necessaires
		cv::cvtColor(RGB, HSV,COLOR_BGR2HSV); 

		//recupère l'info du pixel en question
		Vec3b hsv=HSV.at<Vec3b>(0,0);
		int H=hsv.val[0];
		int S=hsv.val[1];
		int V=hsv.val[2];

		//update position de la teinte directement sur la trackbar
		cv::setTrackbarPos("BasH", "Control",H-10);
		cv::setTrackbarPos("HautH", "Control",H+10);
		int tolerancee=70;
		cv::setTrackbarPos("BasS", "Control",S-tolerancee); //permet d'avoir une certaine tolérance a ajuster si besoin mais 50 en moyenne fonctionne bien
		cv::setTrackbarPos("HautS", "Control",S+tolerancee);

		cv::setTrackbarPos("BasV", "Control",V-tolerancee);
		cv::setTrackbarPos("HautV", "Control",V+tolerancee);


		//texte a afficher pour le hsv
		std::string hsvText = "HSV[" + std::to_string(H) + "," + std::to_string(S)+ "," + std::to_string(V) + "]";

		//affichage
		cv::putText(colorArray, 
					hsvText, 
					cv::Point2d(10, COLOR_LIGNES - 40), //position du texte
					cv::FONT_HERSHEY_SIMPLEX, 
					0.7, 
					textColor); 

		cv::imshow("Control", colorArray);
	}
}

void ecriture(char lettre,int nb){
	for(int i=0;i<nb;i++){
    FILE* fp;
    printf("fichier\n");

    fp = fopen ("/dev/ttyACM0", "w");
    printf("fichier trouvé\n");

    if (fp == NULL) {
        printf("fichier introuvable : abandon\n");
    }

    else {
        fprintf(fp, "%c", lettre);
        printf("écrit : %c \n", lettre);
        //for(int j=0;j<99999999;j++){
          //wait
        //}
    }
    fclose(fp);
    printf("fichier fermé\n");
    }
}


int main(int argc, char** argv) {
	//sudo putty /dev/ttyACM0 -serial -sercfg 9600,8,n,1,N
	//pour le servo
	
	//---------------------selection du module camera
    int val_index_cam =0;
    printf("Renseignez le *numero* de la camera a selectionner: ");
    
    //scanf("%d", &val_index_cam);
    if( scanf("%d", &val_index_cam) != 1 ) 
    {
    fprintf( stderr, "renseignez un numero uniquement  pour le scanf\n");
    exit(1);
    }

    //creation de l'objet videocapture qui recupère directement la video de la webcam en temps reel
    cv::VideoCapture capture(val_index_cam);

    //verification de l'ouverture de la webcam
    if(!capture.isOpened()){
        cout<<"can not open webcam"<<endl;
        printf("renseignez un index correct de camera\n");
        printf("afin de connaitre les camera connectees:\n"); 
        printf(">> sudo apt-get install v4l-utils\n");
        printf(">> sudo v4l2-ctl --list-devices\n");
        return 0;
    }

	//---------------------demande si carte connextée
	int val_index_carte =0;
	printf("Renseignez si la carte ArchPro est connectee (1 si oui) (0 sinon): ");
	if( scanf("%d", &val_index_carte) != 1 ) 
    {
    fprintf( stderr, "renseignez uniquement soit 1 soit 0 pour le scanf\n");
    exit(1);
    }
	printf("Si la carte est connectee et reconnue par la machine, et que vous avez ecris 1 \n");
	printf("alors le programme permettra en plus de détecter la couleur, de suivre l'objet tracker en contrôlant les servomoteurs\n");
	printf("Si la carte n'est pas connectee, et que vous avez ecris 0 \n");
	printf("alors le programme ne permettra que de détecter la couleur souhaitée\n");

	//Fenetre de selection manuel de couleur + affichage couleur cliquée. 
	cv::namedWindow("Control");		//Control Window
	
	int iBasH = 0;		//TEINTE
	int iHautH = 0;

	int iBasS = 0;		//SATURATION
	int iHautS = 0;

	int iBasV = 0;		//VALEUR
	int iHautV = 0;

	//Creation des trackbar dans la fenetre de control 
	cv::createTrackbar("BasH", "Control", 0, 179,0);		//Hue (0 - 179)
	cv::setTrackbarPos("BasH", "Control",iBasH);
	
	cv::createTrackbar("HautH", "Control", 0, 179,0);
	cv::setTrackbarPos("HautH", "Control",iHautH);
	
	cv::createTrackbar("BasS", "Control", 0, 255,0);		//Saturation (0 - 255)
	cv::setTrackbarPos("BasS", "Control",iBasS);
	
	cv::createTrackbar("HautS", "Control", 0, 255,0);
	cv::setTrackbarPos("HautS", "Control",iHautS);

	cv::createTrackbar("BasV", "Control", 0, 255,0);		//Value (0 - 255)
	cv::setTrackbarPos("BasV", "Control",iBasV);
	
	cv::createTrackbar("HautV", "Control", 0, 255,0);
	cv::setTrackbarPos("HautV", "Control",iHautV);
	

	//pour declarer les fenetre: screen webcam + fenetre affichage couleur selec
	cv::Mat imgOriginal, snapshot, colorArray;
	capture.read(imgOriginal);

	snapshot = cv::Mat(imgOriginal.size(), CV_8UC3, cv::Scalar(0,0,0));
	//cv::imshow("Snapshot", snapshot);

	colorArray = cv::Mat(COLOR_LIGNES, COLOR_COLONNES, CV_8UC3, cv::Scalar(0,0,0));
	cv::imshow("Control", colorArray); //affiche la "fenetre" pour la couleur cliquee directement dans la fenetre de Control
	cv::moveWindow("Control",680,10); //position de la fenetre reference pour ecran 1920*1080

	cv::setMouseCallback("Control", on_mouse_click, &snapshot);

	cv::circle(imgOriginal, cv::Point(320, 240), 1, cv::Scalar(0, 0, 255), 2); //point centre image

	printf("entrée main\n");
    	char shaut='w';
    	char sbas='s';
	char sgauche='d'; //INVERSE CAR EN MIROIR
	char sdroite='a';




	int keyVal;
	while (1) {
		if (!capture.read(imgOriginal)) {
			break;
		}
		//cv::imshow("Video", imgOriginal);

		keyVal = cv::waitKey(1) & 0xFF;
		if (keyVal == 113) {    //pour quitter = q
			break;
		} else if (keyVal == 116) {  //pour cloner la fenetre = t
			snapshot = imgOriginal.clone();
			cv::imshow("Control", snapshot);
		}
		else if(keyVal==114){//r = reset position des servos
			ecriture('r',1);
					
		}
		else if(keyVal==101){//e = resert trackbar si besoin
			cv::setTrackbarPos("BasH", "Control",0);
			cv::setTrackbarPos("HautH", "Control",0);
			
			cv::setTrackbarPos("BasS", "Control",0); 
			cv::setTrackbarPos("HautS", "Control",0);

			cv::setTrackbarPos("BasV", "Control",0);
			cv::setTrackbarPos("HautV", "Control",0);
					
		}

		//fonction suivis couleur
		// Convertis imgOriginal de BGR a HSV
		cv::Mat imgHSV;
		cv::cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

		//creer l'image Thresholded
		cv::Mat imgThresholded;
		cv::inRange(imgHSV, cv::Scalar(cv::getTrackbarPos("BasH","Control"),cv::getTrackbarPos("BasS","Control"),cv::getTrackbarPos("BasV","Control")), cv::Scalar(cv::getTrackbarPos("HautH","Control"),cv::getTrackbarPos("HautS","Control"),cv::getTrackbarPos("HautV","Control")), imgThresholded);
		//creer l'image avec la detection des pixels grace aux differentes valeurs des sliders update directement lors du click grace a des pointeurs


		//Traitement de l'image pour assurer bonne analyse
		// Morphological Opening - enleve petits objet en fond
		cv::erode(imgThresholded, imgThresholded, cv::getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5)));
		cv::dilate(imgThresholded, imgThresholded, cv::getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5)));

		// Morphological Closing - enleve petits trous en fond
		cv::dilate(imgThresholded, imgThresholded, cv::getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5)));
		cv::erode(imgThresholded, imgThresholded, cv::getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5)));

		// Calcul des moments de l'image Thresolded		
		// cela permet de trouver le centre de l'objet 
		cv::Moments oMoments = cv::moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;
		//printf("dArea = %f\n",dArea);
		//reference: learnopencv : find-center-of-blob-centroid-using-opencv-cpp

		int delta=60; //notre deadzone autour du centre 
		cv::rectangle(imgOriginal,cv::Point(320-delta,0),cv::Point(320+delta,480), cv::Scalar( 255, 0, 0 )); //cadrillage sur la vidéo pour mieux se reperer
		cv::rectangle(imgOriginal,cv::Point(0,240-delta),cv::Point(640,240+delta), cv::Scalar( 255, 0, 0 )); //idem


		// si area <= on peu considérer que c'est du bruit
		if (dArea > 70000)
		{
			// Calcul du centre de l'object
			int posX = dM10 / dArea;
			int posY = dM01 / dArea;
			//cout << posX << "," << posY << endl;

			// trace cercle rouge autour objet
			int R = sqrt((dArea / 255) / 3.14); //rayon de l'objet en question
			if (posX >= 0 && posY >= 0)
			{
				cv::circle(imgOriginal, cv::Point(posX, posY), R, cv::Scalar(0, 0, 255), 2); //cercle autour objet
				cv::circle(imgOriginal, cv::Point(posX, posY), 1, cv::Scalar(123, 7, 34), 2); //point au centre de l'objet

				if(val_index_carte==1){//si on est en mode carte connectée
					if ((posX>320-delta)&&(posX<320+delta)&&(posY>0)&&(posY<240-delta)) { //carre haut centre
						cv::rectangle(imgOriginal,cv::Point(320-delta,0),cv::Point(320+delta,240-delta), cv::Scalar( 23, 10, 255 ),2);
						ecriture(shaut,1);
					}
					else if ((posX>320-delta)&&(posX<320+delta)&&(posY>240+delta)&&(posY<480)) {  //carre bas centre
						cv::rectangle(imgOriginal,cv::Point(320-delta,240+delta),cv::Point(320+delta,480), cv::Scalar( 23, 10, 255 ),2);
						ecriture(sbas,1);
					}
					else if ((posX>0)&&(posX<320-delta)&&(posY>240-delta)&&(posY<240+delta)) { //carre gauche centre
						cv::rectangle(imgOriginal,cv::Point(0,240-delta),cv::Point(320-delta,240+delta), cv::Scalar( 23, 10, 255 ),2);
						ecriture(sgauche,2);
						//ecriture(sgauche);
					}
					else if ((posX>320+delta)&&(posX<640)&&(posY>240-delta)&&(posY<240+delta)) { //carre droite centre
						cv::rectangle(imgOriginal,cv::Point(320+delta,240-delta),cv::Point(640,240+delta), cv::Scalar( 23, 10, 255 ),2);
						ecriture(sdroite,2);
						//ecriture(sdroite);
					}

					else if ((posX<320-delta)&&(posY<240-delta)) { //carre haut gauche
						cv::rectangle(imgOriginal,cv::Point(0,0),cv::Point(320-delta,240-delta), cv::Scalar( 0, 0, 255 ),2);
						ecriture(shaut,2);
						ecriture(sgauche,1);

					}
					else if ((posX>320+delta)&&(posY<240-delta)) { //carre haut droite
						cv::rectangle(imgOriginal,cv::Point(320+delta,0),cv::Point(640,240-delta), cv::Scalar( 0, 0, 255 ),2);
						ecriture(shaut,2);
						ecriture(sdroite,1);

					}
					else if ((posX<320-delta)&&(posY>240+delta)) { //carre bas gauche
						cv::rectangle(imgOriginal,cv::Point(0,240+delta),cv::Point(320-delta,480), cv::Scalar( 0, 0, 255 ),2);
						ecriture(sbas,2);
						ecriture(sgauche,1);

					}
					else if ((posX>320+delta)&&(posY>240+delta)) { //carre bas droite
						cv::rectangle(imgOriginal,cv::Point(320+delta,240+delta),cv::Point(640,480), cv::Scalar( 0, 0, 255 ),2);
						ecriture(sbas,2);
						ecriture(sdroite,1);
					}
				}
			}
		}

		// Affiche l'image Thresholded 
		cv::imshow("Thresholded Image", imgThresholded);
		cv::moveWindow("Thresholded Image",10,550); //position de la fenetre reference pour ecran 1920*1080

		// Affiche image original + le cercle si detection 
		cv::imshow("Original", imgOriginal);
		cv::moveWindow("Original",10,10); //position de la fenetre reference pour ecran 1920*1080

		//int taillewebx = 640;
		//int tailleweby = 480;
		
	}
	return 0;
}
